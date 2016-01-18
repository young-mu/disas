#!/bin/bash

# dump stub to print assembly of patched codes

ROOT=$(cd `dirname $0`; cd ..; pwd)
SRC_STUB=${ROOT}/stub/stub.c
EXE_STUB=${ROOT}/stub/stub

source ${ROOT}/NDK.env
if [ -z ${NDKROOT} ]; then
    echo "NDKROOT should be set in ${ROOT}/NDK.env"
    exit 1
fi

ARCH=${1}
case $ARCH in
    "i386")
        OBJDUMP=${NDKROOT}/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-objdump
        FIRST_LINE_TO_SHOW=3
        LINES_CUT_FROM_END=2
        BASE=0x8048382
        ;;
    "x86_64")
        OBJDUMP=${NDKROOT}/toolchains/x86_64-4.9/prebuilt/linux-x86_64/bin/x86_64-linux-android-objdump
        FIRST_LINE_TO_SHOW=3
        LINES_CUT_FROM_END=2
        BASE=0x4003fa
        ;;
    "armv7")
        OBJDUMP=${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-objdump
        FIRST_LINE_TO_SHOW=3
        LINES_CUT_FROM_END=3
        BASE=0x82ec
        ;;
    "armv8")
        OBJDUMP=${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-objdump
        FIRST_LINE_TO_SHOW=0
        LINES_CUT_FROM_END=1
        BASE=0x4005a8
        ;;
    "mips")
        OBJDUMP=${NDKROOT}/toolchains/mipsel-linux-android-4.9/prebuilt/linux-x86_64/bin/mipsel-linux-android-objdump
        FIRST_LINE_TO_SHOW=4
        LINES_CUT_FROM_END=5
        BASE=0x805ac
        ;;
    "mips64")
        OBJDUMP=${NDKROOT}/toolchains/mips64el-linux-android-4.9/prebuilt/linux-x86_64/bin/mips64el-linux-android-objdump
        FIRST_LINE_TO_SHOW=4
        LINES_CUT_FROM_END=4
        BASE=0x12000097c
        ;;
    *)
        echo "wrong ARCH:" $ARCH
        exit 1
esac

${OBJDUMP} -d ${EXE_STUB} |
sed -n '/<stub_func>:/,/<main>:/{/<stub_func>:/n; /^$/b; /<main>:/d; s/^\s*/0x/g; p}' |
tail -n +${FIRST_LINE_TO_SHOW} | head -n -${LINES_CUT_FROM_END} |
awk -v base="${BASE}" --non-decimal-data 'BEGIN {FS=":"} {$1 = $1 - base; print $0}'
