#include <iostream>
#include "lab2.h"

using namespace std;

int main() {
//    int fd = lab2_open("test.txt");
    int fd = lab2_open("trash.txt");
    char* buf = new char[20000];
    lab2_read(fd, buf, 20000);
    cout<<"PADLA\n\n\n";
    lab2_lseek(fd, 0, 0);
    lab2_read(fd, buf, 20000);
//    lab2_lseek(fd, 0, 0);
//    lab2_read(fd, buf, 20000);
//    lab2_lseek(fd, 0, 0);
//    lab2_read(fd, buf, 20000);
//    lab2_lseek(fd, 0, 0);
//    lab2_read(fd, buf, 20000);
//    lab2_lseek(fd, 0, 0);
//    lab2_read(fd, buf, 20000);
    cout<<"Cache hits: "<<get_cache_stats().cache_hits<<", Cache miss: "<<get_cache_stats().cache_miss<<endl;
//    cout<<buf<<endl;
    return 0;
}
