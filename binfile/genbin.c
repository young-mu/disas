#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define NELEM(x)    ((int)(sizeof(x) / sizeof(x[0])))

int main(int argc, const char *argv[])
{
    /* i386 */
    //char code[] = {0x55, 0x89, 0xe5};
    /* x86_64 */
    //char code[] = {0x55, 0x48, 0x89, 0xe5};
    /* armv7 */
    //char code[] = {0x00, 0x48, 0x2d, 0xe9};
    /* armv8 */
    //char code[] = {0xfd, 0x03, 0x00, 0x91};
    /* mips */
    //char code[] = {0x21, 0xf0, 0xa0, 0x03};
    /* mips64 */
    char code[] = {0x2d, 0xf0, 0xa0, 0x03};

    int fd;
    fd = open("./bin", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd != -1) {
        write(fd, (char*)code, NELEM(code));
    } else {
        fprintf(stderr, "fail to open\n");
        exit(1);
    }
    close(fd);
    return 0;
}
