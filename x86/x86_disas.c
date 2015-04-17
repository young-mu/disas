#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#define X86_STUB_OFFSET     (0x3e0+0x3)

#define NELEM(x)    ((int)(sizeof(x) / sizeof(x[0])))

int getFileSize(const char *path)
{
    struct stat file_stats;
    if (stat(path, &file_stats) != -1) {
        return file_stats.st_size;
    } else {
        return -1;
    }
}

int main(int argc, const char *argv[])
{
    int fd_src, fd_stub;
    int code_sz;
    int read_sz, write_sz;
    int cn;
    char *code = NULL;

    if (argc == 1) {
        fprintf(stderr, "Usage: ./x86_disas FILE\n");
        exit(1);
    } else if (argc == 2) {
        /* get code size */
        code_sz = getFileSize(argv[1]);
        if (code_sz == -1) {
            fprintf(stderr, "fail to getFileSize\n");
            exit(2);
        }
        code = (char*)malloc(code_sz);

        /* generate x86_stub */
        system("sed -i '/asm/d' ./stub/x86_stub.c");
        for (cn = 0; cn < code_sz; cn++) {
            system("sed -i '/insertion point/a\\    asm(\"nop\\\\n\\\\t\");' ./stub/x86_stub.c");
        }
        system("gcc ./stub/x86_stub.c -m32 -o ./x86_stub");

        /* read code */
        fd_src = open(argv[1], O_RDONLY);
        if (fd_src == -1) {
            fprintf(stderr, "fail to open %s\n", argv[1]);
            exit(3);
        }
        read_sz = read(fd_src, code, code_sz);
        if (read_sz != code_sz) {
            fprintf(stderr, "fail to read %s\n", argv[1]);
            exit(4);
        }

        /* patch code into stub */
        fd_stub = open("./x86_stub", O_RDWR, 0);
        if (fd_stub == -1) {
            fprintf(stderr, "fail to open x86_stub\n");
            exit(5);
        }
        lseek(fd_stub, X86_STUB_OFFSET, SEEK_SET);
        write_sz = write(fd_stub, code, code_sz);
        if (write_sz != code_sz) {
            fprintf(stderr, "fail to write x86_stub\n");
            exit(6);
        }

        free(code);
        close(fd_src);
        close(fd_stub);
    }

    system("objdump -d x86_stub \
           | sed -n '/<x86_stub>:/,/<main>:/{/<x86_stub>:/n; /^$/b; /<main>:/d; s/^ /0x/g; p}' \
           | sed -n '3,$p' | head -n -2 \
           | awk --non-decimal-data 'BEGIN {FS=\":\"} {$1 = $1 - 0x80483e3; print $0}'");
    return 0;
}
