#!/bin/bash

# generate stub.c with SIZE nop instructions
# SIZE is code_sz for CISC and inst_sz for RISC

ROOT=$(cd `dirname $0`; cd ..; pwd)
SRC_STUB=${ROOT}/stub/stub.c

SIZE=${1:-0}
ARCH=${2}

# delete all legacy asm("nop\n\t") codes;
sed -i '/asm/d' ${SRC_STUB}

# insert SIZE asm("nop\n\t") lines
for ((i = 0; i < ${SIZE}; i++))
do
case $ARCH in
    "i386"|"x86_64"|"armv7"|"armv8")
        sed -i '/insertion point/a\    asm("nop\\n\\t");' ${SRC_STUB}
        ;;
    "mips"|"mips64")
        sed -i '/insertion point/a\    asm("ssnop\\n\\t");' ${SRC_STUB}
        ;;
    *)
        echo "wrong ARCH:" $ARCH
        exit 1
esac
done

