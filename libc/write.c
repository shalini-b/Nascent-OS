#include <sys/syscall.h>
#include <unistd.h>
/*
int write() {
    int res;
    __asm__ __volatile__(
    "int $0x80;"
    : "=a" (res)
    : "0"(SYS_write)
    );
    return res;
}*/
