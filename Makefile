CC = clang

elf64: elf64_hexdump.c hexdump.h
	${CC} elf64_hexdump.c -o elf64 -g

elf64_dev: elf64_hexdump.c hexdump.h
	${CC} elf64_hexdump.c -o elf64 -g -O0 -fsanitize=address

m32: ./repro/m32.c
	${CC} ./repro/m32.c -o m32 -g

test_avr_gcc: ./repro/avr.c
	avr-gcc ./repro/avr.c -o avr

test_x86: ./repro/avr.c
	${CC} ./repro/m32.c -o m32 -m32

clean:
	rm -f avr 
	rm -f m32
	rm -f elf64