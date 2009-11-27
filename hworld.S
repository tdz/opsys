
.text

.global _start

_start:

        # print

        movl $4, %eax  # (Syscall 4: write)
        movl $1, %ebx  # (stdout)
        movl $HW, %ecx # "Hello world!"
        movl $13, %edx # strlen("Hello world!")
        int $0x80

        # Exit

        movl $1, %eax
        movl $0, %ebx
        int $0x80

.data

HW: .ascii "Hello world!\n"

