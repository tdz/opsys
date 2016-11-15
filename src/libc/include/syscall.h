/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2009-2010  Thomas Zimmermann
 *  Copyright (C) 2016       Thomas Zimmermann
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
 * \return 0 on success, or a negative error value otherwise
 * \retval -EAGAIN resource is temporarily not available
 * \retval -EINVAL the flags argument is invalid
 * \retval -ESRCH the specified receiver thread does not exist
 * \retval -ETIMEDOUT the receiver thread did not respond within the specified timeout
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
 * The exact register layout of an IPC message is shown in the table below.
 *
 * <table>
 *  <caption>IPC register layout</caption>
 *  <tr>
 *   <td>bits</td><td align=center>31</td><td align=center>30</td><td align=center>29</td><td align=center>28</td>
 *                <td align=center>27</td><td align=center>26</td><td align=center>25</td><td align=center>24</td>
 *                <td align=center>23</td><td align=center>22</td><td align=center>21</td><td align=center>20</td>
 *                <td align=center>19</td><td align=center>18</td><td align=center>17</td><td align=center>16</td>
 *                <td align=center>15</td><td align=center>14</td><td align=center>13</td><td align=center>12</td>
 *                <td align=center>11</td><td align=center>10</td><td align=center> 9</td><td align=center> 8</td>
 *                <td align=center> 7</td><td align=center> 6</td><td align=center> 5</td><td align=center> 4</td>
 *                <td align=center> 3</td><td align=center> 2</td><td align=center> 1</td><td align=center> 0</td>
 *  </tr>
 *  <tr>
 *   <td>REG0</td><td align=center colspan=16>receiver task id</td>
 *                <td align=center colspan=16>receiver thread id</td>
 *  </tr>
 *  <tr>
 *   <td>REG1</td><td align=center colspan=20>timeout</td>
 *                <td align=center colspan=2>opcode</td>
 *                <td align=center colspan=4 bgcolor=#c0c0c0>reserved</td>
 *                <td align=center colspan=1>0</td>
 *                <td align=center colspan=1>0</td>
 *                <td align=center colspan=4>user flags</td>
 *  </tr>
 *  <tr>
 *   <td>REG2</td><td align=center colspan=32>message word 0</td>
 *  </tr>
 *  <tr>
 *   <td>REG3</td><td align=center colspan=32>message word 1</td>
 *  </tr>
 * </table>
 *
 * The IPC mechanism is also used to map memory pages among address
 * spaces. Therefore, the flags IPC_FLAG_MMAP has to be set. Msg0
 * contains the index of the first page to map, msg1 contains the
 * number of pages to map. The receiver thread has to be read to receive
 * memory mappings. The sender can also specify access rights of
 * how map the pages. The sender can only grant right that it
 * has itself.
 *
 * The register layout for memory mappings is shown in the table below.
 *
 * <table>
 *  <caption>Mmap register layout</caption>
 *  <tr>
 *   <td>bits</td><td align=center>31</td><td align=center>30</td><td align=center>29</td><td align=center>28</td>
 *                <td align=center>27</td><td align=center>26</td><td align=center>25</td><td align=center>24</td>
 *                <td align=center>23</td><td align=center>22</td><td align=center>21</td><td align=center>20</td>
 *                <td align=center>19</td><td align=center>18</td><td align=center>17</td><td align=center>16</td>
 *                <td align=center>15</td><td align=center>14</td><td align=center>13</td><td align=center>12</td>
 *                <td align=center>11</td><td align=center>10</td><td align=center> 9</td><td align=center> 8</td>
 *                <td align=center> 7</td><td align=center> 6</td><td align=center> 5</td><td align=center> 4</td>
 *                <td align=center> 3</td><td align=center> 2</td><td align=center> 1</td><td align=center> 0</td>
 *  </tr>
 *  <tr>
 *   <td>REG0</td><td align=center colspan=16>receiver task id</td>
 *                <td align=center colspan=16>receiver thread id</td>
 *  </tr>
 *  <tr>
 *   <td>REG1</td><td align=center colspan=20>timeout</td>
 *                <td align=center colspan=2>opcode</td>
 *                <td align=center colspan=4 bgcolor=#c0c0c0>reserved</td>
 *                <td align=center colspan=1>0</td>
 *                <td align=center colspan=1>1</td>
 *                <td align=center colspan=4>user flags</td>
 *  </tr>
 *  <tr>
 *   <td>REG2</td><td align=center colspan=20>first page index</td>
 *                <td align=center colspan=9 bgcolor=#c0c0c0>reserved</td>
 *                <td align=center colspan=3>access flags</td>
 *  </tr>
 *  <tr>
 *   <td>REG3</td><td align=center colspan=32>page count</td>
 *  </tr>
 * </table>
 *
 * Another form of IPC messages is the error message. This is used to signal error states in a coherent
 * way to a receiver thread. The message layout is shown below. The receiver can either be in IPC or in mmap
 * mode.
 *
 * <table>
 *  <caption>Error-message register layout</caption>
 *  <tr>
 *   <td>bits</td><td align=center>31</td><td align=center>30</td><td align=center>29</td><td align=center>28</td>
 *                <td align=center>27</td><td align=center>26</td><td align=center>25</td><td align=center>24</td>
 *                <td align=center>23</td><td align=center>22</td><td align=center>21</td><td align=center>20</td>
 *                <td align=center>19</td><td align=center>18</td><td align=center>17</td><td align=center>16</td>
 *                <td align=center>15</td><td align=center>14</td><td align=center>13</td><td align=center>12</td>
 *                <td align=center>11</td><td align=center>10</td><td align=center> 9</td><td align=center> 8</td>
 *                <td align=center> 7</td><td align=center> 6</td><td align=center> 5</td><td align=center> 4</td>
 *                <td align=center> 3</td><td align=center> 2</td><td align=center> 1</td><td align=center> 0</td>
 *  </tr>
 *  <tr>
 *   <td>REG0</td><td align=center colspan=16>receiver task id</td>
 *                <td align=center colspan=16>receiver thread id</td>
 *  </tr>
 *  <tr>
 *   <td>REG1</td><td align=center colspan=20>timeout</td>
 *                <td align=center colspan=2>opcode</td>
 *                <td align=center colspan=4 bgcolor=#c0c0c0>reserved</td>
 *                <td align=center colspan=1>1</td>
 *                <td align=center colspan=1>0/1</td>
 *                <td align=center colspan=4>user flags</td>
 *  </tr>
 *  <tr>
 *   <td>REG2</td><td align=center colspan=32>system-defined error value</td>
 *  </tr>
 *  <tr>
 *   <td>REG3</td><td align=center colspan=32 bgcolor=#c0c0c0>reserved</td>
 *  </tr>
 * </table>
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
