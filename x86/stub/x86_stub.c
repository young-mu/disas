#include <stdio.h>

void x86_stub(void)
{
    /* insertion point */
    asm("nop\n\t");
    asm("nop\n\t");
    asm("nop\n\t");
    asm("nop\n\t");
    asm("nop\n\t");
}

int main(int argc, const char *argv[])
{
    x86_stub();
    return 0;
}
