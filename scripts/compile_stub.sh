#!/bin/bash

# compile stub.c to binary exec stub

ROOT=$(cd `dirname $0`; cd ..; pwd)
SRC_STUB=${ROOT}/stub/stub.c
EXE_STUB=${ROOT}/stub/stub

source ${ROOT}/NDK.env
if [ -z ${NDKROOT} ]; then
    echo "NDKROOT should be set in ${ROOT}/NDK.env"
    exit 1
fi

if [ ! -d ${NDKROOT} ]; then
    echo "${NDKROOT} does not exsit"
    exit 1
fi

ARCH=${1}
case $ARCH in
    "i386")
        COMPILER=${NDKROOT}/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-gcc
        SYSROOT=${NDKROOT}/platforms/android-21/arch-x86
        ;;
    "x86_64")
        COMPILER=${NDKROOT}/toolchains/x86_64-4.9/prebuilt/linux-x86_64/bin/x86_64-linux-android-gcc
        SYSROOT=${NDKROOT}/platforms/android-21/arch-x86_64
        ;;
    "armv7")
        COMPILER=${NDKROOT}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc
        SYSROOT=${NDKROOT}/platforms/android-21/arch-arm
        ;;
    "armv8")
        COMPILER=${NDKROOT}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-gcc
        SYSROOT=${NDKROOT}/platforms/android-21/arch-arm64
        ;;
    "mips")
        COMPILER=${NDKROOT}/toolchains/mipsel-linux-android-4.9/prebuilt/linux-x86_64/bin/mipsel-linux-android-gcc
        SYSROOT=${NDKROOT}/platforms/android-21/arch-mips
        ;;
    "mips64")
        COMPILER=${NDKROOT}/toolchains/mips64el-linux-android-4.9/prebuilt/linux-x86_64/bin/mips64el-linux-android-gcc
        SYSROOT=${NDKROOT}/platforms/android-21/arch-mips64
        ;;
    *)
        echo "wrong ARCH:" $ARCH
        exit 1
esac

${COMPILER} --sysroot=${SYSROOT} ${SRC_STUB} -o ${EXE_STUB}
