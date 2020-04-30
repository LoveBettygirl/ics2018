#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  //TODO();
  rtl_li(&t0, cpu.esp); // save esp
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t0); // push esp
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  //TODO();
  rtl_mv(&cpu.esp, &cpu.ebp);  // movl %ebp, %esp
  rtl_pop(&cpu.ebp);  // popl %ebp

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    //TODO();
    rtl_lr(&t0, R_AX, 2);
    if((int32_t)(int16_t)t0 < 0) {
      rtl_li(&t0, 0x0000ffff);
    }
    else {
      rtl_li(&t0, 0);
    }
    rtl_sr(R_DX, 2, &t0);
  }
  else {
    //TODO();
    rtl_lr(&t0, R_EAX, 4);
    if((int32_t)t0 < 0) {
      rtl_li(&t0, 0xffffffff);
    }
    else {
      rtl_li(&t0, 0);
    }
    rtl_sr(R_EDX, 4, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    //TODO();
    rtl_lr(&t0, R_AL, 1);
    rtl_sext(&t0, &t0, 1);
    rtl_sr(R_AX, 2, &t0);
  }
  else {
    //TODO();
    rtl_lr(&t0, R_AX, 2);
    rtl_sext(&t0, &t0, 2);
    rtl_sr(R_EAX, 4, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
