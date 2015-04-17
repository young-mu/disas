#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define NELEM(x)    ((int)(sizeof(x) / sizeof(x[0])))

int main(int argc, const char *argv[])
{
    char code[] = {0x55, 0x89, 0xe5, 0x5d, 0xc3};

    int fd;
    fd = open("./bin", O_RDWR | O_CREAT, 0666);
    if (fd != -1) {
        write(fd, (char*)code, NELEM(code));
    } else {
        fprintf(stderr, "fail to open\n");
        exit(1);
    }
    close(fd);
    return 0;
}
