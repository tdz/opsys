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

#include <tid.h>

threadid_type
threadid_create(unsigned int taskid, unsigned char tcbid)
{
        return (taskid<<8) | (tcbid&0xff);
}

unsigned int
threadid_get_taskid(threadid_type tid)
{
        return tid>>8;
}

unsigned char
threadid_get_tcbid(threadid_type tid)
{
        return tid&0xff;
}

