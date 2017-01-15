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

enum syscall_op {
    SYSCALL_OP_SEND, /**< send message to another thread */
    SYSCALL_OP_SEND_AND_WAIT, /**< send message to another thread and wait for its answer */
    SYSCALL_OP_RECV, /**< receive from any thread */
    SYSCALL_OP_REPLY_AND_RECV /**< replay to thread and receive from any thread */
};
