#ifndef _DISASSEMBLER_H
#define _DISASSEMBLER_H_

#include <iostream>
using namespace std;

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>

#ifdef DEBUG
    #define DBG 1
#else
    #define DBG 0
#endif

#define DLOG(sw, TAG, fmt, args...)                 \
    do {                                            \
        if (sw) {                                   \
            printf("["TAG"] " fmt "\n", ##args);     \
        }                                           \
    } while (0)
#define ELOG(fmt, args...)  DLOG(1, "ERROR", fmt, ##args)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

// do NOT change the sequence
typedef enum _Arch {
    none
    , i386
    , x86_64
    , armv7
    , armv8
    , mips
    , mips64
} Arch;

typedef enum _Endian {
    little_endian = 0
    , big_endian
} Endian;

uint32_t const stub_offset_values[] = {
    0
    , 0x382 // i386
    , 0x3fa // x86_64
    , 0x2ec // armv7
    , 0x5a8 // armv8
    , 0x5ac // mips
    , 0x97c // mips64
};

class Disassembler {
public:
    explicit Disassembler(char *code, char *file, Arch arch, Endian endian);
    ~Disassembler(void);
    void generateStub(void);
    void compileStub(void);
    void patchStub(void);
    void dumpStub(void);
private:
    string archName[7];
    Arch arch;
    Endian endian;
    uint32_t stub_offset;
    uint32_t code_sz;
    uint32_t inst_sz;
    char *code;
    char *file;
    void convertToLittleEndian(char *code, uint32_t inst_sz);
    void fillCode(char *dst, const char *src, uint32_t code_sz);
    int getFileSize(const char *path);
};

void printUsage(const char *exec);
Disassembler* processOptsAndGetDisassembler(int argc, char *argv[]);

#endif
