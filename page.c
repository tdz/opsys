
#include "virtpage.h"

pd_entry
pd_entry_create(unsigned long physaddr, unsigned long flags)
{
        return (physaddr&0xfffffc00) | (flags&PDE_ALL_FLAGS);
}

unsigned long
pd_entry_get_address(pd_entry pde)
{
        return pde&0xfffffc00;
}

pt_entry
pt_entry_create(unsigned long physaddr, unsigned long flags)
{
        return (physaddr&0xfffffc00) | (flags&PTE_ALL_FLAGS);
}

unsigned long
pt_entry_get_address(pt_entry pte)
{
        return pte&0xfffffc00;
}

