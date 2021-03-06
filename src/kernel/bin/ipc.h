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

#pragma once

struct ipc_msg;
struct tcb;

int
ipc_send(struct ipc_msg *msg, struct tcb *rcv);

int
ipc_send_and_wait(struct ipc_msg *msg, struct tcb *rcv);

int
ipc_recv(struct ipc_msg *msg, struct tcb *rcv);

int
ipc_reply_and_recv(struct ipc_msg *msg, struct tcb *rcv);

int
ipc_reply(struct ipc_msg *msg, struct tcb *rcv);
