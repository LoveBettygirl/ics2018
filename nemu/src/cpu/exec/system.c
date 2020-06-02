#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  //TODO();
  rtl_lm((rtlreg_t*)&cpu.idtr.limit, &id_dest->addr, 2);
  if (decoding.is_operand_size_16) {
    rtl_addi(&t0, &id_dest->addr, 2);
    rtl_lm(&cpu.idtr.base, &t0, 3);
  }
  else {
    rtl_addi(&t0, &id_dest->addr, 2);
    rtl_lm(&cpu.idtr.base, &t0, 4);
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  //TODO();
  printf("mov_r2cr: %d\n", id_dest->reg);
  switch (id_dest->reg) {
    case 0: cpu.cr0.val = id_src->val; break;
    case 3: cpu.cr3.val = id_src->val; break;
    default: panic("n86 does not have cr%d", id_dest->reg);
  }

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  //TODO();
  printf("mov_cr2r: %d\n", id_src->reg);
  switch (id_src->reg) {
    case 0: id_dest->val = cpu.cr0.val; break;
    case 3: id_dest->val = cpu.cr3.val; break;
    default: panic("n86 does not have cr%d", id_src->reg);
  }

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

extern void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(int) {
  //TODO();
  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  //TODO();
  rtl_pop(&decoding.jmp_eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&cpu.eflags.val);
  decoding.is_jmp = 1;

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  //TODO();
  t0 = pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //TODO();
  pio_write(id_dest->val, id_dest->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
