#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //ramdisk_read((void*)DEFAULT_ENTRY, 0, get_ramdisk_size());
  fs_read(fs_open(filename, 0, 0), (void*)DEFAULT_ENTRY, get_ramdisk_size());
  return (uintptr_t)DEFAULT_ENTRY;
}
