/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "common.h"
#include "debug.h"
#include "macro.h"
#include "sdb.h"
#include <stdint.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[32];
  uint32_t oldval;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
// Alloc new watchpoint
// Return watchpoint number, -1 for ERROR
int
new_wp(char *e)
{
  WP *wp;
  bool success;

  if (!(wp = free_)) {
    panic("Too much watchpoints\n");
    return -1;
  }
  free_ = free_->next;
  wp->next = head;
  head = wp;
  
  if (strlen(e) > 32-1) {
    panic("expr is too long\n");
    return -1;
  }
  memcpy(wp->expr, e, strlen(e));
  wp->expr[strlen(e)] = 0;
  wp->oldval = expr(e, &success);
  if (success != true) {
    panic("Caculate old value for watch failed\n");
    return -1;
  }
  return wp->NO;
}

void
free_wp(int NO)
{
  WP *wp=head, *pre=NULL;

  while (wp) {
    if (wp->NO == NO)
      break;
    pre = wp;
    wp = wp->next;
  }
  assert(wp);
  if (!pre) {
    head = wp->next;
  } else {
    pre->next = wp->next;
  }
  wp->next = free_;
  free_ = wp;
}

bool
check_wp(void)
{
  WP *wp = head;
  bool success;
  word_t newval;
  
  while (wp) {
    newval = expr(wp->expr, &success);
    assert(success == true);
    if (newval != wp->oldval) 
      return false;
    wp = wp->next;
  }
  return true;
}

void
display_wp(void)
{
  WP *wp = head;

  while (wp) {
    printf("[%d]: %s, old=%d\n", 
        wp->NO, wp->expr, wp->oldval);
    wp = wp->next;
  }
}

