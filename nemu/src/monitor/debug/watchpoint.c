#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"
#include <assert.h>

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static int using = 0;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
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
	if (!using) {
		init_wp_pool();
	}
	WP *temp = free_;
	free_ = free_->next;
	temp->next = head;
	head = temp;
	strcpy(temp->expr, e);
	temp->val = val;
	using++;
	return temp;
}

void free_wp(int NO) {
	Assert(NO >= 0 && NO < NR_WP, "Illegal watchpoint number %d", NO);
	if (head == NULL) {
		printf("No watchpoint number %d\n", NO);
		return;
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
		printf("No watchpoint number %d\n", NO);
		return;
	}
	find->next = free_;
	free_ = find;
	using--;
	printf("Watchpoint number %d is deleted successfully.\n", NO);
}

void print_wp() {
	if (head == NULL) {
		printf("No watchpoints.\n");
		return;
	}
	printf("%s%-20s%-20s\n", "Num", "What", "Value");
	WP *temp = head;
	while (temp != NULL) {
		printf("%d%-20s\t%-20u\n", temp->NO, temp->expr, temp->val);
		temp = temp->next;
	}
}

WP* check_wp() {
	assert(head != NULL);
	WP *temp = head;
	uint32_t val;
	bool success = false;
	while (temp != NULL) {
		val = expr(temp->expr, &success);
		if (!success) {
			printf("Evaluate failed in watchpoint number %d\n", temp->NO);
			return NULL;
		}
		if (val != temp->val) {
			printf("Watchpoint %d: %s\n\n", temp->NO, temp->expr);
			printf("Old value = %u\n", temp->val);
			printf("New value = %u\n", val);
			printf("At address: 0x%x\n", cpu.eip);
			temp->val = val;
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}
