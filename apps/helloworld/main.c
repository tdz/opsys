
#include <types.h>

void exit(int) __attribute__((noreturn));

void
crt_write(const char *str, size_t len)
{
        __asm__("pushl %1\n\t" /* push len */
                "pushl %0\n\t" /* push str */
                "pushl $1\n\t" /* push syscall number */
                "int $0x80\n\t"
                "addl $12, %%esp\n\t" /* restore stack pointer */
                        :
                        : "r"(str), "r"(len));
}

int
main(int argc, char **argv)
{
        crt_write("Hello world", 11);

        exit(0);
}

