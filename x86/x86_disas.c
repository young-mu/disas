#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>

typedef enum _Arch {
    i386 = 0, x86_64
} Arch;

#define X86_STUB_OFFSET_i386        (0x3ed+0x3)
#define X86_STUB_OFFSET_x86_64      (0x4ed+0x4)

void printUsage(void)
{
    fprintf(stdout, "Usages:\n");
    fprintf(stdout, "1) ./x86_disas f [filepath] [arch]\n");
    fprintf(stdout, "2) ./x86_disas [bincode] [arch]\n");
    fprintf(stdout, "[arch]: i386|32|x86_64|64\n\n");
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "1) ./x86_disas f ./genbin/bin 64\n");
    fprintf(stdout, "2) ./x86_disas 5589e5 i386\n");
}

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
    int ret;
    struct stat file_stats;

    ret = stat(path, &file_stats);
    if (ret == -1) {
        fprintf(stderr, "fail to getFileSize\n");
        exit(1);
    }

    return file_stats.st_size;
}

int main(int argc, const char *argv[])
{
    Arch arch;
    int stub_offset;
    int fd_src, fd_stub;
    int code_sz;
    int read_sz, write_sz;
    char filepath[100];
    int cn;
    unsigned char *code = NULL;

    if (argc == 3 || argc == 4) {
        if (strcmp(argv[argc-1], "i386") == 0 || strcmp(argv[argc-1], "32") == 0) {
            arch = i386;
            stub_offset = X86_STUB_OFFSET_i386;
        } else if (strcmp(argv[argc-1], "x86_64") == 0 || strcmp(argv[argc-1], "64") == 0) {
            arch = x86_64;
            stub_offset = X86_STUB_OFFSET_x86_64;
        } else {
            fprintf(stderr, "no arch argument\n");
            exit(1);
        }

        if (argc == 3) { /* bincode */
            assert(strlen(argv[1]) % 2 == 0);

            /* get code size */
            code_sz = strlen(argv[1]) >> 1;
            code = (unsigned char*)malloc(code_sz);

            /* get code */
            fillCode(code, argv[1], code_sz);
        } else if (argc == 4 && *argv[1] == 'f') { /* binfile */
            strcpy(filepath, argv[2]);

            /* get code size */
            code_sz = getFileSize(filepath);
            code = (unsigned char*)malloc(code_sz);

            /* get code */
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
        } else {
            printUsage();
            exit(1);
        }
    } else {
        printUsage();
        exit(1);
    }


    /* generate x86_stub */
    system("sed -i '/asm/d' ./stub/x86_stub.c");
    for (cn = 0; cn < code_sz; cn++) {
        system("sed -i '/insertion point/a\\    asm(\"nop\\\\n\\\\t\");' ./stub/x86_stub.c");
    }

    if (arch == i386) {
        system("gcc ./stub/x86_stub.c -m32 -o ./x86_stub");
    } else if (arch == x86_64) {
        system("gcc ./stub/x86_stub.c -m64 -o ./x86_stub");
    }

    /* patch code into stub */
    fd_stub = open("./x86_stub", O_RDWR, 0);
    if (fd_stub == -1) {
        fprintf(stderr, "fail to open x86_stub\n");
        exit(1);
    }
    lseek(fd_stub, stub_offset, SEEK_SET);
    write_sz = write(fd_stub, code, code_sz);
    if (write_sz != code_sz) {
        fprintf(stderr, "fail to write x86_stub\n");
        exit(1);
    }

    free(code);
    close(fd_stub);

    /* generate assembly */
    if (arch == i386) {
        system("objdump -d x86_stub \
               | sed -n '/<x86_stub>:/,/<main>:/{/<x86_stub>:/n; /^$/b; /<main>:/d; s/^ /0x/g; p}' \
               | sed -n '3,$p' | head -n -2 \
               | awk --non-decimal-data 'BEGIN {FS=\":\"} {$1 = $1 - 0x80483f0; print $0}'");
    } else if (arch == x86_64) {
        system("objdump -d x86_stub \
               | sed -n '/<x86_stub>:/,/<main>:/{/<x86_stub>:/n; /^$/b; /<main>:/d; s/^  /0x/g; p}' \
               | sed -n '3,$p' | head -n -2 \
               | awk --non-decimal-data 'BEGIN {FS=\":\"} {$1 = $1 - 0x4004f1; print $0}'");
    }

    return 0;
}
