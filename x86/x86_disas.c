#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>

#define X86_STUB_OFFSET     (0x3e0+0x3)

void fillCode(unsigned char *dst, const char *src, int size)
{
    int i, h, l;
    for (i = 0; i < size; i++) {
        if (isxdigit(src[0]) && isxdigit(src[1])) {
            h = isdigit(src[0]) ? (src[0] - '0') : (src[0] - 'a' + 10);
            l = isdigit(src[1]) ? (src[1] - '0') : (src[1] - 'a' + 10);
            *dst++ = h * 16 + l;
            src += 2;
        } else {
            fprintf(stderr, "%c%c is NOT hex!\n", src[0], src[1]);
            exit(1);
        }
    }
}

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
    char filepath[100];
    int cn;
    unsigned char *code = NULL;

    if (argc == 1) {
        fprintf(stderr, "Usage:\n1) ./x86_disas f <file>\n2) ./x86_disas <bincode>\n");
        exit(1);
    } else if (argc == 2 || argc == 3) {

        if (argc == 2) { /* bincode */
            assert(strlen(argv[1]) % 2 == 0);
            code_sz = strlen(argv[1]) >> 1;
            code = (unsigned char*)malloc(code_sz);
            fillCode(code, argv[1], code_sz);
        } else if (argc == 3 && *argv[1] == 'f') { /* binfile */
            strcpy(filepath, argv[2]);

            /* get code size */
            code_sz = getFileSize(filepath);
            if (code_sz == -1) {
                fprintf(stderr, "fail to getFileSize\n");
                exit(1);
            }
            code = (unsigned char*)malloc(code_sz);

            /* read code */
            fd_src = open(filepath, O_RDONLY);
            if (fd_src == -1) {
                fprintf(stderr, "fail to open %s\n", filepath);
                exit(1);
            }
            read_sz = read(fd_src, code, code_sz);
            if (read_sz != code_sz) {
                fprintf(stderr, "fail to read %s\n", filepath);
                exit(1);
            }

            close(fd_src);
        }

        /* generate x86_stub */
        system("sed -i '/asm/d' ./stub/x86_stub.c");
        for (cn = 0; cn < code_sz; cn++) {
            system("sed -i '/insertion point/a\\    asm(\"nop\\\\n\\\\t\");' ./stub/x86_stub.c");
        }
        system("gcc ./stub/x86_stub.c -m32 -o ./x86_stub");

        /* patch code into stub */
        fd_stub = open("./x86_stub", O_RDWR, 0);
        if (fd_stub == -1) {
            fprintf(stderr, "fail to open x86_stub\n");
            exit(1);
        }
        lseek(fd_stub, X86_STUB_OFFSET, SEEK_SET);
        write_sz = write(fd_stub, code, code_sz);
        if (write_sz != code_sz) {
            fprintf(stderr, "fail to write x86_stub\n");
            exit(1);
        }

        free(code);
        close(fd_stub);
    }

    /* generate assembly */
    system("objdump -d x86_stub \
           | sed -n '/<x86_stub>:/,/<main>:/{/<x86_stub>:/n; /^$/b; /<main>:/d; s/^ /0x/g; p}' \
           | sed -n '3,$p' | head -n -2 \
           | awk --non-decimal-data 'BEGIN {FS=\":\"} {$1 = $1 - 0x80483e3; print $0}'");

    return 0;
}
