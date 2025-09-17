ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
  . = 0x0100000;

  .text :
  {
    *(.multiboot)
    *(.text*)
    *(.rodata)
  } : text

  .data  :
  {
    start_ctors = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    end_ctors = .;

    *(.data)
  } : data

  .bss  :
  {
    *(.bss)
  } : bss

  /DISCARD/ : { *(.fini_array*) *(.comment) }
}

PHDRS {
  text PT_LOAD FLAGS(0x5);  /* R+X */
  data PT_LOAD FLAGS(0x6);  /* R+W */
  bss  PT_LOAD FLAGS(0x6);  /* R+W */
}