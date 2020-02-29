#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[32];
  uint32_t val;
  /* TODO: Add more members if necessary */


} WP;

void init_wp_pool();
WP* new_wp(char *e, uint32_t val);
void free_wp(int NO);
void print_wp();
WP* check_wp();

#endif
