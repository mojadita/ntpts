/* ntpts.c -- program to convert timestamps from NTP to UNIX.
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sun Dec 11 02:11:17 2016 +0200
 * Copyright: (C) 2016-2021 Luis Colorado.  All rights reserved.
 * License: BSD
 */

#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include <langinfo.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define NTP_OFFSET                (2208988800UL)
#define FRAC_PARTS                (1000000000UL)

#define F(x)        __FILE__":%d:%s: "x,__LINE__,__func__

char *months[12];

void process(const struct timespec *now);

int main(int argc, char **argv)
{
    struct timespec now;
    int i;

    /* locale info */
    setlocale(LC_ALL, "");

    argc--; argv++;
    if (argc) {
        int i;
        for(i = 0; i < argc; i++) {
            char digits[16] = "0123456789abcdef";
            int base = 10;
            int neg = 0;
            char *p = argv[i], *q;

            now.tv_sec  = 0;
            now.tv_nsec = 0;

            /* skip spaces */
            while (isspace(*p)) p++;

            /* neg */
            if (*p == '-') {
                neg = 1;
                p++;
            }

            /* base */
            if (*p == '0') {
                base = 8; p++;
                switch (*p) {
                case 'x': case 'X': base = 16; p++; break;
                case '.': base = 10; break;
                case 'b': case 'B': base = 2; p++; break;;
                }
            }

            digits[base] = 0;

            /* integer part */
            while(*p && (q = strchr(digits, tolower(*p)))) {
                now.tv_sec *= base;
                now.tv_sec += q - digits;
                p++;
            }


            /* fractional part */
            if (*p++ == '.') {
                int i;
                uint64_t quot = 0;
                uint64_t div = 1;
                int top = (base == 2) ? 30 : (base == 8) ? 10 : (base == 10) ? 9 : /* (base == 16) */ 8;
                for(i = 0; *p && (i < top) && (q = strchr(digits, tolower(*p))); i++) {
                    quot *= base;
                    div *= base;
                    if ((q = strchr(digits, tolower(*p))) != NULL) {
                        quot += q - digits;
                        p++;
                    }
                }
                quot *= FRAC_PARTS; quot /= div;
                now.tv_nsec = quot;
            }
            if (neg) {
                now.tv_sec = - 1 - now.tv_sec;
                now.tv_nsec = FRAC_PARTS - now.tv_nsec;
                if (now.tv_nsec >= FRAC_PARTS) {
                    now.tv_nsec -= FRAC_PARTS;
                    now.tv_sec++;
                }
            }
            process(&now);
        }
    } else {
        int res = clock_gettime (CLOCK_REALTIME, &now);
        if (res < 0) {
            fprintf(stderr,
                    F("clock_gettime: %s(errno = %d)\n"),
                    strerror(errno), errno);
            exit(1);
        }
        process(&now);
    }
    return EXIT_SUCCESS;
} /* main */

void process(const struct timespec *now)
{
    struct tm *tm;

    uint64_t ntp_sec = now->tv_sec + NTP_OFFSET;
    uint64_t ntp_frac = ((uint64_t) now->tv_nsec << 32) / FRAC_PARTS;

#define PFX "%9s: "

#define Q(f, s, integ, frac) do {              \
        printf(F(PFX f "\n"), s,               \
            (unsigned long long) integ,        \
            (unsigned long) frac);             \
    } while (0)

    Q("%llu.%09lu", "NTP(dec)", ntp_sec, now->tv_nsec);
    Q("%#llx.%08lx", "NTP(hex)", ntp_sec, ntp_frac);
    Q("%llu.%09lu", "UNIX(dec)", now->tv_sec, now->tv_nsec);
    Q("%#llx.%08lx", "UNIX(hex)", now->tv_sec, ntp_frac);

    printf(F(PFX"%lu/%#x\n"), "UNIX",
            (unsigned long) now->tv_sec,
            (unsigned) now->tv_sec);

#define P(x) do {                                               \
        tm = x(&now->tv_sec);                                   \
        printf(F(PFX"%d/%s/%d, %02d:%02d:%02d.%09lu %s\n"),     \
            #x,                                                 \
            tm->tm_mday, nl_langinfo(ABMON_1 + tm->tm_mon),     \
            tm->tm_year + 1900, tm->tm_hour, tm->tm_min,        \
            tm->tm_sec, now->tv_nsec, tm->tm_zone);             \
    } while(0)

    P(gmtime);
    P(localtime);

#undef P
#undef PFX

} /* process */
