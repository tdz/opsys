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

enum pit_counter
{
        PIT_COUNTER_TIMER = 0,
        PIT_COUNTER_DRAM  = 1,
        PIT_COUNTER_SPKR  = 2
};

enum pit_mode
{
        PIT_MODE_TERMINAL = 0x00,
        PIT_MODE_ONESHOT  = 0x01,
        PIT_MODE_RATEGEN  = 0x02,
        PIT_MODE_WAVEGEN  = 0x03,
        PIT_MODE_SWSTROBE = 0x04,
        PIT_MODE_HWSTROBE = 0x05
};

void
pit_install(enum pit_counter counter, unsigned long freq, enum pit_mode mode);

void
pit_irq_handler(unsigned char irqno);

