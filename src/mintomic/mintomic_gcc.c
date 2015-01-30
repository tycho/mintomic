#include <mintomic/core.h>

// When compiling for ELF format, there are no leading underscores on C symbol names.
// For other formats, a leading underscore is expected.
// http://www.nasm.us/doc/nasmdoc9.html
#if __ELF__
   #define UND
#else
   #define UND "_"
#endif

#if MINT_COMPILER_GCC && MINT_CPU_ARM && (MINT_CPU_ARM_VERSION == 6) && MINT_CPU_ARM_THUMB  // ARMv6 Thumb mode

//----------------------------------------------------------------------------
// When compiling for ARMv6 in Thumb mode, the mcr, ldrex and strex instructions
// are not available. To use them, we need to switch temporarily to ARM (non-Thumb) mode
// via standalone functions. The nice thing about writing standalone code blocks
// this way is that can we generate non-Thumb assembly code, regardless of
// the compiler configuration, and it will be callable from any type of code (ARM or Thumb).
//----------------------------------------------------------------------------

__asm__(
"   .text\n"
"   .align 2\n"
"   .globl "UND"mint_thread_fence_acquire\n"
"   .globl "UND"mint_thread_fence_release\n"
"   .globl "UND"mint_thread_fence_seq_cst\n"
UND"mint_thread_fence_acquire:\n"
UND"mint_thread_fence_release:\n"
UND"mint_thread_fence_seq_cst:\n"
// Do we really need to set the source register to 0?
"   mov r0, #0\n"
"   mcr p15, 0, r0, c7, c10, 5\n"
"   bx lr\n"
);

__asm__(
"   .text\n"
"   .align  2\n"
"   .globl  "UND"mint_compare_exchange_strong_32_relaxed\n"
UND"mint_compare_exchange_strong_32_relaxed:\n"
"   push    {r4}\n"
"   mov     r4, r0\n"
"1:\n"
"   ldrex   r0, [r4]\n"
"   cmp     r0, r1\n"
"   bne     2f\n"
"   strex   r3, r2, [r4]\n"
"   cmp     r3, #0\n"
"   bne     1b\n"
"2:\n"
"   pop     {r4}\n"
"   bx      lr\n"
);

__asm__(
"   .text\n"
"   .align  2\n"
"   .globl  "UND"mint_fetch_add_32_relaxed\n"
UND"mint_fetch_add_32_relaxed:\n"
"   push    {r4}\n"
"   mov     r4, r0\n"
"1:\n"
"   ldrex   r0, [r4]\n"
"   adds    r2, r0, r1\n"
"   strex   r3, r2, [r4]\n"
"   cmp     r3, #0\n"
"   bne     1b\n"
"   pop     {r4}\n"
"   bx      lr\n"
);

__asm__(
"   .text\n"
"   .align  2\n"
"   .globl  "UND"mint_fetch_and_32_relaxed\n"
UND"mint_fetch_and_32_relaxed:\n"
"   push    {r4}\n"
"   mov     r4, r0\n"
"1:\n"
"   ldrex   r0, [r4]\n"
"   and     r2, r0, r1\n"
"   strex   r3, r2, [r4]\n"
"   cmp     r3, #0\n"
"   bne     1b\n"
"   pop     {r4}\n"
"   bx      lr\n"
);

__asm__(
"   .text\n"
"   .align  2\n"
"   .globl  "UND"mint_fetch_or_32_relaxed\n"
UND"mint_fetch_or_32_relaxed:\n"
"   push    {r4}\n"
"   mov     r4, r0\n"
"1:\n"
"   ldrex   r0, [r4]\n"
"   orr     r2, r0, r1\n"
"   strex   r3, r2, [r4]\n"
"   cmp     r3, #0\n"
"   bne     1b\n"
"   pop     {r4}\n"
"   bx      lr\n"
);

#endif  // ARMv6 Thumb mode
