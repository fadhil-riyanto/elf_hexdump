/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) Fadhil Riyanto <me@fadev.org>
 * this header map all function on elf64_hexdump.c
 */

#ifndef ELF64_HEXDUMP_H
#define ELF64_HEXDUMP_H

#include <elf.h>
#include <stdint.h>

#ifdef __clang__
#ifndef __hot
#define __hot __attribute__((hot))
#endif
#ifndef __cold
#define __cold __attribute__((cold))
#endif
#else /* __clang__ */
#ifndef __cold
#define __cold __attribute__((__cold__))
#endif

#ifndef __hot
#define __hot __attribute__((__hot__))
#endif
#endif

struct config {
        char *filename;
        uint8_t show_header;
        uint8_t show_header_struct;
        uint8_t hexdump;
        uint8_t show_program_header; /* table */
        uint8_t show_program_header_struct;
        uint8_t show_section_header;
        char *lookup_section_name;

        /*
         * add more in future
         */
};

__cold static void __debug_config(struct config *config);
void __print_process_elf_type(unsigned short e_type);
void __print_machine(int em);
void __print_elf_version(unsigned int elf_version);
void __print_elf_section_header_type(Elf64_Word sh_type);
__cold static void __print_elf64_hdr(Elf64_Ehdr *ehdr, struct config *config);
__cold static void __print_elf32_hdr(Elf32_Ehdr *ehdr, struct config *config);
__cold static void __print_ph_table_header();
__cold static void __print_sh_table_header();
__cold static void __print_p_type(Elf32_Word p_type);
__cold static void __print_p_flags(Elf64_Word p_flags);
__cold static void __print_elf64_ph_table(Elf64_Phdr *data, Elf64_Half e_phnum);
__cold static void __print_elf32_ph_table(Elf32_Phdr *data, Elf64_Half e_phnum);
__cold static void __print_elf64_sh_table(int fd, Elf64_Ehdr *ehdr_data,
                                          Elf64_Shdr *data);
static int __open_file(const char *filename);
__cold static enum ELF_arch_type read_elf_magic(int fd);
__cold static void interpret_elf64_hdr(int fd, Elf64_Ehdr *preallocated_hdr);
__cold static void interpret_elf32_hdr(int fd, Elf32_Ehdr *preallocated_hdr);
__cold static Elf64_Phdr *
interpret_elf64_program_header(int fd, Elf64_Off elf_start, Elf64_Half e_phnum);
__cold static Elf32_Phdr *
interpret_elf32_program_header(int fd, Elf32_Off elf_start, Elf32_Half e_phnum);
__cold static Elf64_Shdr *
interpret_elf64_section_header(int fd, Elf64_Off e_shoff, Elf64_Half e_shnum);
__cold static Elf32_Shdr *
interpret_elf32_section_header(int fd, Elf32_Off e_shoff, Elf32_Half e_shnum);
static int parse_opt(int argc, char *argv[], struct config *config);
__hot static int __get_file_n(int fd);
__hot static int _start_hexdump(int fd);
__hot static char *_resolve_e_shstrndx(int fd, Elf64_Half e_shstrndx,
                                       Elf64_Half e_shentsize,
                                       Elf64_Word sh_name, Elf64_Off e_shoff,
                                       char *dst);
static char *_resolve_e_shstrndx32(int fd, Elf32_Half e_shstrndx,
                                   Elf32_Half e_shentsize, Elf32_Word sh_name,
                                   Elf32_Off e_shoff, char *dst);

#endif /* ELF64_HEXDUMP_H */