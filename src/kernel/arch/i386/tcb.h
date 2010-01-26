/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

enum thread_state {
        THREAD_STATE_ZOMBIE = 0, /* waiting for removal */
        THREAD_STATE_READY, /* ready to be executed */
        THREAD_STATE_BLOCKED /* blocked by some action */
};

struct tcb
{
        enum thread_state state;
        struct task *task; /* task of the thread */
        void *stack; /* stack base address */
        unsigned char id; /* task-local id */

        /* general-purpose register */
        unsigned long eax;
        unsigned long ebx;
        unsigned long ecx;
        unsigned long edx;
        
        /* index registers */
        unsigned long esi;
        unsigned long edi;
        unsigned long ebp;
        unsigned long esp;

        /* segment registers */
        unsigned short cs;
        unsigned short ds;
        unsigned short ss;
        unsigned short es;
        unsigned short fs;
        unsigned short gs;

        /* state registers */
        unsigned long cr0;
        unsigned long cr2;
        unsigned long cr3;
        unsigned long cr4;
        unsigned long eip;
        unsigned long eflags;
        unsigned long tr; /* task register */

        /* FPU registers */
        unsigned long st[8];

        /* MMX registers */
        unsigned long mm[8];

        /* SSE registers */
        unsigned long xmm[8];
        unsigned long mxcsr;

        /* debug registers */        
        unsigned long dr0;
        unsigned long dr1;
        unsigned long dr2;
        unsigned long dr3;
        unsigned long dr6;
        unsigned long dr7;
} __attribute__ (( aligned(256) ));

int
tcb_init_with_id(struct tcb *tcb,
                 struct task *task, unsigned char id, void *stack);

int
tcb_init(struct tcb *tcb, struct task *task, void *stack);

void
tcb_uninit(struct tcb *tcb);

void
tcb_set_state(struct tcb *tcb, enum thread_state state);

enum thread_state
tcb_get_state(const struct tcb *tcb);

int
tcb_is_runnable(const struct tcb *tcb);

int
tcb_set_ready_state_firsttime(struct tcb *tcb,
                              const void *ip,
                              unsigned char irqno);

void
tcb_set_ip(struct tcb *tcb, void *ip);

int
tcb_switch(struct tcb *tcb, const struct tcb *dst);

#ifdef COMMENT
/* save CPU registers in TCB */
void
tcb_save(struct tcb *tcb);

/* load CPU registers from TCB */
int
tcb_load(const struct tcb *tcb);
#endif

/*int
tcb_set_page_directory(struct tcb *tcb, const struct page_directory *pd);*/

int
tcb_init_regs(void *ip);

