#include "Disassembler.h"

int main(int argc, char *argv[])
{
    Disassembler* disas = processOptsAndGetDisassembler(argc, argv);
    disas->generateStub();
    disas->compileStub();
    disas->patchStub();
    disas->dumpStub();

    return 0;
}

void printUsage(const char *exec)
{
    cout << endl;
    cout << "Options:" << endl;
    cout << "-c|--code [code]" << endl;
    cout << "-f|--file [binfile]" << endl;
    cout << "-a|--arch [i386|x86_64|armv7|armv8|mips|mips64]" << endl;
    cout << "-e|--endian [little|big] (little endian is default)" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << exec << " --code 5589e5 --arch i386" << endl;
    cout << exec << " --file binfile/bin ---arch mips64" << endl;
}

Disassembler* processOptsAndGetDisassembler(int argc, char *argv[])
{
    Arch arch = none;
    Endian endian = little_endian;
    char *code = NULL;
    char *file = NULL;
    bool has_code = false;

    int opt;
    opterr = 0;
    const char *short_opts = "c:f:a:e:";
    const struct option long_opts[] = {
        {"code", required_argument, NULL, 'c'}
        , {"file", required_argument, NULL, 'f'}
        , {"arch", required_argument, NULL, 'a'}
        , {"endian", required_argument, NULL, 'e'}
        , {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch (opt) {
            case 'c':
                DLOG(DBG, "OPTION", "code: %s", optarg);
                if (has_code) {
                    ELOG("binfile has been designated");
                    goto bail;
                } else {
                    has_code = true;
                }
                if (strlen(optarg) & 0x00000001) {
                    ELOG("code must be even number");
                    goto bail;
                }
                code = optarg;
                while (*optarg) {
                    if (isxdigit(*optarg) == 0) {
                        ELOG("%s is not HEX character", optarg);
                        goto bail;
                    }
                    optarg ++;
                }
                break;
            case 'f':
                DLOG(DBG, "OPTION", "file: %s", optarg);
                if (has_code) {
                    ELOG("code has been designated");
                    goto bail;
                } else {
                    has_code = true;
                }
                file = optarg;
                if (access(optarg, F_OK) != 0) {
                    ELOG("%s does not existed", optarg);
                    goto bail;
                }
                break;
            case 'a':
                DLOG(DBG, "OPTION", "arch: %s", optarg);
                if (strcmp(optarg, "i386") == 0) {
                    arch = i386;
                } else if (strcmp(optarg, "x86_64") == 0) {
                    arch = x86_64;
                } else if (strcmp(optarg, "armv7") == 0) {
                    arch = armv7;
                } else if (strcmp(optarg, "armv8") == 0) {
                    arch = armv8;
                } else if (strcmp(optarg, "mips") == 0) {
                    arch = mips;
                } else if (strcmp(optarg, "mips64") == 0) {
                    arch = mips64;
                } else {
                    ELOG("invalid arch value: %s", optarg);
                    goto bail;
                }
                break;
            case 'e':
                DLOG(DBG, "OPTION", "endian: %s", optarg);
                if (strncmp(optarg, "little", 6) == 0) {
                    endian = little_endian;
                } else if (strncmp(optarg, "big", 3) == 0) {
                    endian = big_endian;
                } else {
                    ELOG("invalid endian value: %s", optarg);
                    goto bail;
                }
                break;
            default:
                ELOG("invalid option: %s", argv[optind - 1]);
                goto bail;
        }
    }

    if (has_code == false) {
        ELOG("you must designate code or file");
        goto bail;
    }

    if (arch == none) {
        ELOG("you must designate arch");
        goto bail;
    }

    if (arch == i386 || arch == x86_64) {
        if (endian != little_endian) {
            ELOG("big_endian can not be set when arch is i386 or x86_64");
            goto bail;
        }
    }

    return new Disassembler(code, file, arch, endian);

bail:
    printUsage(argv[0]);
    exit(1);
}
