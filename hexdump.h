/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) Fadhil Riyanto <fadhil.riyanto@gnuweeb.org>
 *
 * this version is inspired by Ammar Faizi's versions
 * ref: https://gist.githubusercontent.com/ammarfaizi2/e88a21171358b5092a3df412eeb80b2f/raw/59141b48f59b70b1e96208716c45b1703c56daa7/vt_hexdump.h
 */

#include <complex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef VT_HEXDUMP
#define VT_HEXDUMP(PTR, SIZE)                                           \
        size_t ptr_size = SIZE;                                         \
        unsigned char *realptr = (unsigned char*)PTR;                   \
        unsigned int initial_counter = 0;                               \
        printf("================= VT_HEXDUMP =================\n");     \
        printf("file\t\t: %s:%d\n", __FILE__, __LINE__);                \
        printf("func\t\t: %s\n", __FUNCTION__);                         \
        printf("addr\t\t: 0x%016lx\n", realptr);                        \
        printf("dump_size\t: %ld\n\n", ptr_size);                       \
                                                                        \
        for (int i = 0; i < (ptr_size / 16) + 1; i++) {                 \
                printf("|0x%016lx|", (uintptr_t)(realptr));             \
                                                                        \
                for (int i = 0; i < 16; i++) {                          \
                        if (i % 4 == 0 && i != 0) {                     \
                                printf("  ");                           \
                        }                                               \
                        if (i != 15) {                                  \
                                if (initial_counter <= ptr_size) {      \
                                        printf(" %2x", realptr[i]);     \
                                }                                       \
                        } else {                                        \
                                printf(" %2x ", realptr[i]);            \
                        }                                               \
                }                                                       \
                                                                        \
                printf(" | ");                                          \
                for (int i = 0; i < 16; i++) {                          \
                        if (initial_counter <= ptr_size - 1) {          \
                                printf("%c", realptr[i]);               \
                        } else {                                        \
                                printf(".");                            \
                        }                                               \
                        initial_counter = initial_counter + 1;          \
                }                                                       \
                printf(" | ");                                          \
                                                                        \
                printf("\n");                                           \
                realptr = realptr + 16;                                 \
        }
#endif                                                               
