#include "common.h"
#include "syscall.h"

static uintptr_t sys_none(_RegSet *r) {
  SYSCALL_ARG1(r) = 1;
  return 1;
}

static uintptr_t sys_exit(_RegSet *r) {
  int ret = SYSCALL_ARG1(r);
  _halt(ret);
  return ret;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
  	case SYS_none: sys_none(r); break;
  	case SYS_exit: sys_exit(r); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
