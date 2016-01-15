/* Do *NOT* edit this file */
#include <stdio.h>

void arm_stub(void)
{
    /* insertion point */
    asm("nop\n\t");
}

int main(int argc, const char *argv[])
{
    arm_stub();
    return 0;
}
