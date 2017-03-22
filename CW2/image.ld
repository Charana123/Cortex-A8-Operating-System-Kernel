SECTIONS {
  /* assign load address (per  QEMU) */
  .       =     0x70010000;
  /* place text segment(s)           */
  .text : { kernel/lolevel.o(.text) *(.text .rodata) }
  /* place data segment(s)           */
  .data : {                         *(.data        ) }
  /* place bss  segment(s)           */
  .bss  : {                         *(.bss         ) }
  .heap : {
    end = .;
    _heap_start = .;
    . = . + 0x00010000;
    _heap_end =   .;
  }
  /* align       address (per AAPCS) */
  .       = ALIGN( 8 );
  /* allocate stack for irq mode     */
  .       = . + 0x00001000;
  tos_irq = .;
  /* allocate stack for svc mode     */
  .       = . + 0x00001000;
  tos_svc = .;
  /* allocate stack for Console           */
  .       = . + 0x00001000;
  tos_console  = .;
  /* allocate stack for P4           */
  .       = . + 0x00001000;
  tos_P4  = .;
  /* allocate stack for P5           */
  .       = . + 0x00001000;
  tos_P5  = .;


}
