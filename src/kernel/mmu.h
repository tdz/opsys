/*
 *  oskernel - A small experimental operating-system kernel
 *  Copyright (C) 2010  Thomas Zimmermann <tdz@users.sourceforge.net>
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

static __inline__ void
mmu_load(unsigned long reg)
{
        __asm__("movl %0, %%cr3\n\t"
                        :
                        : "r" (reg));
}

static __inline__ void
mmu_enable_paging(void)
{
        /* set cr0.pg */
        __asm__("movl %%cr0, %%eax\n\t"
                "or $0x80000000, %%eax\n\t"
                "movl %%eax, %%cr0\n\t"
                        :
                        :
                        : "eax");
}

static __inline__ void
mmu_disable_paging(void)
{
        /* clear cr0.pg */
        __asm__("movl %%cr0, %%eax\n\t"
                "and $0x7fffffff, %%eax\n\t"
                "movl %%eax, %%cr0\n\t"
                        :
                        :
                        : "eax");
}

static __inline__ void
mmu_flush_tlb(void)
{
        __asm__("movl %%cr3, %%eax\n\t"
                "movl %%eax, %%cr3\n\t"
                        :
                        :
                        : "eax");
}

static __inline__ void
mmu_flush_tlb_entry(const void *pfaddr)
{
        mmu_flush_tlb();
}

