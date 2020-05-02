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
  if (x + w >= _screen.width) {
  	_draw_rect(buf, x, y, _screen.width - x, 1);
  	w -= (_screen.width - x);
  	uint32_t *buf1 = (uint32_t*)buf + (_screen.width - x);
  	if (w / _screen.width > 0) {
  	  _draw_rect(buf1, 0, y + 1, _screen.width, w / _screen.width);
  	  buf1 += (w - w % _screen.width);
  	  w %= _screen.width;
  	  y += (w / _screen.width);
  	}
  	_draw_rect(buf1, 0, y + 1, w, 1);
  }
  else {
	_draw_rect(buf, x, y, w, 1);
  }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH: %d\nHEIGHT: %d", _screen.width, _screen.height);
  Log("WIDTH: %d HEIGHT: %d", _screen.width, _screen.height);
}
