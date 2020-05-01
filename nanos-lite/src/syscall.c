#include "common.h"
#include "syscall.h"

static uintptr_t sys_none() {
	return 1;
}

static uintptr_t sys_exit(int status) {
	_halt(status);
	return status;
}

static uintptr_t sys_write(int fd, void *buf, size_t count) {
	Log("");
	if (fd == 1 || fd == 2) {
		size_t i;
		for (i = 0; i < count; i++) {
			_putc(*((char*)buf + i));
		}
		return i;
	}
	return -1;
}

static uintptr_t sys_brk(void *addr) {
	return 0;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
  	case SYS_none: a[0] = sys_none(); break;
  	case SYS_exit: a[0] = sys_exit(a[1]); break;
  	case SYS_write: a[0] = sys_write(a[1], (void*)a[2], a[3]); break;
  	case SYS_brk: a[0] = sys_brk((void*)a[1]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  SYSCALL_ARG1(r) = a[0];

  return NULL;
}
