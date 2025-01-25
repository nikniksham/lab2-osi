#ifndef LAB2_H
#define LAB2_H

#include <sys/types.h>

// Статистика работы кэша
struct CacheStats {
    int cache_hits, cache_miss;
};

CacheStats get_cache_stats();
void reset_cache_stats();
void free_all_cache_blocks();

int get_rand_from_to(int min, int max);

int lab2_open(const char *path);
int lab2_close(int fd);
ssize_t lab2_read(int fd, void *buf, size_t count);
ssize_t lab2_write(int fd, const void *buf, size_t count);
off_t lab2_lseek(int fd, off_t offset, int whence);
int lab2_fsync(int fd);

#endif
