#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void ramdisk_read(void *buf, off_t offset, size_t len);

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 3; i < NR_FILES; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  assert(fd > 2);
  int filesz = fs_filesz(fd);
  off_t addr = file_table[fd].disk_offset + file_table[fd].open_offset;
  if (addr < file_table[fd].disk_offset || addr >= file_table[fd].disk_offset + filesz)
    return -1;
  int ret = file_table[fd].open_offset + len >= filesz? 
    (filesz - 1 - file_table[fd].open_offset): len;
  ramdisk_read(buf, addr, ret);
  file_table[fd].open_offset += ret;
  return ret;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  if (fd == 1 || fd == 2) {
    size_t i;
    for (i = 0; i < len; i++) {
      _putc(*((char*)buf + i));
    }
    return i;
  }
  assert(fd != 0);
  int filesz = fs_filesz(fd);
  off_t addr = file_table[fd].disk_offset + file_table[fd].open_offset;
  if (addr < file_table[fd].disk_offset || addr >= file_table[fd].disk_offset + filesz)
    return -1;
  int ret = file_table[fd].open_offset + len >= filesz? 
    (filesz - 1 - file_table[fd].open_offset): len;
  ramdisk_write(buf, addr, ret);
  file_table[fd].open_offset += ret;
  return ret;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  off_t start = 0;
  switch (whence) {
    case SEEK_SET: start = 0; break;
    case SEEK_CUR: start = file_table[fd].open_offset; break;
    case SEEK_END: start = fs_filesz(fd); break;
    default: return -1;
  }
  if (start + offset < 0) return -1;
  file_table[fd].open_offset = start + offset;
  return start + offset;
}

int fs_close(int fd) {
  return 0;
}
