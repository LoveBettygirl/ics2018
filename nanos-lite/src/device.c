#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

extern _Screen _screen;
extern void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h);
extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
	int x = offset % _screen.width;
	int y = offset / _screen.width;
	int w = len % (_screen.width - x);
	int h = len / (_screen.width - x);
	_draw_rect(buf, x, y, w, h);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int fd = fs_open("/proc/dispinfo", 0, 0);
  fs_read(fd, dispinfo, fs_filesz(fd));
  Log("%s", dispinfo);
  fs_close(fd);
}
