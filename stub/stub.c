/* Do *NOT* edit this file */
#include <stdio.h>

void stub_func(void)
{
    /* insertion point */
    asm("ssnop\n\t");
}

int main(int argc, const char *argv[])
{
    stub_func();
    return 0;
}
