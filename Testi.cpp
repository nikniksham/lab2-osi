#include <iostream>
#include "cache/lab2.h"
#include <chrono>
#include <fcntl.h>
#include <csignal>

using namespace std;

int main() {
    bool test1 = false;
    bool test2 = false;
    bool test3 = false;
    bool test4 = true;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    chrono::duration<double> duration;
    int fd, times;
    if (test1) {
        char* filename = "test.txt";
        times = 100001;
        char *buf = new char[35000];

        cout<<"Тест #1 - простое чтение одного и того же большого блока данных "<< times <<" раз\n\n";

        start = chrono::high_resolution_clock::now();
        fd = open(filename, O_DIRECT, NULL);
        for (int i = 0; i < times; ++i) {
            pread(fd, buf, 35000, 0);
        }
        close(fd);
        duration = chrono::high_resolution_clock::now() - start;
        cout << "Время выполнения без кэша: " << duration.count() << " секунд.\n\n";

        start = chrono::high_resolution_clock::now();
        fd = lab2_open(filename);
        for (int i = 0; i < times; ++i) {
            lab2_read(fd, buf, 35000);
            lab2_lseek(fd, 0, 0);
        }
        lab2_close(fd);
        cout << "Cache hits: " << get_cache_stats().cache_hits << ", Cache miss: " << get_cache_stats().cache_miss << endl;
        duration = chrono::high_resolution_clock::now() - start;
        cout << "Время выполнения с кэшем: " << duration.count() << " секунд.\n";


        free_all_cache_blocks();
        reset_cache_stats();
        cout<<"\n----------------------------------------\n\n\n";
    }
    if (test2) {
        char* filename = "trash.txt";
        times = 100001;
        char *buf = new char[9000];

        cout<<"Тест #2 - чтение случайного блока данных "<< times <<" раз\n\n";

        start = chrono::high_resolution_clock::now();
        fd = open(filename, O_DIRECT, NULL);
        for (int i = 0; i < times; ++i) {
            lseek(fd, get_rand_from_to(0, 800000), 0);
            read(fd, buf, 9000);
        }
        close(fd);
        duration = chrono::high_resolution_clock::now() - start;
        cout << "Время выполнения без кэша: " << duration.count() << " секунд.\n\n";

        start = chrono::high_resolution_clock::now();
        fd = lab2_open(filename);
        for (int i = 0; i < times; ++i) {
            lab2_lseek(fd, get_rand_from_to(0, 800000), 0);
            lab2_read(fd, buf, 9000);
        }
        lab2_close(fd);
        cout << "Cache hits: " << get_cache_stats().cache_hits << ", Cache miss: " << get_cache_stats().cache_miss << endl;
        duration = chrono::high_resolution_clock::now() - start;
        cout << "Время выполнения с кэшем: " << duration.count() << " секунд.\n";


        free_all_cache_blocks();
        reset_cache_stats();
        cout<<"\n----------------------------------------\n\n\n";

    }
    if (test3) {
        char* filename = "write_test22.txt";
        times = 100001;
        char *buf = new char[500];

        cout<<"Тест #3 - Чтение из произвольного места и запись результата в произвольное место файла "<< times <<" раз\n\n";

        start = chrono::high_resolution_clock::now();
        fd = open(filename, O_DIRECT, NULL);
        for (int i = 0; i < times; ++i) {
            lseek(fd, get_rand_from_to(0, 800000), 0);
            read(fd, buf, 500);
            lseek(fd, get_rand_from_to(0, 800000), 0);
            write(fd, buf, 500);
        }
        close(fd);
        duration = chrono::high_resolution_clock::now() - start;
        cout << "Время выполнения без кэша: " << duration.count() << " секунд.\n\n";

        start = chrono::high_resolution_clock::now();
        fd = lab2_open(filename);
        for (int i = 0; i < times; ++i) {
            lab2_lseek(fd, get_rand_from_to(0, 800000), 0);
            lab2_read(fd, buf, 500);
            lab2_lseek(fd, get_rand_from_to(0, 800000), 0);
            lab2_write(fd, buf, 500);
        }
        lab2_fsync(fd);
        lab2_close(fd);
        cout << "Cache hits: " << get_cache_stats().cache_hits << ", Cache miss: " << get_cache_stats().cache_miss << endl;
        duration = chrono::high_resolution_clock::now() - start;
        cout << "Время выполнения с кэшем: " << duration.count() << " секунд.\n";


        free_all_cache_blocks();
        reset_cache_stats();
        cout<<"\n----------------------------------------\n\n\n";
    }
    if (test4) {
        char* filename1 = "cifri.txt";  // 6780
        char* filename2 = "bukvi.txt";  // 5761
        char* filename3 = "itoito.txt"; // 0
        char *buf = new char[100];

        cout<<"Тест #4 - Работа с несколькими файлами одновременно\n\n";

        int fd1 = lab2_open(filename1);
        int fd2 = lab2_open(filename2);
        int fd3 = lab2_open(filename3);

        lab2_lseek(fd1, get_rand_from_to(0, 6780), 0);
        lab2_read(fd1, buf, 100);

        lab2_write(fd3, buf, 50);

        lab2_lseek(fd2, get_rand_from_to(0, 5761), 0);
        lab2_read(fd2, buf, 100);

        lab2_write(fd3, buf, 100);

        lab2_fsync(fd3);
        lab2_close(fd1);
        lab2_close(fd2);
        lab2_close(fd3);
        cout << "Cache hits: " << get_cache_stats().cache_hits << ", Cache miss: " << get_cache_stats().cache_miss << endl;

        free_all_cache_blocks();
        reset_cache_stats();
        cout<<"\n----------------------------------------\n\n\n";
    }
    return 0;
}
