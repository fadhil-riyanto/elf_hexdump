/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) Fadhil Riyanto <me@fadev.org>
 */

#include "hexdump.h"
#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef __clang__
#ifndef __hot
#define __hot __attribute__((hot))
#endif
#ifndef __cold
#define __cold __attribute__((cold))
#endif
#endif /* __clang__ */

#define SIZE(x, y) sizeof(x) / sizeof(y)

enum ELF_arch_type {
        ELF32,
        ELF64,
        NOT_ELF,
        UNDEFINED_ARCH /* need triage */
};

static char elf_magic[5] = { 0x7F, 0x45, 0x4C, 0x46, 0x0 };

/*
 * this is ELF related
 * ported from:
 * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/elf.h#n234
 */

/* 32-bit ELF base types. */
typedef u_int32_t Elf32_Addr;
typedef u_int16_t Elf32_Half;
typedef u_int32_t Elf32_Off;
typedef int32_t Elf32_Sword;
typedef u_int32_t Elf32_Word;
typedef u_int16_t Elf32_Versym;

/* 64-bit ELF base types. */
typedef u_int64_t Elf64_Addr;
typedef u_int16_t Elf64_Half;
typedef int16_t Elf64_SHalf;
typedef u_int64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef u_int32_t Elf64_Word;
typedef u_int64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;
typedef u_int16_t Elf64_Versym;

#define EI_NIDENT 16

typedef struct elf32_hdr {
        unsigned char e_ident[EI_NIDENT];
        Elf32_Half e_type;
        Elf32_Half e_machine;
        Elf32_Word e_version;
        Elf32_Addr e_entry; /* Entry point */
        Elf32_Off e_phoff;
        Elf32_Off e_shoff;
        Elf32_Word e_flags;
        Elf32_Half e_ehsize;
        Elf32_Half e_phentsize;
        Elf32_Half e_phnum;
        Elf32_Half e_shentsize;
        Elf32_Half e_shnum;
        Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct elf64_hdr {
        unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
        Elf64_Half e_type;
        Elf64_Half e_machine;
        Elf64_Word e_version;
        Elf64_Addr e_entry; /* Entry point virtual address */
        Elf64_Off e_phoff;  /* Program header table file offset */
        Elf64_Off e_shoff;  /* Section header table file offset */
        Elf64_Word e_flags;
        Elf64_Half e_ehsize;
        Elf64_Half e_phentsize;
        Elf64_Half e_phnum;
        Elf64_Half e_shentsize;
        Elf64_Half e_shnum;
        Elf64_Half e_shstrndx;
} Elf64_Ehdr;

static struct option long_options[] = { { "file", 1, 0, 'f' },
                                        { "elf", 1, 0, 'e' },
                                        { "hexdump", 0, 0, 'x' },
                                        NULL };

struct config {
        char *filename;
        u_int8_t elf;
        u_int8_t hexdump;
        /*
         * add more in future
         */
};

static int __open_file(const char *filename) {
        int fd = open(filename, O_RDONLY);

        if (fd < 0) {
                perror("open()");
                return -1;
        }

        return fd;
}

__cold static enum ELF_arch_type read_elf_magic(int fd) {
        u_int8_t buf[64];
        size_t bufsize = SIZE(buf, u_int8_t);
        memset(buf, 0, bufsize);

        read(fd, buf, bufsize);
        // VT_HEXDUMP(buf, bufsize);

        if (buf[0] == elf_magic[0] && buf[1] == elf_magic[1] &&
            buf[2] == elf_magic[2] && buf[3] == elf_magic[3]) {
                if (buf[4] == 1) {
                        return ELF32;
                } else if (buf[4] == 2) {
                        return ELF64;
                } else {
                        return UNDEFINED_ARCH;
                }
        } else {
                return NOT_ELF;
        }
}

__cold static void interpret_elf64_hdr(int fd, Elf64_Ehdr *preallocated_hdr) {
        u_int8_t *buf = (u_int8_t *)malloc(sizeof(Elf64_Ehdr));

        lseek(fd, 0, SEEK_SET);
        int ret = read(fd, buf, sizeof(Elf64_Ehdr));
        if (ret < 0) {
                perror("read()");
                return;
        }

        memcpy(preallocated_hdr, buf, sizeof(Elf64_Ehdr));

        free(buf);
}

__cold static void interpret_elf32_hdr(int fd, Elf32_Ehdr *preallocated_hdr) {
        u_int8_t *buf = (u_int8_t *)malloc(sizeof(Elf32_Ehdr));

        lseek(fd, 0, SEEK_SET);
        int ret = read(fd, buf, sizeof(Elf32_Ehdr));
        if (ret < 0) {
                perror("read()");
                return;
        }

        memcpy(preallocated_hdr, buf, sizeof(Elf32_Ehdr));

        free(buf);
}

static int parse_opt(int argc, char *argv[], struct config *config) {
        int retval = 0;
        char opt = 0;
        int index = 0;

        u_int64_t conv_optarg = 0;

        while (1) {
                opt = getopt_long(argc, argv, "f:e:x::", long_options, &index);

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
                                fprintf(stderr,
                                        "ERANGE: strtoul() return out-of-range "
                                        "integer, use 1 for true, or 0 for "
                                        "false instead");
                                break;
                        }

                        if (conv_optarg > 0xFF ||
                            conv_optarg != 0 && conv_optarg != 1) {
                                fprintf(stderr,
                                        "ELF option \"%ld\" option is illegal, "
                                        "use 1 or 0, defaulting 0",
                                        conv_optarg);
                                config->elf = 0;
                        } else {
                                config->elf = conv_optarg;
                        }
                        break;

                case 'x':
                        config->hexdump = 1;
                        break;
                }
        }

        return retval;
}

__hot static int __get_file_n(int fd) {

}

__cold static void __debug_config(struct config *config) {
        printf("config->filename: %s\n"
               "config->elf: %d\n"
               "config->hexdump: %d",
               config->filename, config->elf, config->hexdump);
}

__cold static void __print_elf64_hdr(Elf64_Ehdr *ehdr) {
        printf("Type:                             %u\n", ehdr->e_type);
        printf("Machine:                          %u\n", ehdr->e_machine);
        printf("Version:                          0x%x\n", ehdr->e_version);
        printf("Entry point address:              0x%016" PRIx64 "\n",
               ehdr->e_entry);
        printf("Start of program headers:         %" PRIu64
               " (bytes into file)\n",
               ehdr->e_phoff);
        printf("Start of section headers:         %" PRIu64
               " (bytes into file)\n",
               ehdr->e_shoff);
        printf("Flags:                            0x%x\n", ehdr->e_flags);
        printf("Size of this header:              %" PRIu16 " (bytes)\n",
               ehdr->e_ehsize);
        printf("Size of program headers:          %" PRIu16 " (bytes)\n",
               ehdr->e_phentsize);
        printf("Number of program headers:        %" PRIu16 "\n",
               ehdr->e_phnum);
        printf("Size of section headers:          %" PRIu16 " (bytes)\n",
               ehdr->e_shentsize);
        printf("Number of section headers:        %" PRIu16 "\n",
               ehdr->e_shnum);
        printf("Section header string table index:%" PRIu16 "\n",
               ehdr->e_shstrndx);
}

int main(int argc, char **argv) {
        struct config config;
        memset(&config, 0, sizeof(config));

        int ret = parse_opt(argc, argv, &config);
        __debug_config(&config);

        ret = __open_file(config.filename);
        if (ret < 0) {
                return -1;
                // VT_HEXDUMP(ehdr, sizeof(Elf64_Ehdr));
                // asm volatile("nop");
                // printf(const char *restrict format, ...)
        }
        if (config.elf) {
                
                int elf_arch_type = read_elf_magic(ret);
                
                if (elf_arch_type == ELF64) {
                        Elf64_Ehdr *ehdr = (Elf64_Ehdr *)malloc(sizeof(Elf64_Ehdr));
                        interpret_elf64_hdr(ret, ehdr);
                        __print_elf64_hdr(ehdr);
                        free(ehdr);
                }

                if (elf_arch_type == ELF32) {
                        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
                        interpret_elf32_hdr(ret, ehdr);
                }

        }
        // __debug_config(&config);
}
