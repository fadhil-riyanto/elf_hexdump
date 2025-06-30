#ifndef PRINT_PRETTY_H
#define PRINT_PRETTY_H

#include <stdio.h>
#ifndef PRINT_PRETTY_FILL_PAD
#define PRINT_PRETTY_FILL_PAD(pad_space, len)                                  \
        do {                                                                   \
                if (((long)pad_space - (long)len) < 0) {                       \
                        /* warn, text overflow */                              \
                } else {                                                       \
                        for (int i = 0; i < pad_space - len; i++) {            \
                                printf(" ");                                   \
                        }                                                      \
                }                                                              \
        } while (0)
#endif /* PRINT_PRETTY_FILL_PAD */

#ifdef USE_PRETTY_PRINT_PAD_COUNT
int __init_print_pad_count = 0;

#ifndef PRINT_PRETTYF_PAD_COUNT
#define PRINT_PRETTYF_PAD_COUNT(f, chr, len, pad_space)                        \
        do {                                                                   \
                printf(f, chr);                                                \
                PRINT_PRETTY_FILL_PAD(pad_space, len);                         \
                __init_print_pad_count = __init_print_pad_count + pad_space;   \
        } while (0)
#endif /* PRINT_PRETTYF_PAD_COUNT */

#ifndef PRINT_PRETTY_PAD_COUNT
#define PRINT_PRETTY_PAD_COUNT(chr, len, pad_space)                            \
        do {                                                                   \
                printf("%s", chr);                                             \
                PRINT_PRETTY_FILL_PAD(pad_space, len);                         \
                __init_print_pad_count = __init_print_pad_count + pad_space;   \
        } while (0)
#endif /* PRINT_PRETTY_PAD_COUNT */

#define PRETTY_PRINT_PAD_COUNT_RESET()                                         \
        do {                                                                   \
                __init_print_pad_count = 0;                                    \
        } while (0)

#endif /* USE_PRETTY_PRINT_PAD_COUNT */

#ifndef PRINT_PRETTY
#define PRINT_PRETTY(chr, len, pad_space)                                      \
        do {                                                                   \
                printf("%s", chr);                                             \
                PRINT_PRETTY_FILL_PAD(pad_space, len);                         \
        } while (0)
#endif /* PRINT_PRETTY */

#ifndef PRINT_PRETTYF
#define PRINT_PRETTYF(f, chr, len, pad_space)                                  \
        do {                                                                   \
                printf(f, chr);                                                \
                PRINT_PRETTY_FILL_PAD(pad_space, len);                         \
        } while (0)
#endif /* PRINT_PRETTYF */

#ifndef PRINT_PRETTYF_NUM
#define PRINT_PRETTYF_NUM(f, chr, pad_space)                                   \
        do {                                                                   \
                printf(f, chr);                                                \
                                                                               \
                /* Predict usage of char space */                              \
                char buf[64];                                                  \
                snprintf(buf, 64, f, chr);                                     \
                PRINT_PRETTY_FILL_PAD(pad_space, strlen(buf));                 \
        } while (0)
#endif /* PRINT_PRETTYF_NUM */

#ifndef PRINT_PRETTYF_NO_OVERFLOW
#define PRINT_PRETTYF_NO_OVERFLOW(f, chr, pad_space)                           \
        do {                                                                   \
                if (((long)pad_space - (long)strlen(chr)) < 0) {               \
                        for (int i = 0; i < pad_space - 2; i++) {              \
                                printf("%c", chr[i]);                          \
                        }                                                      \
                                                                               \
                        printf("  ");                                          \
                } else {                                                       \
                        PRINT_PRETTYF_NUM(f, chr, pad_space);                  \
                }                                                              \
        } while (0)
#endif /* PRINT_PRETTYF_NO_OVERFLOW */

#endif /* PRINT_PRETTY_H*/