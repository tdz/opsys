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

enum
{
        IPC_OPSYS_TASK_QUIT = 0,
};

enum
{
        IPC_MMAP_RD = 1<<1, /**< \brief map pages readable */
        IPC_MMAP_WR = 1<<2, /**< \brief map pages writeable */
        IPC_MMAP_EX = 1<<3 /**< \brief map pages executable */
};

enum
{
        IPC_TIMEOUT_NOW = 0, /**< \brief return immediately if receiver is not ready */
        IPC_TIMEOUT_NEVER = -1 /**< \brief never timeout */
};

enum
{
        IPC_OP_SEND = 0, /**< \brief execute a send call */
        IPC_OP_SEND_AND_WAIT = 1, /**< \brief execute a send call and wait for reply from sender */
        IPC_OP_RECV = 2, /**< \brief receive from any sender */
        IPC_OP_REPLY_AND_RECV = 3 /**< \brief reply to sender and receive from any sender */
};

/**
 * \brief executes a system call
 * \param rcv the threadid of the receiver thread
 * \param flags ipc flags
 * \param msg0 first message word
 * \param msg1 second message word
 * \param[out] reply_rcv the tread id o the receiver thread
 * \param[out] reply_flags flags of the reply
 * \param[out] reply_msg0 first reply message word
 * \param[out] reply_msg1 second reply message word
 * \return 0 on success, or a non-zero error value
 *
 * 32 rcv task 16 | 15 rcv thread 0
 *
 * 32 timeout 12 | 11 system flags 4 | 3 user flags 0
 *
 * 32 first msg word 0
 *
 * 32 second msg word 0
 *
 * This function executes a system call. System calls are implemented
 * as messages of inter-process communication (abbr. IPC). Each call
 * contains a
 * receiver thread, which handles the message. The flags attribute
 * controls several details of the IPC call, the timeout parameter
 * specifies a timeout in seconds or microseconds. Two message words
 * can be passed to the receiver thread at once.
 *
 * In case that a sender-specified timeout is reached without the
 * receiver handling the call, the function returns with -ETIMEDOUT.
 *
 * The IPC mechanism is also used to map memory pages among address
 * spaces. Therefore, the flags IPC_FLAG_MMAP has to be set. Msg0
 * contains the index of the first page to map, msg1 contains the
 * number of pages to map. The receiver thread has to be read to receive
 * memory mappings. The sender can also specify access rights of
 * how map the pages. The sender can only grant right that it
 * has itself.
 * 
 *
 *
 * \dot
 *  <table>
 *   <tcol>
 *    <tr>3</tr>
 *   </tcol>
 *  <table>
 * \enddot
 */
int
syscall(unsigned long rcv,
        unsigned long flags,
        unsigned long msg0,
        unsigned long msg1,
        unsigned long *reply_rcv,
        unsigned long *reply_flags,
        unsigned long *reply_msg0,
        unsigned long *reply_msg1);

/**
 * \brief executes a system call that does not return any values
 * \param rcv the threadid of the receiver thread
 * \param flags ipc flags
 * \param msg0 first message word
 * \param msg1 second message word
 * \return 0 on success, or a non-zero error value
 *
 * This function executes a system call that does not return a reply. The
 * parameters  are the same as for the generic version.
 */
int
syscall0(unsigned long rcv,
         unsigned long flags,
         unsigned long msg0,
         unsigned long msg1);

