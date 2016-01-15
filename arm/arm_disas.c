#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>

typedef enum _Arch {
    armv7 = 0, armv8
} Arch;

#define ARM_STUB_OFFSET_v7        (0x2ec)
#define ARM_STUB_OFFSET_v8        (0x5a8)

void printUsage(void)
{
    fprintf(stdout, "Usages:\n");
    fprintf(stdout, "1) ./arm_disas f [filepath] [arch]\n");
    fprintf(stdout, "2) ./arm_disas [bincode] [arch]\n");
    fprintf(stdout, "[arch]: armv7|32|armv8|64\n\n");
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "1) ./arm_disas f ./genbin/bin 64\n");
    fprintf(stdout, "2) ./arm_disas e92d4800 armv7\n");
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

void convertEndian(unsigned char *code, int inst_sz)
{
    int i;
    unsigned char tmp;
    for (i = 0; i < inst_sz; i++) {
        tmp = code[0 + i * 4];
        code[0 + i * 4] = code[3 + i * 4];
        code[3 + i * 4] = tmp;
        tmp = code[1 + i * 4];
        code[1 + i * 4] = code[2 + i * 4];
        code[2 + i * 4] = tmp;
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

#ifdef DEBUG
    printf("file [%s] size: %d\n", path, (int)file_stats.st_size);
#endif
    return file_stats.st_size;
}

int main(int argc, const char *argv[])
{
    Arch arch;
    int stub_offset;
    int fd_src, fd_stub;
    int code_sz;
    int inst_sz;
    int read_sz, write_sz;
    char filepath[100];
    int cn;
    unsigned char *code = NULL;

    if (argc == 3 || argc == 4) {
        if (strcmp(argv[argc-1], "armv7") == 0 || strcmp(argv[argc-1], "32") == 0) {
            arch = armv7;
            stub_offset = ARM_STUB_OFFSET_v7;
        } else if (strcmp(argv[argc-1], "armv8") == 0 || strcmp(argv[argc-1], "64") == 0) {
            arch = armv8;
            stub_offset = ARM_STUB_OFFSET_v8;
        } else {
            fprintf(stderr, "no arch argument\n");
            exit(1);
        }

        if (argc == 3) { /* bincode */
            /* get code size */
            code_sz = strlen(argv[1]) >> 1;
            assert(code_sz % 4 == 0);
            inst_sz = code_sz / 4;
#ifdef DEBUG
            printf("instruction size: %d\n", inst_sz);
#endif
            code = (unsigned char*)malloc(code_sz);

            /* get code */
            fillCode(code, argv[1], code_sz);
            convertEndian(code, inst_sz);
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

    /* generate arm_stub */
    system("sed -i '/asm/d' ./stub/arm_stub.c");
    for (cn = 0; cn < inst_sz; cn++) {
        system("sed -i '/insertion point/a\\    asm(\"nop\\\\n\\\\t\");' ./stub/arm_stub.c");
    }

    if (arch == armv7) {
        system("arm-linux-androideabi-gcc --sysroot=/home/young/Android/android-ndk-r10e/platforms/android-21/arch-arm ./stub/arm_stub.c -o ./arm_stub");
    } else if (arch == armv8) {
        system("aarch64-linux-android-gcc --sysroot=/home/young/Android/android-ndk-r10e/platforms/android-21/arch-arm64 ./stub/arm_stub.c -o ./arm_stub");
    }

    /* patch code into stub */
    fd_stub = open("./arm_stub", O_RDWR, 0);
    if (fd_stub == -1) {
        fprintf(stderr, "fail to open arm_stub\n");
        exit(1);
    }
    lseek(fd_stub, stub_offset, SEEK_SET);
    write_sz = write(fd_stub, code, code_sz);
    if (write_sz != code_sz) {
        fprintf(stderr, "fail to write arm_stub\n");
        exit(1);
    }

    free(code);
    close(fd_stub);

    /* generate assembly */
    if (arch == armv7) {
        system("arm-linux-androideabi-objdump -d arm_stub \
               | sed -n '/<arm_stub>:/,/<main>:/{/<arm_stub>:/n; /^$/b; /<main>:/d; p}' \
               | sed -n '3,$p' | head -n -3 \
               | awk 'BEGIN {FS=\":\"; n = 1} {$1 = n++; print $0}'");
    } else if (arch == armv8) {
        system("aarch64-linux-android-objdump -d arm_stub \
               | sed -n '/<arm_stub>:/,/<main>:/{/<arm_stub>:/n; /^$/b; /<main>:/d; p}' \
               | head -n -1 \
               | awk 'BEGIN {FS=\":\"; n = 1} {$1 = n++; print $0}'");
    }

    return 0;
}
