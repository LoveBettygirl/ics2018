#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  assert(NO <= cpu.idtr.limit);
  rtl_push(&cpu.eflags.val);
  cpu.eflags.IF = 0;
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr);
  rtl_li(&t0, vaddr_read(cpu.idtr.base + NO * 8, 2));
  rtl_li(&t1, vaddr_read(cpu.idtr.base + NO * 8 + 6, 2));
  memcpy((void*)&decoding.jmp_eip, (void*)&t0, 2);
  memcpy((void*)((char*)&decoding.jmp_eip + 2), (void*)&t1, 2);
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
  cpu.INTR = true;
}
