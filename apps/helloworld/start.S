
.text

.global _start
.global exit

_start:
        int $0x80
        ret
        pushl $0 /* TODO: push argv */
        pushl $0 /* TODO: push argc */
        call main
        pushl %eax /* push exit status */
exit:
        /* when we call here, status should already be on the stack */
        pushl $0 /* push exit system-call number */
        int $0x80
        ret

