#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define NELEM(x)    ((int)(sizeof(x) / sizeof(x[0])))

int main(int argc, const char *argv[])
{
    /* armv7 */
    //char code[] = {0xe9, 0x2d, 0x48, 0x00};
    /* armv8 */
    char code[] = {0xa9, 0xbe, 0x7b, 0xfd};

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
