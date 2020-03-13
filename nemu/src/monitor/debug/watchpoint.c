#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#include <assert.h>

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static int using = 1;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    memset(wp_pool[i].expr, 0, sizeof(wp_pool[i].expr));
    wp_pool[i].val = 0;
    wp_pool[i].hit_count = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char *e, uint32_t val) {
	if (free_ == NULL) {
		printf("Error: No free watchpoints.\n");
		return NULL;
	}
	WP *temp = free_;
	free_ = free_->next;
	strcpy(temp->expr, e);
	temp->val = val;
	temp->hit_count = 0;
	temp->NO = using++;
	temp->next = NULL;
	WP *temp2 = head;
	if (temp2 == NULL) {
		head = temp;
	}
	else {
		while (temp2->next != NULL) {
			temp2 = temp2->next;
		}
		temp2->next = temp;
	}
	return temp;
}

bool free_wp(int NO) {
	if (head == NULL) {
		return false;
	}
	WP *find = NULL;
	if (head->NO == NO) {
		find = head;
		head = head->next;
	}
	else {
		WP *temp = head;
		while (temp->next != NULL) {
			if (temp->next->NO == NO) {
				find = temp->next;
				temp->next = find->next;
				break;
			}
			temp = temp->next;
		}
	}
	if (find == NULL) {
		return false;
	}
	find->next = free_;
	free_ = find;
	return true;
}

void print_wp() {
	if (head == NULL) {
		printf("No watchpoints.\n");
		return;
	}
	printf("%-5s%-20s%-15s%s\n", "Num", "What", "Value", "Hit count");
	WP *temp = head;
	while (temp != NULL) {
		printf("%-5d%-20s%-15d%d\n", temp->NO, temp->expr, temp->val, temp->hit_count);
		temp = temp->next;
	}
}

WP* check_wp() {
	WP *temp = head;
	uint32_t val;
	bool success = false;
	while (temp != NULL) {
		val = expr(temp->expr, &success);
		if (!success) {
			printf("Evaluate failed in watchpoint number %d.\n", temp->NO);
			return NULL;
		}
		if (val != temp->val) {
			temp->hit_count++;
			printf("Watchpoint %d: %s\n\n", temp->NO, temp->expr);
			printf("Old value = %d\n", temp->val);
			printf("New value = %d\n", val);
			printf("Hit count: %d\n", temp->hit_count);
			printf("Hit at address: 0x%x\n", cpu.eip);
			temp->val = val;
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}
