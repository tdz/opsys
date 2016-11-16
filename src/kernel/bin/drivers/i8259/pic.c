/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2010  Thomas Zimmermann
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

#include "pic.h"
#include <stdint.h>
#include "ioports.h"

void
pic_install()
{
        __asm__(/* out ICW 1 */
                "mov $0x11, %%al\n\t"
                "out %%al, $0x20\n\t" "out %%al, $0xa0\n\t"
                /* out ICW 2 */
                "mov $0x20, %%al\n\t"
                "out %%al, $0x21\n\t"
                "mov $0x28, %%al\n\t" "out %%al, $0xa1\n\t"
                /* out ICW 3 */
                "mov $0x04, %%al\n\t"
                "out %%al, $0x21\n\t"
                "mov $0x02, %%al\n\t" "out %%al, $0xa1\n\t"
                /* out ICW 4 */
                "mov $0x01, %%al\n\t" "out %%al, $0x21\n\t" "out %%al, $0xa1\n\t"
                :
                :
                : "al");
}

/* signal end of interupt to PIC */
void
pic_eoi(unsigned char intno)
{
    static const uint8_t ioport[2] = {
        0x20,
        0xa0
    };

    if (intno > 15) {
        return; /* silently ignore software interrupts */
    }

    io_outb(ioport[intno > 7], 0x20);
}
