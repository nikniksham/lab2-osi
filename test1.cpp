#include <iostream>
#include "cache/lab2.h"

using namespace std;

int main() {
    bool test1 = true;
    if (test1) {
        cout<<"Тест #1 - простое чтение и запись одного и того же большого блока данных\n";
        int fd = lab2_open("test.txt");
        char *buf = new char[35000];
        for (int i = 0; i < 100; ++i) {
            lab2_read(fd, buf, 35000);
            lab2_lseek(fd, 0, 0);
        }
        lab2_close(fd);
        cout << "Cache hits: " << get_cache_stats().cache_hits << ", Cache miss: " << get_cache_stats().cache_miss
             << endl;

        free_all_cache_blocks();
        reset_cache_stats();
    }
//    cout<<buf<<endl;
    return 0;
}
