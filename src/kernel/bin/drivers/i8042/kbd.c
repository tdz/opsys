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

#include "kbd.h"
#include <errno.h>
#include "idt.h"
#include "ioports.h"
#include "irq.h"

enum {
    IOPORT_CTRL = 0x64,
    IOPORT_ENCD = 0x60
};

enum kbd_ctrl_cmd {
    KBD_CTRL_CMD_RDCMDBYTE = 0x20,
    KBD_CTRL_CMD_WRCMDBYTE = 0x60,
    KBD_CTRL_CMD_SELFTEST = 0xaa,
    KBD_CTRL_CMD_KBDITFTEST = 0xab,
    KBD_CTRL_CMD_DISABLEKBD = 0xad,
    KBD_CTRL_CMD_ENABLEKBD = 0xae,
    KBD_CTRL_CMD_RDINPORT = 0xc0,
    KBD_CTRL_CMD_RDOUTPOURT = 0xd0,
    KBD_CTRL_CMD_WROUTPORT = 0xd1,
    KBD_CTRL_CMD_RDTESTIN = 0xe0,
    KBD_CTRL_CMD_RESET = 0xfe,
    KBD_CTRL_CMD_DISABLEAUX = 0xa7,
    KBD_CTRL_CMD_ENABLEAUX = 0xa8,
    KBD_CTRL_CMD_AUXITFTEST = 0xa9,
    KBD_CTRL_CMD_WRMOUSE = 0xd4
};

enum {
    KBD_CTRL_CMDBYTE_KBDUSEIRQ = 1 << 0,
    KBD_CTRL_CMDBYTE_AUXUSEIRQ = 1 << 1,
    KBD_CTRL_CMDBYTE_DISABLE_KBD = 1 << 4,
    KBD_CTRL_CMDBYTE_DISABLE_AUX = 1 << 5,
    KBD_CTRL_CMDBYTE_SCANCODE_XT = 1 << 6
};

enum {
    KBD_CTRL_FLAGS_OUTBUF_FULL = 1 << 0,
    KBD_CTRL_FLAGS_INBUF_FULL = 1 << 1,
    KBD_CTRL_FLAGS_SELF_TEST_SUCCESS = 1 << 2,
    KBD_CTRL_FLAGS_LASTCMD_IN = 1 << 3,
    KBD_CTRL_FLAGS_NOT_LOCKED = 1 << 4,
    KBD_CTRL_FLAGS_AUXBUF_FULL = 1 << 5,
    KBD_CTRL_FLAGS_TIMEOUT = 1 << 6,
    KBD_CTRL_FLAGS_PARERR = 1 << 7
};

enum kbd_enc_cmd {
    KBD_ENCD_CMD_SET_LED = 0xed,
    KBD_ENCD_CMD_ECHO = 0xee,
    KBD_ENCD_CMD_SET_SCANCODESET = 0xf0,
    KBD_ENCD_CMD_SEND_ID = 0xf2,
    KBD_ENCD_CMD_SET_AUTOREPEAT = 0xf3,
    KBD_ENCD_CMD_SET_ENABLE_KBD = 0xf4,
    KBD_ENCD_CMD_RESET_WAIT = 0xf5,
    KBD_ENCD_CMD_RESET_SCAN = 0xf6,
    KBD_ENCD_CMD_ALL_AUTOREPEAT = 0xf7,
    KBD_ENCD_CMD_ALL_MAKEBREAK = 0xf8,
    KBD_ENCD_CMD_ALL_MAKE = 0xf9,
    KBD_ENCD_CMD_ALL_AUTOREPMB = 0xfa,
    KBD_ENCD_CMD_SETKEY_AUTOREPEAT = 0xfb,
    KBD_ENCD_CMD_SETKEY_MAKEBREAK = 0xfc,
    KBD_ENCD_CMD_SETKEY_BREAK = 0xfd,
    KBD_ENCD_CMD_RESEND = 0xfe,
    KBD_ENCD_CMD_RESET_KBD = 0xff
};

/* I/O port interface
 */

static unsigned char
kbd_ctrl_inb(void)
{
    return io_inb(IOPORT_CTRL);
}

static void
kbd_ctrl_outb(unsigned char byte)
{
    /* wait for completion of previous command */
    while (kbd_ctrl_inb() & KBD_CTRL_FLAGS_INBUF_FULL);

    io_outb(IOPORT_CTRL, byte);
}

static unsigned char
kbd_enc_inb(void)
{
    /* wait for input to appear in buffer */
    while (!(kbd_ctrl_inb() & KBD_CTRL_FLAGS_OUTBUF_FULL));

    return io_inb(IOPORT_ENCD);
}

static void
kbd_enc_outb(unsigned char byte)
{
    /* wait for completion of previous command */
    while (kbd_ctrl_inb() & KBD_CTRL_FLAGS_INBUF_FULL);

    io_outb(IOPORT_ENCD, byte);
}

/* command interface
 */

static void
kbd_ctrl_outcmd(enum kbd_ctrl_cmd cmd, unsigned char byte)
{
    kbd_ctrl_outb(cmd);
    kbd_enc_outb(byte);
}

static unsigned char
kbd_ctrl_incmd(enum kbd_ctrl_cmd cmd)
{
    kbd_ctrl_outb(cmd);
    return kbd_enc_inb();
}

#ifdef ENCCMD
static void
kbd_enc_outcmd(enum kbd_enc_cmd cmd, unsigned char byte)
{
    kbd_enc_outb(cmd);
    kbd_enc_outb(byte);
}

static unsigned char
kbd_enc_incmd(enum kbd_enc_cmd cmd)
{
    kbd_enc_outb(cmd);
    return kbd_enc_inb();
}
#endif

/* high level interface */

static int
kbd_selftest(void)
{
    /* self test */
    return (kbd_ctrl_incmd(KBD_CTRL_CMD_SELFTEST) == 0x55) ? 0 : -ENODEV;
}

static int
kbd_interfacetest(void)
{
    /* interface test */
    return kbd_ctrl_incmd(KBD_CTRL_CMD_KBDITFTEST) ? -EIO : 0;
}

#include "console.h"

static enum irq_status
irq_handler_func(unsigned char irqno, struct irq_handler* irqh)
{
    int scancode = kbd_get_scancode();

    console_printf("%s:%x: keyboard handler scancode=%x.\n",
                   __FILE__, __LINE__, scancode);

    return IRQ_HANDLED;
}

static struct irq_handler g_irq_handler;

int
kbd_init()
{
    /* enable keyboard */
    kbd_ctrl_outb(KBD_CTRL_CMD_ENABLEKBD);

    /* test controller */

    int res = kbd_selftest();
    if (res < 0) {
        return res;
    }

    /* test interface between keyboard and controller */

    if (kbd_interfacetest() < 0) {

        /* reset, try again */
        kbd_ctrl_outb(KBD_CTRL_CMD_DISABLEKBD);
        kbd_enc_outb(KBD_ENCD_CMD_RESET_KBD);
        kbd_ctrl_outb(KBD_CTRL_CMD_ENABLEKBD);

        res = kbd_interfacetest();
        if (res < 0) {
            goto err_kbd_interfacetest;
        }
    }

    irq_handler_init(&g_irq_handler, irq_handler_func);
    install_irq_handler(1, &g_irq_handler);

    /* enable interrupts, set XT scancode set */

    kbd_ctrl_outcmd(KBD_CTRL_CMD_WRCMDBYTE,
                    (kbd_ctrl_incmd(KBD_CTRL_CMD_RDCMDBYTE) |
                     KBD_CTRL_CMDBYTE_KBDUSEIRQ |
                     KBD_CTRL_CMDBYTE_DISABLE_AUX) &
                    ~(KBD_CTRL_CMDBYTE_DISABLE_KBD |
                      KBD_CTRL_CMDBYTE_SCANCODE_XT));

    return 0;

err_kbd_interfacetest:
    return res;
}

int
kbd_get_scancode()
{
    return io_inb(IOPORT_ENCD);
}
