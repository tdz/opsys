/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009  Thomas Zimmermann
 *  Copyright (C) 2016  Thomas Zimmermann
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

#include "interupt.h"
#include "cpu.h"
#include "drivers/i8259/pic.h"

/* clear interupts */
void
cli()
{
        __asm__("cli\n\t");
}

/* set interupts */
void
sti()
{
        __asm__("sti\n\t");
}

/* signal end of interupt to PIC */
void
eoi(unsigned char intno)
{
        pic_eoi(intno);
}

int
int_enabled()
{
        return !!(eflags() & EFLAGS_IF);
}
