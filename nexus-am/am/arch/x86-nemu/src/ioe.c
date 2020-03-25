#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int width = x + w <= _screen.width ? w : _screen.width - x;
  int i;
  for (i = y; i < _screen.height; i++) {
  	if(i > y + h) break;
  	memcpy(&fb[i * _screen.width + x], pixels + (i - y)*width, width * sizeof(uint32_t));
  }
}

void _draw_sync() {
}

int _read_key() {
  if(inb(0x64) == 1) { // 0x64: status port (uint32_t)
  	return inl(0x60); // 0x60: data port (uint8_t)
  }
  return _KEY_NONE;
}
