#include "common.h"
#include "syscall.h"

static uintptr_t sys_write(_RegSet *r, int fd, void *buf, size_t count) {
	if (fd == 1 || fd == 2) {
		size_t i;
		for (i = 0; i < count; i++) {
			_putc(*((char*)buf + i));
		}
		SYSCALL_ARG1(r) = i;
		return i;
	}
	SYSCALL_ARG1(r) = -1;
	return -1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
  	case SYS_none: SYSCALL_ARG1(r) = 1; break;
  	case SYS_exit: _halt(a[1]); break;
  	case SYS_write: sys_write(r, a[1], (void*)a[2], a[3]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
