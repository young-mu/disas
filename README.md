# disas
This is a **diassembler** that **turns binary code into assembly**, which supports architectures of i386/x86_64, armv7/armv8 and mips/mips64.

The principle is very simple:

1. generate source stub with fixed-number inline-assembly nop instructions as place-holder
2. compile source stub into exec stub
3. patch the codes into place-holder of exec stub
4. dump the exec stub and print assembly of patched codes

**REQUIREMENT**  
This project is based on Android NDK (compiler/binutils), so before using it, you should set NDK root directory in NDK.env first.

The input can be a **binfile** (dumped from gdb, etc.) or **codes**.  
Here is some examples:
<pre><code>
\# ./disas --code 5589e5 --arch i386
0   55          push   %ebp
1   89 e5       mov    %esp, %ebp
\# ./disas --code 554889e5 --arch x86_64
0   55          push   %rbp
1   48 89 e5    mov    %rsp, %rbp
\# ./disas --code e92d4800 --arch armv7
0   e92d4800    push   {fp, lr}
\# ./disas --code 910003fd --arch armv8
0   910003fd    mov    x29, sp
\# ./disas --code 03a0f021 --arch mips
0   03a0f021    move   sp, sp
\# ./disas --code 03a0f02d --arch mips64
0   03a0f02d    move   s8, sp
\# ./disas --file ./binfile/bin --arch mips64
0   03a0f02d    move   s8, sp
</pre></code>
