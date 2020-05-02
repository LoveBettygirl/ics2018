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
  Log("fs_open()");
  for (int i = 3; i < NR_FILES; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = file_table[i].disk_offset;
      Log("%s", pathname);
      return i;
    }
  }
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  Log("fs_read()");
  assert(fd > 2);
  int ret = file_table[fd].open_offset + len >= fs_filesz(fd)? 
    fs_filesz(fd) - 1 - file_table[fd].open_offset: len;
  Log("ret: %d", ret);
  ramdisk_read(buf, file_table[fd].open_offset, ret);
  file_table[fd].open_offset += ret;
  return ret;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  Log("fs_write()");
  if (fd == 1 || fd == 2) {
    size_t i;
    for (i = 0; i < len; i++) {
      _putc(*((char*)buf + i));
    }
    return i;
  }
  assert(fd != 0);
  int ret = file_table[fd].open_offset + len >= fs_filesz(fd)? 
    fs_filesz(fd) - 1 - file_table[fd].open_offset: len;
  ramdisk_write(buf, file_table[fd].open_offset, ret);
  file_table[fd].open_offset += ret;
  return ret;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  Log("fs_lseek()");
  assert(whence >= file_table[fd].disk_offset && whence + offset < file_table[fd].disk_offset + fs_filesz(fd));
  //int ret = whence + offset >= fs_filesz(fd)? file_table[fd].disk_offset + fs_filesz(fd) - 1: whence + offset;
  file_table[fd].open_offset = whence + offset;
  return whence + offset;
}

int fs_close(int fd) {
  Log("fs_close()");
  return 0;
}
