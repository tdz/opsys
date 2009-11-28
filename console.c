
#include <stdarg.h>

#include "types.h"
#include "string.h"
#include "crt.h"

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
                        case 'd':
                                break;
                        case 'i':
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
                        crt_setpos(row+1, 0);

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

