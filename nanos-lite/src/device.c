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

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  off_t x = offset / sizeof(uint32_t) % _screen.width;
  off_t y = offset / sizeof(uint32_t) / _screen.width;
  off_t w = len / sizeof(uint32_t);
  if (w >= _screen.width - x) {
  	_draw_rect(buf, x, y, _screen.width - 1 - x, 0);
  	off_t temp;
  	int i;
  	uint32_t *buf1 = (uint32_t*)buf + (_screen.width - 1 - x);
  	temp = w = w - (_screen.width - 1 - x);
  	for (i = 1; i <= temp / (_screen.width - 1); i++) {
  	  _draw_rect(buf1, 0, y + i, _screen.width - 1, 0);
  	  w -= (_screen.width - 1);
  	  buf1 += (_screen.width - 1);
  	}
  	if (w != 0)
  	  _draw_rect(buf1, 0, y + i, w, 0);
  }
  else {
	_draw_rect(buf, x, y, w, 0);
  }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH: %d\nHEIGHT: %d", _screen.width, _screen.height);
  Log("WIDTH: %d HEIGHT: %d", _screen.width, _screen.height);
}
