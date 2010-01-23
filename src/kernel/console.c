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

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <types.h>
#include "crt.h"

static size_t
console_hextostr(unsigned long v, char *str)
{
        static const char symbol[] = {'0','1','2','3','4','5','6','7',
                                      '8','9','a','b','c','d','e','f'};

        char *digit;
        size_t len;

        digit = str;

        if (v) {
                while (v) {
                        *(digit++) = symbol[v&0xf];
                        v >>= 4;
                }
        } else {
                *(digit++)  = '0';
        }

        *(digit++) = 'x';
        *(digit++) = '0';

        len = digit-str;

        *(digit--) = '\0';

        while (str < digit) {
                const char tmp = *str;
                *str = *digit;
                *digit = tmp;
                ++str;
                --digit;
        }

        return len;
}

int
console_printf(const char *str, ...)
{
        const char *strbeg;
        unsigned short row, col;
        size_t len;

        va_list args;

        va_start(args, str);

        for (strbeg=str; *str; ++str) {

                if (*str == '%') {

                        len = str-strbeg;
                        crt_getpos(&row, &col);
                        crt_write(crt_getaddress(row, col), strbeg, len, 0x07);
                        crt_setpos(row, col+len);
                        strbeg = ++str;

                        switch (*str) {
                        case 's':
                                {
                                        const char *a;

                                        a = va_arg(args, char*);

                                        len = strlen(a);

                                        crt_getpos(&row, &col);
                                        crt_write(crt_getaddress(row, col),
                                                  a, len, 0x07);
                                        crt_setpos(row, col+len);
                                        strbeg = str+1;
                                }
                                break;
                        case 'x':
                                {
                                        char astr[12];
                                        size_t len;
                                        unsigned long a;

                                        a = va_arg(args, unsigned long);

                                        len = console_hextostr(a, astr);

                                        crt_getpos(&row, &col);
                                        crt_write(crt_getaddress(row, col),
                                                  astr, len, 0x07);
                                        crt_setpos(row, col+len);
                                        strbeg = str+1;
                                }
                                break;
                        case '%':
                                break;
                        default:
                                break;
                        }

                } else if (*str == '\n') {

                        len = str-strbeg;

                        crt_getpos(&row, &col);
                        crt_write(crt_getaddress(row, col), strbeg, len, 0x07);
                        crt_setpos(row, col+len);
                        crt_getpos(&row, &col);
                        crt_setpos((row+1)%25, 0);

                        strbeg = str+1;

                } else if (*str == '\t') {

                        len = str-strbeg;

                        crt_getpos(&row, &col);
                        crt_write(crt_getaddress(row, col), strbeg, len, 0x07);
                        crt_setpos(row, col+len+8);

                        strbeg = str+1;
                }
        }

        va_end(args);

        len = str-strbeg;

        crt_getpos(&row, &col);
        crt_write(crt_getaddress(row, col), strbeg, len, 0x07);
        crt_setpos(row, col+len);

        return 0;
}

int
console_perror(const char *s, int err)
{
        if ((err < 0) || (err >= LAST_ERROR)) {
                return console_perror("console_perror", EINVAL);
        }

        return console_printf("%s: %s\n", s, sys_errlist[err]);
}

