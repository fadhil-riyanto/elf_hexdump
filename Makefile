CC = clang

elf64: elf64_hexdump.c hexdump.h
	${CC} elf64_hexdump.c -o elf64 -g