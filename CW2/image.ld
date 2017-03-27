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
  .       =     0x70700000 - 8;
  tos_irq = .;
  /* allocate stack for svc mode     */
  .       =     0x70800000 - 8;
  tos_svc = .;
  /* allocate stack for abt mode     */
  .       =     0x70900000 - 8;
  tos_abt = .;
}
