#include "common.h"

//#define DEFAULT_ENTRY ((void *)0x4000000)
#define DEFAULT_ENTRY ((void *)0x8048000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);
extern void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //ramdisk_read((void*)DEFAULT_ENTRY, 0, get_ramdisk_size());
  /*assert(filename);
  Log("The image is %s", filename);
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, (void*)DEFAULT_ENTRY, fs_filesz(fd));
  fs_close(fd);*/
  assert(filename);
  Log("The image is %s", filename);
  void *pa, *va = DEFAULT_ENTRY;
  int fd = fs_open(filename, 0, 0);
  size_t filesz = fs_filesz(fd);
  for (size_t i = 0; i < filesz; i += PGSIZE) {
  	pa = new_page();
  	_map(as, va, pa);
  	fs_read(fd, pa, PGSIZE);
  	va += PGSIZE;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
