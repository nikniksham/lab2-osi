#include "lab2.h"
#include <iostream>
#include <map>
#include <unistd.h>
#include <cerrno>
#include <random>
#include <cstring>
#include <fcntl.h>


// Размер блока
#define BLOCK_SIZE 4096
// Макс кол-во блоков
#define MAX_BLOCKS_IN_CACHE 180

// Создаем объект генератора случайных чисел, инициализируем его случайным значением
std::random_device rd;
std::mt19937 gen(rd());

// Статистика работы кэша
struct CacheStats cache_stats {0, 0};

// Кэшблок
struct CacheBlock {
    char* data;
    bool dirty_data;
    ssize_t useful_data;
};


// Файловый дескриптор
struct FileDescriptor {
    int fd;
    off_t offset;
};

// Пара - fd / id блока, соответсвующий отступу в файле
typedef std::pair<int, off_t> CacheKey;

// Таблица файловых дескрипторов
std::map<int, FileDescriptor> fd_table;
// Таблица блоков кэша
std::map<CacheKey, CacheBlock> cache_table;

// ???
FileDescriptor& get_file_descriptor(int fd);

// Получение случайного числа [min; max]
int get_rand_from_to(int min, int max) {
    return std::uniform_int_distribution<>(min, max)(gen);
}

// Выделение памяти под кэшблок (адреса выравнены)
char* allocate_aligned_buffer() {
    void* buf = nullptr;
    if (posix_memalign(&buf, BLOCK_SIZE, BLOCK_SIZE) != 0) {
        errno = EBADF;
        std::cerr<<"Cant allocate aligned buffer";
        return nullptr;
    }
    return static_cast<char*>(buf);
}



// Запись кэшблока на диск
int write_cache_block(int fd, void *buf, int count, int start_pos) {
//    std::cout << fd << " " << count << " " << start_pos << "\n";
    ssize_t ret = pwrite(fd, static_cast<char*>(buf), count, start_pos);
//    std::cout << ret << " " << count << "\n";
    if (ret != count) {
        std::cerr<<"Cant write cache_block\n";
        return -1;
    }
    return 0;
}

// Освобождение случайного кэшблока
void free_cache_block(int found_fd) {
    if (!cache_table.empty()) {
        int rn_id = get_rand_from_to(0, static_cast<int>(cache_table.size() - 1));
        for (auto obj : cache_table) {
            if (rn_id == 0) {
                if (obj.second.dirty_data) {
                    if (write_cache_block(found_fd, obj.second.data, (int)obj.second.useful_data, obj.first.second * BLOCK_SIZE) != 0) {
                        std::cerr<<"Cant flush block (free cache block)\n";
                        return;
                    }
                    obj.second.dirty_data = false;
                }
                if (obj.second.data != nullptr) {
                    free(obj.second.data);
                }
                cache_table.erase(obj.first);
                return;
            }
            rn_id--;
        }
    }
}

void free_all_cache_blocks() {
    for (auto obj = cache_table.begin(); obj != cache_table.end();) {
        if (obj->second.data != nullptr) {
            free(obj->second.data);
        }
        cache_table.erase(obj++);
    }
}

// Открытие файла
int lab2_open(const char* path) {
    std::cout<<"666666666\n";
    // Отключаем буферизацию
    const int fd = open(path, O_DIRECT | O_RDWR, NULL);
    if (fd < 0) {
        std::cerr<<"Cant open file\n";
        return -1;
    }

    fd_table[fd] = {fd, 0};
    return fd;
}

// Закрытие файла
int lab2_close(const int fd) {
    const auto iterator = fd_table.find(fd);
    if (iterator == fd_table.end()) {
        std::cerr<<"Bad fd, cant close\n";
        return -1;
    }
    lab2_fsync(fd);
    const int result = close(iterator->second.fd);
    fd_table.erase(iterator);
    return result;
}

// Чтение из файла
ssize_t lab2_read(const int fd, void *buf, const size_t count) {
    auto& [found_fd, file_offset] = get_file_descriptor(fd);
    if (found_fd < 0 || file_offset < 0 || !buf) {
        errno = EIO;
        return -1;
    }
    ssize_t bytes_read = 0;
    const auto buffer = static_cast<char*>(buf);

    while (bytes_read < count) {
        // Получаем id блока, в который будем читать
        off_t block_id = file_offset / BLOCK_SIZE;
        // Отступ внутри кэшблока
        const size_t block_offset = file_offset % BLOCK_SIZE;
        // Сколько байт прочтём на данной итерации
        const int iteration_read = static_cast<int>(std::min(BLOCK_SIZE - block_offset, count - bytes_read));
        // Смотрим, есть ли блок в кэше
        CacheKey key = {found_fd, block_id};
        auto cache_iterator = cache_table.find(key);
        size_t bytes_from_block;
        if (cache_iterator != cache_table.end()) {
            // Попали в кэшблоки
            cache_stats.cache_hits++;

            CacheBlock& found_block = cache_iterator->second;
//            std::cout<<"("<<block_id<<") --------------------- "<<key.first<<" --- "<<key.second<<"\n"<<found_block.data<<"\n";
//            std::cout<<"("<<block_id<<") --------------------- "<<key.first<<" --- "<<key.second<<"\n";
            // Получаем количество байт, которое можем прочесть, как <полезная дата> - (<размер блока> - <смещение в блоке>)
            ssize_t available_bytes = found_block.useful_data - (int)block_offset;

            // На случай, если что ничего прочесть не можем (полезной даты первые 300 байт, надо читать с 500-ого)
            if (available_bytes <= 0) {
                break;
            }

            // Берём минимум из: сколько байт можем прочесть / сколько надо
            bytes_from_block = std::min(iteration_read, static_cast<int>(available_bytes));

            // Копируем данные из кэша
            memcpy(buffer + bytes_read, found_block.data + block_offset, bytes_from_block);
        } else {
            // Не попали в кэшблоки
            cache_stats.cache_miss++;

            // Если место закончилось, то удаляем какой-нибудь уже существующий кэшблок
            if (cache_table.size() >= MAX_BLOCKS_IN_CACHE) {
                free_cache_block(found_fd);
            }

            // Создаём будущий кэшблок и читаем в него
            char* aligned_buf = allocate_aligned_buffer();
            const ssize_t valid_read = pread(found_fd, aligned_buf, BLOCK_SIZE, block_id * BLOCK_SIZE);
//            std::cout<<valid_read<<"---------------------"<<key.first<<" --- "<<key.second<<"\n";
//            std::cout<<valid_read<<"---------------------"<<key.first<<" --- "<<key.second<<"\n"<<aligned_buf<<"\n";
            // Проверки на ошибки чтение и окончание файла
            if (valid_read < 0) {
                free(aligned_buf);
                std::cerr<<"Cant read file to cache\n";
                return -1;
            }
            if (valid_read == 0) {
                free(aligned_buf);
                break;
            }

            // Создаём блок, записываем в него, сколько данных мы прочли
            CacheBlock new_block = {aligned_buf, false, valid_read};
            cache_table[key] = new_block;

            // Смотрим, сколько байт сможем прочесть (считаем, как <прочитанные байты в блок> - <смещение, с которого надо прочесть в блоке>)
            int available_bytes = static_cast<int>(valid_read) - static_cast<int>(block_offset);
            if (available_bytes <= 0) {
                break;
            }

            // Записываем данные из только что созданного кэшблока, ровно столько, сколько требуется
            bytes_from_block = std::min(available_bytes, iteration_read);
            std::memcpy(buffer + bytes_read, aligned_buf + block_offset, bytes_from_block);
        }
        // Фиксируем результаты итерации
        file_offset += static_cast<int>(bytes_from_block);
        bytes_read += static_cast<ssize_t>(bytes_from_block);
//        std::cout<<bytes_read<<"\n";
    }

    return bytes_read;
}

// Запись в кэшблоки
ssize_t lab2_write(const int fd, const void* buf, const size_t count) {
    auto& [found_fd, file_offset] = get_file_descriptor(fd);
    if (found_fd < 0 || file_offset < 0 || !buf) {
        errno = EBADF;
        return -1;
    }
//    std::cout<<file_offset<<"\n-------------------\n";
    ssize_t bytes_written = 0;
    const auto buffer = static_cast<const char*>(buf);

    while (bytes_written < count) {
        // Получаем id блока, в который будем писать
        off_t block_id = file_offset / BLOCK_SIZE;
        // Отступ внутри кэшблока
        const size_t block_offset = file_offset % BLOCK_SIZE;
        // Сколько байт запишем на данной итерации
        const int iteration_write = static_cast<int>(std::min(BLOCK_SIZE - block_offset, count - bytes_written));

        CacheKey key = {found_fd, block_id};
        auto cache_iterator = cache_table.find(key);
        CacheBlock* block_ptr;
        if (cache_iterator == cache_table.end()) {
            // Не попали в кэшблоки
            cache_stats.cache_miss++;

            // Освобождаем место, если закончилось
            if (cache_table.size() == MAX_BLOCKS_IN_CACHE) {
                free_cache_block(found_fd);
            }

            // Создаём кэшблок, записываем в него данные из файла
            char* aligned_buf = allocate_aligned_buffer();
            const ssize_t valid_write = pread(fd, aligned_buf, BLOCK_SIZE, BLOCK_SIZE * block_id);
            if (valid_write < 0) {
                free(aligned_buf);
                std::cerr<<"Cant write file to cache\n";
                return -1;
            }

            // Создаём кэшблок, пока что в нём нет грязной даты и полезная дата - та, сколько успели прочесть
            CacheBlock& block = cache_table[key] = {aligned_buf, false, valid_write};
            block_ptr = &block;
        } else {
            // Попали в кэшблоки
            cache_stats.cache_hits++;
            block_ptr = &cache_iterator->second;
        }
        // Записываем в кэшблок, теперь он содержит грязные данные
//        std::cout<<block_offset<<" "<<bytes_written<<" "<<iteration_write<<"\n";
        memcpy(block_ptr->data + block_offset, buffer + bytes_written, iteration_write);
        block_ptr->dirty_data = true;
        // Обновляем useful_data - мы могли записать чуть больше, чем было записано в блок раньше
        block_ptr->useful_data = std::max(block_ptr->useful_data, static_cast<ssize_t>(block_offset+iteration_write));

        // Фиксируем результаты итерации
        file_offset += iteration_write;
        bytes_written += iteration_write;
    }

    return bytes_written;
}

// Смещение в файле
off_t lab2_lseek(const int fd, const off_t offset, const int whence) {
    auto& [found_fd, file_offset] = get_file_descriptor(fd);
    if (found_fd < 0 || file_offset < 0) {
        errno = EBADF;
        return -1;
    }
    if (whence != SEEK_SET || offset < 0) {
        errno = EINVAL;
        return -1;
    }
    file_offset = offset;
    return file_offset;
}

int lab2_fsync(int fd) {
    const int found_fd = get_file_descriptor(fd).fd;
    if (found_fd < 0) {
        errno = EBADF;
        return -1;
    }

    // Запишем блоки с грязными данными
    for (auto& [key, block] : cache_table) {
        if (key.first == found_fd && block.dirty_data) {
            if (write_cache_block(found_fd, block.data, (int)block.useful_data, key.second * BLOCK_SIZE) != 0) {
                std::cerr<<"Cant flush block (fsync)\n";
                return -1;
            }
            block.dirty_data = false;
        }
    }

    return 0;
}

// Получить пару: file_descriptor / file_offset
FileDescriptor& get_file_descriptor(const int fd) {
    const auto iterator = fd_table.find(fd);
    if (iterator == fd_table.end()) {
        static FileDescriptor invalid_fd = {-1, -1};
        return invalid_fd;
    }
    return iterator->second;
}

// Получение статистики работы кэша
int get_cache_miss() {
    return cache_stats.cache_miss;
}
int get_cache_hit() {
    return cache_stats.cache_hits;
}

void reset_cache_stats() {
    cache_stats = {0, 0};
}

