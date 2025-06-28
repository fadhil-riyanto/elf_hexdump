/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) Fadhil Riyanto <me@fadev.org>
 */

#define USE_PRETTY_PRINT_PAD_COUNT

#include "getopt_custom.h"
#include "hexdump.h"
#include "print_pretty.h"
#include <asm-generic/errno-base.h>
#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __clang__
#ifndef __hot
#define __hot __attribute__((hot))
#endif
#ifndef __cold
#define __cold __attribute__((cold))
#endif
#endif /* __clang__ */

#define SIZE(x, y) sizeof(x) / sizeof(y)
// #define EI_NIDENT 16
#define FILE_BUFSIZE 4096 /* BYTES */

enum ELF_arch_type {
        ELF32,
        ELF64,
        NOT_ELF,
        UNDEFINED_ARCH /* need triage */
};

static char elf_magic[5] = { 0x7F, 0x45, 0x4C, 0x46, 0x0 };

struct file_off_control {
        u_int64_t offset;
        int n;
};

/* 0x3f is reserved */
static struct option long_options[] = {
        { "file", 1, 0, GETOPT_CUSTOM_FILE },
        { "header", 0, 0, GETOPT_CUSTOM_HEADER },
        { "ph", 0, 0, GETOPT_CUSTOM_PROGRAM_HEADER },
        { "header-struct", 0, 0, GETOPT_CUSTOM_HEADER_STRUCT },
        { "header-ph", 0, 0, GETOPT_CUSTOM_PROGRAM_HEADER_STRUCT },
        { "hexdump", 0, 0, GETOPT_CUSTOM_HEXDUMP },
        NULL
};

struct config {
        char *filename;
        u_int8_t show_header;
        u_int8_t show_header_struct;
        u_int8_t hexdump;
        u_int8_t show_program_header; /* table */
        u_int8_t show_program_header_struct;

        /*
         * add more in future
         */
};

__cold static void __debug_config(struct config *config) {
        printf("config->filename: %s\n"
               "config->show_header: %d\n"
               "config->show_header_struct: %d\n"
               "config->hexdump: %d"
               "config->show_program_header: %d",

               config->filename, config->show_header,
               config->show_header_struct, config->hexdump,
               config->show_program_header);
}

void __print_process_elf_type(unsigned short e_type) {
        switch (e_type) {
        case ET_NONE:
                printf("No file type (ET_NONE)");
                break;
        case ET_REL:
                printf("Relocatable file (ET_REL)");
                break;
        case ET_EXEC:
                printf("Executable file (ET_EXEC)");
                break;
        case ET_DYN:
                printf("Shared object file (ET_DYN)");
                break;
        case ET_CORE:
                printf("Core file (ET_CORE)");
                break;
        case ET_NUM:
                printf("Number of defined types (ET_NUM) - This should not be "
                       "a real file type.");
                break;
        case ET_LOOS:
                printf("OS-specific range start (ET_LOOS)");
                break;
        case ET_HIOS:
                printf("OS-specific range end (ET_HIOS)");
                break;
        case ET_LOPROC:
                printf("Processor-specific range start (ET_LOPROC)");
                break;
        case ET_HIPROC:
                printf("Processor-specific range end (ET_HIPROC)");
                break;
        default:
                // For OS-specific or Processor-specific types not explicitly
                // listed, or unknown types within these ranges.
                if (e_type >= ET_LOOS && e_type <= ET_HIOS) {
                        printf("OS-specific type (0x%hx, within range "
                               "0x%hx-0x%hx)",
                               e_type, ET_LOOS, ET_HIOS);
                } else if (e_type >= ET_LOPROC && e_type <= ET_HIPROC) {
                        printf("Processor-specific type (0x%hx, within range "
                               "0x%hx-0x%hx)",
                               e_type, ET_LOPROC, ET_HIPROC);
                } else {
                        printf("Unknown or reserved type (0x%hx)", e_type);
                }
                break;
        }
}

void __print_machine(int em) {
        switch (em) {
        case 0:
                printf("No machine");
                break;
        case 1:
                printf("AT&T WE 32100");
                break;
        case 2:
                printf("SUN SPARC");
                break;
        case 3:
                printf("Intel 80386");
                break;
        case 4:
                printf("Motorola m68k family");
                break;
        case 5:
                printf("Motorola m88k family");
                break;
        case 6:
                printf("Intel MCU");
                break;
        case 7:
                printf("Intel 80860");
                break;
        case 8:
                printf("MIPS R3000 big-endian");
                break;
        case 9:
                printf("IBM System/370");
                break;
        case 10:
                printf("MIPS R3000 little-endian");
                break;
        case 15:
                printf("HPPA");
                break;
        case 17:
                printf("Fujitsu VPP500");
                break;
        case 18:
                printf("Sun's \"v8plus\"");
                break;
        case 19:
                printf("Intel 80960");
                break;
        case 20:
                printf("PowerPC");
                break;
        case 21:
                printf("PowerPC 64-bit");
                break;
        case 22:
                printf("IBM S390");
                break;
        case 23:
                printf("IBM SPU/SPC");
                break;
        case 36:
                printf("NEC V800 series");
                break;
        case 37:
                printf("Fujitsu FR20");
                break;
        case 38:
                printf("TRW RH-32");
                break;
        case 39:
                printf("Motorola RCE");
                break;
        case 40:
                printf("ARM");
                break;
        case 41:
                printf("Digital Alpha");
                break;
        case 42:
                printf("Hitachi SH");
                break;
        case 43:
                printf("SPARC v9 64-bit");
                break;
        case 44:
                printf("Siemens Tricore");
                break;
        case 45:
                printf("Argonaut RISC Core");
                break;
        case 46:
                printf("Hitachi H8/300");
                break;
        case 47:
                printf("Hitachi H8/300H");
                break;
        case 48:
                printf("Hitachi H8S");
                break;
        case 49:
                printf("Hitachi H8/500");
                break;
        case 50:
                printf("Intel Merced");
                break;
        case 51:
                printf("Stanford MIPS-X");
                break;
        case 52:
                printf("Motorola Coldfire");
                break;
        case 53:
                printf("Motorola M68HC12");
                break;
        case 54:
                printf("Fujitsu MMA Multimedia Accelerator");
                break;
        case 55:
                printf("Siemens PCP");
                break;
        case 56:
                printf("Sony nCPU embedded RISC");
                break;
        case 57:
                printf("Denso NDR1 microprocessor");
                break;
        case 58:
                printf("Motorola Start*Core processor");
                break;
        case 59:
                printf("Toyota ME16 processor");
                break;
        case 60:
                printf("STMicroelectronic ST100 processor");
                break;
        case 61:
                printf("Advanced Logic Corp. Tinyj emb.fam");
                break;
        case 62:
                printf("AMD x86-64 architecture");
                break;
        case 63:
                printf("Sony DSP Processor");
                break;
        case 64:
                printf("Digital PDP-10");
                break;
        case 65:
                printf("Digital PDP-11");
                break;
        case 66:
                printf("Siemens FX66 microcontroller");
                break;
        case 67:
                printf("STMicroelectronics ST9+ 8/16 mc");
                break;
        case 68:
                printf("STmicroelectronics ST7 8 bit mc");
                break;
        case 69:
                printf("Motorola MC68HC16 microcontroller");
                break;
        case 70:
                printf("Motorola MC68HC11 microcontroller");
                break;
        case 71:
                printf("Motorola MC68HC08 microcontroller");
                break;
        case 72:
                printf("Motorola MC68HC05 microcontroller");
                break;
        case 73:
                printf("Silicon Graphics SVx");
                break;
        case 74:
                printf("STMicroelectronics ST19 8 bit mc");
                break;
        case 75:
                printf("Digital VAX");
                break;
        case 76:
                printf("Axis Communications 32-bit emb.proc");
                break;
        case 77:
                printf("Infineon Technologies 32-bit emb.proc");
                break;
        case 78:
                printf("Element 14 64-bit DSP Processor");
                break;
        case 79:
                printf("LSI Logic 16-bit DSP Processor");
                break;
        case 80:
                printf("Donald Knuth's educational 64-bit proc");
                break;
        case 81:
                printf("Harvard University machine-independent object files");
                break;
        case 82:
                printf("SiTera Prism");
                break;
        case 83:
                printf("Atmel AVR 8-bit microcontroller");
                break;
        case 84:
                printf("Fujitsu FR30");
                break;
        case 85:
                printf("Mitsubishi D10V");
                break;
        case 86:
                printf("Mitsubishi D30V");
                break;
        case 87:
                printf("NEC v850");
                break;
        case 88:
                printf("Mitsubishi M32R");
                break;
        case 89:
                printf("Matsushita MN10300");
                break;
        case 90:
                printf("Matsushita MN10200");
                break;
        case 91:
                printf("picoJava");
                break;
        case 92:
                printf("OpenRISC 32-bit embedded processor");
                break;
        case 93:
                printf("ARC International ARCompact");
                break;
        case 94:
                printf("Tensilica Xtensa Architecture");
                break;
        case 95:
                printf("Alphamosaic VideoCore");
                break;
        case 96:
                printf("Thompson Multimedia General Purpose Proc");
                break;
        case 97:
                printf("National Semi. 32000");
                break;
        case 98:
                printf("Tenor Network TPC");
                break;
        case 99:
                printf("Trebia SNP 1000");
                break;
        case 100:
                printf("STMicroelectronics ST200");
                break;
        case 101:
                printf("Ubicom IP2xxx");
                break;
        case 102:
                printf("MAX processor");
                break;
        case 103:
                printf("National Semi. CompactRISC");
                break;
        case 104:
                printf("Fujitsu F2MC16");
                break;
        case 105:
                printf("Texas Instruments msp430");
                break;
        case 106:
                printf("Analog Devices Blackfin DSP");
                break;
        case 107:
                printf("Seiko Epson S1C33 family");
                break;
        case 108:
                printf("Sharp embedded microprocessor");
                break;
        case 109:
                printf("Arca RISC");
                break;
        case 110:
                printf("PKU-Unity & MPRC Peking Uni. mc series");
                break;
        case 111:
                printf("eXcess configurable cpu");
                break;
        case 112:
                printf("Icera Semi. Deep Execution Processor");
                break;
        case 113:
                printf("Altera Nios II");
                break;
        case 114:
                printf("National Semi. CompactRISC CRX");
                break;
        case 115:
                printf("Motorola XGATE");
                break;
        case 116:
                printf("Infineon C16x/XC16x");
                break;
        case 117:
                printf("Renesas M16C");
                break;
        case 118:
                printf("Microchip Technology dsPIC30F");
                break;
        case 119:
                printf("Freescale Communication Engine RISC");
                break;
        case 120:
                printf("Renesas M32C");
                break;
        case 131:
                printf("Altium TSK3000");
                break;
        case 132:
                printf("Freescale RS08");
                break;
        case 133:
                printf("Analog Devices SHARC family");
                break;
        case 134:
                printf("Cyan Technology eCOG2");
                break;
        case 135:
                printf("Sunplus S+core7 RISC");
                break;
        case 136:
                printf("New Japan Radio (NJR) 24-bit DSP");
                break;
        case 137:
                printf("Broadcom VideoCore III");
                break;
        case 138:
                printf("RISC for Lattice FPGA");
                break;
        case 139:
                printf("Seiko Epson C17");
                break;
        case 140:
                printf("Texas Instruments TMS320C6000 DSP");
                break;
        case 141:
                printf("Texas Instruments TMS320C2000 DSP");
                break;
        case 142:
                printf("Texas Instruments TMS320C55x DSP");
                break;
        case 143:
                printf("Texas Instruments App. Specific RISC");
                break;
        case 144:
                printf("Texas Instruments Prog. Realtime Unit");
                break;
        case 160:
                printf("STMicroelectronics 64bit VLIW DSP");
                break;
        case 161:
                printf("Cypress M8C");
                break;
        case 162:
                printf("Renesas R32C");
                break;
        case 163:
                printf("NXP Semi. TriMedia");
                break;
        case 164:
                printf("QUALCOMM DSP6");
                break;
        case 165:
                printf("Intel 8051 and variants");
                break;
        case 166:
                printf("STMicroelectronics STxP7x");
                break;
        case 167:
                printf("Andes Tech. compact code emb. RISC");
                break;
        case 168:
                printf("Cyan Technology eCOG1X");
                break;
        case 169:
                printf("Dallas Semi. MAXQ30 mc");
                break;
        case 170:
                printf("New Japan Radio (NJR) 16-bit DSP");
                break;
        case 171:
                printf("M2000 Reconfigurable RISC");
                break;
        case 172:
                printf("Cray NV2 vector architecture");
                break;
        case 173:
                printf("Renesas RX");
                break;
        case 174:
                printf("Imagination Tech. META");
                break;
        case 175:
                printf("MCST Elbrus");
                break;
        case 176:
                printf("Cyan Technology eCOG16");
                break;
        case 177:
                printf("National Semi. CompactRISC CR16");
                break;
        case 178:
                printf("Freescale Extended Time Processing Unit");
                break;
        case 179:
                printf("Infineon Tech. SLE9X");
                break;
        case 180:
                printf("Intel L10M");
                break;
        case 181:
                printf("Intel K10M");
                break;
        case 183:
                printf("ARM AARCH64");
                break;
        case 185:
                printf("Amtel 32-bit microprocessor");
                break;
        case 186:
                printf("STMicroelectronics STM8");
                break;
        case 187:
                printf("Tilera TILE64");
                break;
        case 188:
                printf("Tilera TILEPro");
                break;
        case 189:
                printf("Xilinx MicroBlaze");
                break;
        case 190:
                printf("NVIDIA CUDA");
                break;
        case 191:
                printf("Tilera TILE-Gx");
                break;
        case 192:
                printf("CloudShield");
                break;
        case 193:
                printf("KIPO-KAIST Core-A 1st gen.");
                break;
        case 194:
                printf("KIPO-KAIST Core-A 2nd gen.");
                break;
        case 195:
                printf("Synopsys ARCv2 ISA.");
                break;
        case 196:
                printf("Open8 RISC");
                break;
        case 197:
                printf("Renesas RL78");
                break;
        case 198:
                printf("Broadcom VideoCore V");
                break;
        case 199:
                printf("Renesas 78KOR");
                break;
        case 200:
                printf("Freescale 56800EX DSC");
                break;
        case 201:
                printf("Beyond BA1");
                break;
        case 202:
                printf("Beyond BA2");
                break;
        case 203:
                printf("XMOS xCORE");
                break;
        case 204:
                printf("Microchip 8-bit PIC(r)");
                break;
        case 205:
                printf("Intel Graphics Technology");
                break;
        case 210:
                printf("KM211 KM32");
                break;
        case 211:
                printf("KM211 KMX32");
                break;
        case 212:
                printf("KM211 KMX16");
                break;
        case 213:
                printf("KM211 KMX8");
                break;
        case 214:
                printf("KM211 KVARC");
                break;
        case 215:
                printf("Paneve CDP");
                break;
        case 216:
                printf("Cognitive Smart Memory Processor");
                break;
        case 217:
                printf("Bluechip CoolEngine");
                break;
        case 218:
                printf("Nanoradio Optimized RISC");
                break;
        case 219:
                printf("CSR Kalimba");
                break;
        case 220:
                printf("Zilog Z80");
                break;
        case 221:
                printf("Controls and Data Services VISIUMcore");
                break;
        case 222:
                printf("FTDI Chip FT32");
                break;
        case 223:
                printf("Moxie processor");
                break;
        case 224:
                printf("AMD GPU");
                break;
        case 243:
                printf("RISC-V");
                break;
        case 247:
                printf("Linux BPF -- in-kernel virtual machine");
                break;
        case 252:
                printf("C-SKY");
                break;
        case 258:
                printf("LoongArch");
                break;
        default:
                printf("Unknown machine");
                break;
        }
}

void __print_elf_version(unsigned int elf_version) {
        switch (elf_version) {
        case EV_NONE:
                printf("ELF Version: EV_NONE (Invalid ELF version)\n");
                break;
        case EV_CURRENT:
                printf("ELF Version: EV_CURRENT (Current version)\n");
                break;
        case EV_NUM:
                // EV_NUM is typically used to indicate the count of valid
                // versions, not a valid version itself.
                printf("ELF Version: EV_NUM (Number of defined versions - not "
                       "a valid version identifier)\n");
                break;
        default:
                printf("ELF Version: Unknown or reserved version (0x%x)\n",
                       elf_version);
                break;
        }
}

__cold static void __print_elf64_hdr(Elf64_Ehdr *ehdr, struct config *config) {
        if (config->show_header_struct == 1) {
                printf("{\n");
                printf("  e_ident = ");
                VT_SIMPLE_HEXDUMP(ehdr->e_ident, 16);
                printf("\n");
                printf("  e_type = %hu,\n", ehdr->e_type);
                printf("  e_machine = %u,\n", ehdr->e_machine);
                printf("  e_version = 0x%x,\n", ehdr->e_version);
                printf("  e_entry = 0x%016" PRIx64 "\n", ehdr->e_entry);
                printf("  e_phoff = 0x%016" PRIx64 "\n", ehdr->e_phoff);
                printf("  e_shoff = 0x%016" PRIx64 "\n", ehdr->e_shoff);
                printf("  e_flags = 0x%x,\n", ehdr->e_flags);
                printf("  e_ehsize = %" PRIu16 ",\n", ehdr->e_ehsize);
                printf("  e_phentsize = %" PRIu16 ",\n", ehdr->e_phentsize);
                printf("  e_phnum = %" PRIu16 ",\n", ehdr->e_phnum);
                printf("  e_shentsize = %" PRIu16 ",\n", ehdr->e_shentsize);
                printf("  e_shnum = %" PRIu16 ",\n", ehdr->e_shnum);
                printf("  e_shstrndx = %" PRIu16 ",\n", ehdr->e_shstrndx);
                printf("}\n");

                return;
        } else if (config->show_header == 1) {
                printf("ELF64 class\n");

                printf("\tType\t\t\t\t");
                __print_process_elf_type(ehdr->e_type);
                printf("\n");

                printf("\tMachine\t\t\t\t");
                __print_machine(ehdr->e_machine);
                printf("\n");

                printf("\tVersion\t\t\t\t");
                __print_elf_version(ehdr->e_version);

                printf("\tEntry point address\t\t0x%016" PRIx64 "\n",
                       ehdr->e_entry);
                printf("\tStart of program headers\t%" PRIu64
                       " (bytes into file)\n",
                       ehdr->e_phoff);
                printf("\tStart of section headers\t%" PRIu64
                       " (bytes into file)\n",
                       ehdr->e_shoff);
                printf("\tFlags\t\t\t\t0x%x\n",
                       ehdr->e_flags);
                printf("\tSize of this header\t\t%" PRIu16
                       " (bytes)\n",
                       ehdr->e_ehsize);
                printf("\tSize of program headers\t\t%" PRIu16
                       " (bytes)\n",
                       ehdr->e_phentsize);
                printf("\tNumber of program headers\t%" PRIu16 "\n",
                       ehdr->e_phnum);
                printf("\tSize of section headers\t\t%" PRIu16
                       " (bytes)\n",
                       ehdr->e_shentsize);
                printf("\tNumber of section headers\t%" PRIu16 "\n",
                       ehdr->e_shnum);
                printf("\tSection header string table idx\t%" PRIu16 "\n",
                       ehdr->e_shstrndx);
        } else {
                // NOP
        }
}

__cold static void __print_elf32_hdr(Elf32_Ehdr *ehdr, struct config *config) {
        if (config->show_header_struct == 1) {
                printf("{\n");
                printf("  e_ident = ");
                VT_SIMPLE_HEXDUMP(ehdr->e_ident, 16);
                printf("\n");
                printf("  e_type = %u,\n", ehdr->e_type);
                printf("  e_machine = %u,\n", ehdr->e_machine);
                printf("  e_version = 0x%x,\n", ehdr->e_version);
                printf("  e_entry = 0x%016" PRIx32 "\n", ehdr->e_entry);
                printf("  e_phoff = 0x%016" PRIx32 "\n", ehdr->e_phoff);
                printf("  e_shoff = 0x%016" PRIx32 "\n", ehdr->e_shoff);
                printf("  e_flags = 0x%x,\n", ehdr->e_flags);
                printf("  e_ehsize = %" PRIu16 ",\n", ehdr->e_ehsize);
                printf("  e_phentsize = %" PRIu16 ",\n", ehdr->e_phentsize);
                printf("  e_phnum = %" PRIu16 ",\n", ehdr->e_phnum);
                printf("  e_shentsize = %" PRIu16 ",\n", ehdr->e_shentsize);
                printf("  e_shnum = %" PRIu16 ",\n", ehdr->e_shnum);
                printf("  e_shstrndx = %" PRIu16 ",\n", ehdr->e_shstrndx);
                printf("}\n");

                return;
        } else if (config->show_header) {
                printf("ELF32 class\n");

                printf("\tType\t\t\t\t");
                __print_process_elf_type(ehdr->e_type);
                printf("\n");

                printf("\tMachine\t\t\t\t");
                __print_machine(ehdr->e_machine);
                printf("\n");

                printf("\tVersion\t\t\t\t");
                __print_elf_version(ehdr->e_version);

                printf("\tEntry point address\t\t0x%016" PRIx32 "\n",
                           ehdr->e_entry);
                printf("\tStart of program headers\t%" PRIu32
                           " (bytes into file)\n",
                           ehdr->e_phoff);
                printf("\tStart of section headers\t%" PRIu32
                           " (bytes into file)\n",
                           ehdr->e_shoff);
                printf("\tFlags\t\t\t\t0x%x\n",
                           ehdr->e_flags);
                printf("\tSize of this header\t\t%" PRIu16
                           " (bytes)\n",
                           ehdr->e_ehsize);
                printf("\tSize of program headers\t\t%" PRIu16
                           " (bytes)\n",
                           ehdr->e_phentsize);
                printf("\tNumber of program headers\t%" PRIu16 "\n",
                           ehdr->e_phnum);
                printf("\tSize of section headers\t\t%" PRIu16
                           " (bytes)\n",
                           ehdr->e_shentsize);
                printf("\tNumber of section headers\t%" PRIu16 "\n",
                           ehdr->e_shnum);
                printf("\tSection header string table idx\t%" PRIu16 "\n",
                           ehdr->e_shstrndx);
        } else {
                // NOP
        }
}

__cold static void __print_table_header() {
        PRINT_PRETTY_PAD_COUNT("type", 4, 16);
        PRINT_PRETTY_PAD_COUNT("flags", 5, 9);
        PRINT_PRETTY_PAD_COUNT("offset", 6, 19);
        PRINT_PRETTY_PAD_COUNT("virtual addr", 12, 19);
        PRINT_PRETTY_PAD_COUNT("physical addr", 13, 19);
        PRINT_PRETTY_PAD_COUNT("file size", 9, 19);
        PRINT_PRETTY_PAD_COUNT("mem size", 8, 19);
        PRINT_PRETTY_PAD_COUNT("align", 5, 19);

        printf("\n");

        for (int i = 0; i < __init_print_pad_count; i++) {
                printf("-");
        }

        PRETTY_PRINT_PAD_COUNT_RESET();
        printf("\n");

        // printf("Type\t\t"
        //        "offset\t\t"
        //        "virtual addr\t\t"
        //        "physical addr\t\t"
        //        "filesz\t\t"
        //        "flags\t\t"
        //        "align\n\n");
}

__cold static void __print_elf64_ph_table(Elf64_Phdr *data,
                                          Elf64_Half e_phnum) {
        __print_table_header();

        for (int i = 0; i < e_phnum; i++) {
                switch (data[i].p_type) {
                case PT_NULL:
                        PRINT_PRETTY("NULL", strlen("NULL"), 16);
                        break;
                case PT_LOAD:
                        PRINT_PRETTY("LOAD", strlen("LOAD"), 16);
                        break;
                case PT_DYNAMIC:
                        PRINT_PRETTY("DYNAMIC", strlen("DYNAMIC"), 16);
                        break;
                case PT_INTERP:
                        PRINT_PRETTY("INTERP", strlen("INTERP"), 16);
                        break;
                case PT_NOTE:
                        PRINT_PRETTY("NOTE", strlen("NOTE"), 16);
                        break;
                case PT_SHLIB:
                        PRINT_PRETTY("SHLIB", strlen("SHLIB"), 16);
                        break;
                case PT_PHDR:
                        PRINT_PRETTY("PHDR", strlen("PHDR"), 16);
                        break;
                case PT_TLS:
                        PRINT_PRETTY("TLS", strlen("TLS"), 16);
                        break;
                case PT_NUM:
                        PRINT_PRETTY("NUM", strlen("NUM"), 16);
                        break;
                case PT_GNU_EH_FRAME:
                        PRINT_PRETTY("GNU_EH_FRAME", strlen("GNU_EH_FRAME"),
                                     16);
                        break;
                case PT_GNU_STACK:
                        PRINT_PRETTY("GNU_STACK", strlen("GNU_STACK"), 16);
                        break;
                case PT_GNU_RELRO:
                        PRINT_PRETTY("GNU_RELRO", strlen("GNU_RELRO"), 16);
                        break;
                case PT_GNU_PROPERTY:
                        PRINT_PRETTY("GNU_PROPERTY", strlen("GNU_PROPERTY"),
                                     16);
                        break;
                case PT_GNU_SFRAME:
                        PRINT_PRETTY("GNU_SFRAME", strlen("GNU_SFRAME"), 16);
                        break;
                case PT_SUNWBSS:
                        PRINT_PRETTY("LOSUNW/SUNWBSS", strlen("LOSUNW/SUNWBSS"),
                                     16);
                        break;
                case PT_SUNWSTACK:
                        PRINT_PRETTY("SUNWSTACK", strlen("SUNWSTACK"), 16);
                        break;
                default:
                        if (data->p_type >= PT_LOOS &&
                            data->p_type <= PT_HIOS) {
                                PRINT_PRETTY("OS_SPESIFIC",
                                             strlen("OS_SPESIFIC"), 16);
                        } else if (data->p_type >= PT_LOPROC &&
                                   data->p_type <= PT_HIPROC) {
                                PRINT_PRETTY("PROCESSOR_SPESIFIC",
                                             strlen("PROCESSOR_SPESIFIC"), 16);
                        } else {
                                PRINT_PRETTY("UNKNOWN", strlen("UNKNOWN"), 16);
                        }
                        break;

                        /* pad */
                        // printf("\t\t");
                }

                /* print flags */
                if (data[i].p_flags == PF_X) {
                        PRINT_PRETTY("X", 1, 9);
                } else if (data[i].p_flags == PF_W) {
                        PRINT_PRETTY("W", 1, 9);
                } else if (data[i].p_flags == PF_R) {
                        PRINT_PRETTY("R", 1, 9);
                } else if (data[i].p_flags == PF_MASKOS) {
                        PRINT_PRETTY("MASKOS", 6, 9);
                } else if (data[i].p_flags == PF_MASKPROC) {
                        PRINT_PRETTY("MASKPROC", 7, 9);
                } else if (data[i].p_flags == (PF_R | PF_W)) {
                        PRINT_PRETTY("RW", 2, 9);
                } else if (data[i].p_flags == (PF_R | PF_X)) {
                        PRINT_PRETTY("RX", 2, 9);
                } else if (data[i].p_flags == (PF_R | PF_X | PF_W)) {
                        PRINT_PRETTY("RXW", 3, 9);
                } else {
                        PRINT_PRETTY("UNDEF", 5, 9);
                }

                // /* print offset */
                PRINT_PRETTYF("0x%016lx", data[i].p_offset, 18, 19);

                /* print vaddr */
                PRINT_PRETTYF("0x%016lx", data[i].p_vaddr, 18, 19);

                /* print paddr */
                PRINT_PRETTYF("0x%016lx", data[i].p_paddr, 18, 19);

                /* print file size */
                PRINT_PRETTYF("0x%016lx", data[i].p_filesz, 18, 19);

                /* print file size */
                PRINT_PRETTYF("0x%016lx", data[i].p_memsz, 18, 19);

                /* print file size */
                PRINT_PRETTYF("0x%0lx", data[i].p_align, 18, 19);

                // char predicted_max_int[20];
                // sprintf(predicted_max_int, "%lu", data[i].p_filesz);
                // PRINT_PRETTYF("%s", predicted_max_int,
                //               strlen(predicted_max_int), 18);

                /* end */
                printf("\n");
        }
}

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

__cold static Elf64_Phdr *interpret_elf64_program_header(int fd,
                                                         Elf64_Off elf_start,
                                                         Elf64_Half e_phnum) {
        /* alloc once, free at function exits */
        u_int8_t *buf_each = (u_int8_t *)malloc(sizeof(Elf64_Phdr));

        Elf64_Phdr *program_header_section =
            (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr) * e_phnum);

        for (int i = 0; i < e_phnum; i++) {
                lseek(fd, elf_start + (sizeof(Elf64_Phdr) * i), SEEK_SET);

                int ret = read(fd, buf_each, sizeof(Elf64_Phdr));
                if (ret < 0) {
                        perror("read() on interpret_elf64_program_header");
                } else {
                        memcpy(&program_header_section[i], buf_each,
                               sizeof(Elf64_Phdr));
                        memset(buf_each, 0, sizeof(Elf64_Phdr));
                }
        }
        free(buf_each);

        return program_header_section;
}

static int parse_opt(int argc, char *argv[], struct config *config) {
        int retval = 0;
        char opt = 0;
        int index = 0;

        u_int64_t conv_optarg = 0;

        while (1) {
                opt = getopt_long(argc, argv, "", long_options, &index);

                if (opt == -1) {
                        break;
                }

                switch (opt) {
                case '?':
                        break;

                case GETOPT_CUSTOM_FILE:
                        config->filename = optarg;
                        break;

                case GETOPT_CUSTOM_HEADER:
                        config->show_header = 1;
                        break;

                case GETOPT_CUSTOM_HEADER_STRUCT:
                        config->show_header_struct = 1;
                        break;

                case GETOPT_CUSTOM_PROGRAM_HEADER:
                        config->show_program_header = 1;
                        break;

                case GETOPT_CUSTOM_PROGRAM_HEADER_STRUCT:
                        config->show_program_header_struct = 1;
                        break;

                case GETOPT_CUSTOM_HEXDUMP:
                        config->hexdump = 1;
                        break;
                }
        }

        return retval;
}

__hot static int __get_file_n(int fd) {
        struct stat statbuf;
        memset(&statbuf, 0, sizeof(struct stat));

        int ret = fstat(fd, &statbuf);
        if (ret == 0) {
                // asm volatile("nop");

                return statbuf.st_size;
        } else {
                perror("fstat()");
                return -1;
        }
}

__hot static int _start_hexdump(int fd) {
        struct file_off_control file_off_control = { .offset = FILE_BUFSIZE,
                                                     .n = 0 };
        // memset(&file_off_control, 0, sizeof(struct file_off_control));

        int filesize = __get_file_n(fd);

        char *buf = (char *)malloc(sizeof(char) * FILE_BUFSIZE);

        VT_TITLE(buf, filesize);
        for (int i = 0; i < (filesize / FILE_BUFSIZE); i++) {
                lseek(fd, SEEK_SET,
                      (file_off_control.offset * file_off_control.n));
                read(fd, buf, FILE_BUFSIZE);
                HEXDUMP(buf, FILE_BUFSIZE);

                file_off_control.n = file_off_control.n + 1;
        }

        int last_off = file_off_control.offset * file_off_control.n;
        int n = filesize - last_off;

        lseek(fd, SEEK_SET, last_off);
        memset(buf, 0, n);
        read(fd, buf, n);
        HEXDUMP(buf, n);

        free(buf);
}

int main(int argc, char **argv) {
        struct config config;
        memset(&config, 0, sizeof(config));

        int ret = parse_opt(argc, argv, &config);
        // __debug_config(&config);

        ret = __open_file(config.filename);
        if (ret < 0) {
                return -1;
                // VT_HEXDUMP(ehdr, sizeof(Elf64_Ehdr));
                // asm volatile("nop");
                // printf(const char *restrict format, ...)
        }

        int elf_arch_type = read_elf_magic(ret);

        if (elf_arch_type == ELF64) {
                Elf64_Ehdr *ehdr = (Elf64_Ehdr *)malloc(sizeof(Elf64_Ehdr));
                interpret_elf64_hdr(ret, ehdr);
                __print_elf64_hdr(ehdr, &config);
                free(ehdr);

                if (config.show_program_header) {
                        Elf64_Phdr *phdr_table = interpret_elf64_program_header(
                            ret, ehdr->e_phoff, ehdr->e_phnum);

                        // for (int i = 0; i < )
                        __print_elf64_ph_table(phdr_table, ehdr->e_phnum);
                        free(phdr_table);
                        // VT_HEXDUMP(phdr_table, sizeof(Elf64_Phdr));
                }
        }

        if (elf_arch_type == ELF32) {
                Elf32_Ehdr *ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
                interpret_elf32_hdr(ret, ehdr);
                __print_elf32_hdr(ehdr, &config);
                free(ehdr);
        }

        if (elf_arch_type == NOT_ELF) {
                fprintf(stderr, "NOT A ELF FILE!\n");
        }

        if (elf_arch_type == UNDEFINED_ARCH) {
                fprintf(stderr, "its confirmed as ELF, but arch is not "
                                "x86 (legacy) or x86-64\n");
        }

        if (config.hexdump) {
                _start_hexdump(ret);
        }
        // __debug_config(&config);
}
