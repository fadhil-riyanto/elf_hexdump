# ELF (Executable and Linkable Format)

<img src="https://raw.githubusercontent.com/corkami/pics/28cb0226093ed57b348723bc473cea0162dad366/binary/elf101/elf101-64.svg">

this is dumb tool used to inspect Linux ELF binary structure and also act as hexdump.

# compiling & use
the compile process is very easy, just type `make`. by default its compile debug version (with symbol inside).

## example usage
### dump ELF header struct
`./elf64 --file elf64 --header 1 --header-struct`

### hexdump only
`./elf64 --file elf64 --hexdump`

### show user friendly header
`./elf64 --file elf64 --header 1`

# useful resources
- [https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/elf.h#n234](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/elf.h#n234)
- [https://0xax.gitbooks.io/linux-insides/content/Theory/linux-theory-2.html](https://0xax.gitbooks.io/linux-insides/content/Theory/linux-theory-2.html)


# todo
- add option to print as elf64/32_ehdr struct
- add option to jump into spesific index of shdr
- add option to show all symbol inside of shdr and .