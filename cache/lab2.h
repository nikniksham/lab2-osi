#ifndef LAB2_H
#define LAB2_H

#include <sys/types.h>

// Статистика работы кэша
struct CacheStats {
    int cache_hits, cache_miss;
};

extern int get_cache_miss();
extern int get_cache_hit();

extern void reset_cache_stats();
extern void free_all_cache_blocks();

extern int get_rand_from_to(int min, int max);

extern int lab2_open(const char *path);
extern int lab2_close(int fd);
extern ssize_t lab2_read(int fd, void *buf, size_t count);
extern ssize_t lab2_write(int fd, const void *buf, size_t count);
extern off_t lab2_lseek(int fd, off_t offset, int whence);
extern int lab2_fsync(int fd);

#endif
