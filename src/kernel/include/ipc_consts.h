/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2017  Thomas Zimmermann
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

#pragma once

enum ipc_msg_flags {
    IPC_MSG_FLAGS_RESERVED = 0xe0000000,
    IPC_MSG_FLAGS_MMAP     = 1<<17,
    IPC_MSG_FLAG_IS_ERRNO  = 1<<16
};

enum {
    IPC_OPSYS_TASK_QUIT = 0,
};

enum {
    IPC_MMAP_RD = 1<<1, /**< \brief map pages readable */
    IPC_MMAP_WR = 1<<2, /**< \brief map pages writeable */
    IPC_MMAP_EX = 1<<3 /**< \brief map pages executable */
};

enum {
    IPC_TIMEOUT_NOW = 0, /**< \brief return immediately if receiver is not ready */
    IPC_TIMEOUT_NEVER = -1 /**< \brief never timeout */
};
