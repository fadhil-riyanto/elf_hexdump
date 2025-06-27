#ifndef GETOPT_CUSTOM_H
#define GETOPT_CUSTOM_H                         1

#define GETOPT_CUSTOM_FILE                      0x01 /* --file x */
#define GETOPT_CUSTOM_HEADER                    0x02 /* --header x, show user friendly ELF header */
#define GETOPT_CUSTOM_HEADER_STRUCT             0x03 /* --struct-header x */
                                                /* show ELF struct header that sync with linux kernel
                                                struct flavor
                                                */
#define GETOPT_CUSTOM_HEXDUMP                   0x04 /* hexdump all */
#define GETOPT_CUSTOM_PROGRAM_HEADER            0x05 /* dump program header (pretty table) */
#define GETOPT_CUSTOM_PROGRAM_HEADER_STRUCT     0x06 /* dump program header (kernel struct)*/

#endif /* GETOPT_CUSTOM_H */