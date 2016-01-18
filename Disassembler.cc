#include "Disassembler.h"

Disassembler::Disassembler(char *code, char *file, Arch arch, Endian endian)
{
    this->arch = arch;
    DLOG(DBG, "OBJ", "arch: %d", this->arch);
    this->stub_offset = stub_offset_values[arch];
    DLOG(DBG, "OBJ", "stub_offset: %x", this->stub_offset);

    this->endian = endian;
    DLOG(DBG, "OBJ", "endian: %d", this->endian);

    if (code != NULL) {
        this->code_sz = (strlen(code) >> 1);
        DLOG(DBG, "OBJ", "code_sz: %d", this->code_sz);

        this->code = new char[this->code_sz];
        fillCode(this->code, code, this->code_sz);

        // RISC, it has been LE already for CISC
        if (arch != i386 && arch != x86_64) {
            this->inst_sz = (this->code_sz >> 2);
            DLOG(DBG, "OBJ", "inst_sz: %d", this->inst_sz);
            if (this->endian == little_endian) {
                convertToLittleEndian(this->code, this->inst_sz);
            }
        }
    }

    if (file != NULL) {
        this->file = new char[strlen(file) + 1];
        strncpy(this->file, file, strlen(file) + 1);
        DLOG(DBG, "OBJ", "file: %s", this->file);

        this->code_sz = getFileSize(file);
        DLOG(DBG, "OBJ", "code_sz: %d", this->code_sz);

        this->inst_sz = (this->code_sz >> 2);
        DLOG(DBG, "OBJ", "inst_sz: %d", this->inst_sz);

        this->code = new char[this->code_sz];
        int fd = open(file, O_RDONLY);
        if (fd == -1) {
            ELOG("fail to open %s", file);
            exit(1);
        }
        int read_sz = read(fd, this->code, this->code_sz);
        if (read_sz != code_sz) {
            ELOG("fail to read %s, read %d from %d bytes", file, read_sz, code_sz);
            exit(1);
        }
        close(fd);
    }

    if (this->arch == i386 || this->arch == x86_64) {
        for (int i = 0; i < this->code_sz; i++) {
            DLOG(DBG, "OBJ", "code[%d]: %x", i, (uint8_t)this->code[i]);
        }
    } else {
        for (int i = 0; i < this->inst_sz; i++) {
            DLOG(DBG, "OBJ", "code[%d]: %02x%02x%02x%02x", i,
                 (uint8_t)this->code[0 + i * 4],
                 (uint8_t)this->code[1 + i * 4],
                 (uint8_t)this->code[2 + i * 4],
                 (uint8_t)this->code[3 + i * 4]);
        }
    }

    this->archName[i386] = "i386";
    this->archName[x86_64] = "x86_64";
    this->archName[armv7] = "armv7";
    this->archName[armv8] = "armv8";
    this->archName[mips] = "mips";
    this->archName[mips64] = "mips64";
}

Disassembler::~Disassembler(void)
{
    if (this->code != NULL) { delete this->code; }
    if (this->file != NULL) { delete this->file; }
}

void Disassembler::fillCode(char *dst, const char *src, uint32_t code_sz)
{
    uint32_t i, high, low;
    for (i = 0; i < code_sz; i++) {
        high = isdigit(src[0]) ? (src[0] - '0') : (src[0] - 'a' + 10);
        low = isdigit(src[1]) ? (src[1] - '0') : (src[1] - 'a' + 10);
        *dst++ = high * 16 + low;
        src += 2;
    }
}

void Disassembler::convertToLittleEndian(char *code, uint32_t inst_sz)
{
    uint32_t i;
    char tmp;
    for (i = 0; i < inst_sz; i++) {
        DLOG(0, "ENDIAN", "%d before: %02x%02x%02x%02x", i,
             (uint8_t)code[0 + i * 4],
             (uint8_t)code[1 + i * 4],
             (uint8_t)code[2 + i * 4],
             (uint8_t)code[3 + i * 4]);
        tmp = code[0 + i * 4];
        code[0 + i * 4] = code[3 + i * 4];
        code[3 + i * 4] = tmp;
        tmp = code[1 + i * 4];
        code[1 + i * 4] = code[2 + i * 4];
        code[2 + i * 4] = tmp;
        DLOG(0, "ENDIAN", "%d after: %02x%02x%02x%02x", i,
             (uint8_t)code[0 + i * 4],
             (uint8_t)code[1 + i * 4],
             (uint8_t)code[2 + i * 4],
             (uint8_t)code[3 + i * 4]);
    }
}

int Disassembler::getFileSize(const char *file)
{
    int ret;
    struct stat file_stats;

    ret = stat(file, &file_stats);
    if (ret == -1) {
        ELOG("fail to stat %s", file);
        exit(1);
    }

    return file_stats.st_size;
}

void Disassembler::generateStub(void)
{
    string cmd = "./scripts/generate_stub.sh";
    char full_cmd[50];
    if (this->arch == i386 || this->arch == x86_64) {
        snprintf(full_cmd, sizeof(full_cmd), "%s %d %s", cmd.c_str(), this->code_sz, archName[this->arch].c_str());
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s %d %s", cmd.c_str(), this->inst_sz, archName[this->arch].c_str());
    }
    DLOG(DBG, "STUB", "generate stub command: %s", full_cmd);
    system(full_cmd);
}

void Disassembler::compileStub(void)
{
    string cmd = "./scripts/compile_stub.sh";
    char full_cmd[50];
    snprintf(full_cmd, sizeof(full_cmd), "%s %s", cmd.c_str(), archName[this->arch].c_str());
    DLOG(DBG, "STUB", "compile stub command: %s", full_cmd);
    system(full_cmd);
}

void Disassembler::patchStub(void)
{
    string stub = "./stub/stub";
    int fd = open(stub.c_str(), O_RDWR, 0);
    if (fd == -1) {
        ELOG("fail to open %s", stub.c_str());
        exit(1);
    }
    lseek(fd, this->stub_offset, SEEK_SET);
    int write_sz = write(fd, this->code, code_sz);
    if (write_sz != code_sz) {
        ELOG("fail to write %s, write %d of %d bytes", stub.c_str(), write_sz, code_sz);
        exit(1);
    }
    close(fd);
}

void Disassembler::dumpStub(void)
{
    string cmd = "./scripts/dump_stub.sh";
    char full_cmd[50];
    snprintf(full_cmd, sizeof(full_cmd), "%s %s", cmd.c_str(), archName[this->arch].c_str());
    DLOG(DBG, "STUB", "dump stub command: %s", full_cmd);
    system(full_cmd);
}
