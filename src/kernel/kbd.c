/*
 *  opsys - A small, experimental operating system
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

#include <errno.h>

#include <ioports.h>

#include "kbd.h"

enum {
        IOPORT_CTRL = 0x64,
        IOPORT_ENCD = 0x60
};

enum {
        KBD_CTRL_CMD_RDCMDBYTE  = 0x20,
        KBD_CTRL_CMD_WRCMDBYTE  = 0x60,
        KBD_CTRL_CMD_SELFTEST   = 0xaa,
        KBD_CTRL_CMD_KBDITFTEST = 0xab,
        KBD_CTRL_CMD_DISABLEKBD = 0xad,
        KBD_CTRL_CMD_ENABLEKBD  = 0xae,
        KBD_CTRL_CMD_RDINPORT   = 0xc0,
        KBD_CTRL_CMD_RDOUTPOURT = 0xd0,
        KBD_CTRL_CMD_WROUTPORT  = 0xd1,
        KBD_CTRL_CMD_RDTESTIN   = 0xe0,
        KBD_CTRL_CMD_RESET      = 0xfe,
        KBD_CTRL_CMD_DISABLEAUX = 0xa7,
        KBD_CTRL_CMD_ENABLEAUX  = 0xa8,
        KBD_CTRL_CMD_AUXITFTEST    = 0xa9,
        KBD_CTRL_CMD_WRMOUSE    = 0xd4
};

enum {
        KBD_CTRL_CMDBYTE_KBDUSEIRQ = 1<<0,
        KBD_CTRL_CMDBYTE_AUXUSEIRQ = 1<<1
};

enum {
        KBD_CTRL_FLAGS_OUTBUF_FULL       = 1<<0,
        KBD_CTRL_FLAGS_INBUF_FULL        = 1<<1,
        KBD_CTRL_FLAGS_SELF_TEST_SUCCESS = 1<<2,
        KBD_CTRL_FLAGS_LASTCMD_IN        = 1<<3,
        KBD_CTRL_FLAGS_NOT_LOCKED        = 1<<4,
        KBD_CTRL_FLAGS_AUXBUF_FULL       = 1<<5,
        KBD_CTRL_FLAGS_TIMEOUT           = 1<<6,
        KBD_CTRL_FLAGS_PARERR            = 1<<7
};

static void
kbd_ctrl_inb(unsigned char *byte)
{
        io_inb(IOPORT_CTRL, byte);
}

static void
kbd_ctrl_outb(unsigned char byte)
{
        unsigned char state;

        /* wait for completion of previous command */
        do {
                kbd_ctrl_inb(&state);
        } while (state & KBD_CTRL_FLAGS_INBUF_FULL);

        io_outb(IOPORT_CTRL, byte);
}

static void
kbd_enc_inb(unsigned char *byte)
{
        io_inb(IOPORT_ENCD, byte);
}

static void
kbd_enc_outb(unsigned char byte)
{
        unsigned char state;

        /* wait for completion of previous command */
        do {
                kbd_ctrl_inb(&state);
        } while (state & KBD_CTRL_FLAGS_INBUF_FULL);

        io_outb(IOPORT_ENCD, byte);
}

static void
kbd_ctrl_outcmd(int cmd, unsigned char byte)
{
        kbd_ctrl_outb(cmd);

        kbd_enc_outb(byte);
}

static void
kbd_ctrl_incmd(int cmd, unsigned char *byte)
{
        unsigned char state;

        kbd_ctrl_outb(cmd);

        do {
                kbd_ctrl_inb(&state);
        } while (!(state & KBD_CTRL_FLAGS_OUTBUF_FULL));

        kbd_enc_inb(byte);
}

static int
kbd_selftest(void)
{
        unsigned char state;

        /* self test */

        kbd_ctrl_outb(KBD_CTRL_CMD_SELFTEST);

        do {
                kbd_ctrl_inb(&state);
        } while (!(state & KBD_CTRL_FLAGS_OUTBUF_FULL));

        kbd_enc_inb(&state);

        return (state==0x55) ? 0 : -ENODEV;
}

static int
kbd_interfacetest(void)
{
        unsigned char state;

        /* interface test */

        kbd_ctrl_outb(KBD_CTRL_CMD_KBDITFTEST);

        do {
                kbd_ctrl_inb(&state);
        } while (!(state & KBD_CTRL_FLAGS_OUTBUF_FULL));

        kbd_enc_inb(&state);

        return state ? -EIO : 0;
}

#include "console.h"

int
kbd_init()
{
        int err;
        unsigned char byte;

        kbd_ctrl_outb(KBD_CTRL_CMD_ENABLEKBD);

        /* test controller */

        if ((err = kbd_selftest()) < 0) {
                goto err_kbd_selftest;
        }

        /* test interface between keyboard and controller */

        if (kbd_interfacetest() < 0) {
        
                /* reset, try again */
                kbd_ctrl_outb(KBD_CTRL_CMD_DISABLEKBD);
                kbd_ctrl_outb(KBD_CTRL_CMD_ENABLEKBD);

                if ((err = kbd_interfacetest()) < 0) {
                        goto err_kbd_interfacetest;
                }
        }

        kbd_ctrl_incmd(KBD_CTRL_CMD_RDCMDBYTE, &byte);

        console_printf("command byte = %x.\n", (unsigned long)byte);

        kbd_ctrl_outcmd(KBD_CTRL_CMD_WRCMDBYTE, byte|
                                                KBD_CTRL_CMDBYTE_KBDUSEIRQ);

        kbd_ctrl_incmd(KBD_CTRL_CMD_RDCMDBYTE, &byte);

        console_printf("command byte = %x.\n", (unsigned long)byte);

        return 0;

err_kbd_interfacetest:
err_kbd_selftest:
        return err;
}

int
kbd_get_scancode()
{
        unsigned char byte;

        kbd_enc_inb(&byte);

        return byte;
}

void
kbd_irq_handler(unsigned char irqno)
{
        int scancode;

        scancode = kbd_get_scancode();

        console_printf("%s:%x: keyboard handler scancode=%x.\n", __FILE__,
                                                                 __LINE__,
                                                                 scancode);
}

