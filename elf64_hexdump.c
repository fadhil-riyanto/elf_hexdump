/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) Fadhil Riyanto <me@fadev.org>
 */


#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

static struct option long_options[] = {
        {"file", 1, 0, 'f'}, 
        {"elf", 1, 0, 'e'},
        NULL
};

struct config {
        char *filename;
        u_int8_t elf;
        /*
         * add more in future
         */
};

static int elf64_open_file(const char *filename) {
        int fd = open(filename, O_RDONLY);

        if (fd < 0) {
                perror("open()");
                return -1;
        }

        return fd;
}

static int parse_opt(int argc, char *argv[], struct config *config) {
        int retval = 0;
        char opt = 0;
        int index = 0;

        u_int64_t conv_optarg = 0;

        while (1) {
                opt = getopt_long(argc, argv, "f:", long_options, &index);

                if (opt == -1) {
                        break;
                }

                switch (opt) {
                case '?':
                        break;

                case 'f':
                        config->filename = optarg;
                        break;

                case 'e':
                        conv_optarg = strtoul(optarg, NULL, 0);

                        if (conv_optarg == EINVAL) {
                                fprintf(stderr, "EINVAL: aboorting strtoul()");
                                break;
                        }

                        if (conv_optarg == ERANGE) {
                                fprintf(stderr, "ERANGE: strtoul() return out-of-range integer, use 1 for true, or 0 for false instead");
                                break;
                        }

                        if (conv_optarg > 0xFF || conv_optarg != 0 && conv_optarg != 1) {
                                fprintf(stderr, "ELF option \"%ld\" option is illegal, use 1 or 0, defaulting 0", conv_optarg);
                                config->elf = 0;
                        } else {
                                config->elf = conv_optarg;
                        }
                        break;
                
                }

                
        }

        return retval;
}

int main(int argc, char **argv) {
        struct config config;
        int ret = parse_opt(argc, argv, &config);

        // ret = elf64_open_file("./ap");
        printf("%d", ret);
}
