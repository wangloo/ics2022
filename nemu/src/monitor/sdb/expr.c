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
#include "memory/paddr.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdint.h>
#include <string.h>

enum {
  TK_NOTYPE = 256,

  /* TODO: Add more token types */
  TK_OP_BEGIN,
  // Sorted by priority
  TK_ADD, TK_SUB, TK_MUL, TK_DIV, TK_EQ,
  TK_OP_END,
  TK_NUM, TK_HEX_NUM, TK_LEFT_PARENTS, TK_RIGHT_PARENTS,
  TK_REG, TK_DEREF,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},         // plus
  {"==", TK_EQ},        // equal
  {"\\-", TK_SUB},
  {"\\*", TK_MUL},   // mul or derefence 
  {"\\/", TK_DIV},
  {"\\(", TK_LEFT_PARENTS},
  {"\\)", TK_RIGHT_PARENTS},
  {"0[xX][0-9]+", TK_HEX_NUM},
  {"[0-9]+", TK_NUM},
  {"\\$[A-Za-z0-9]+", TK_REG},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_REG:
          if (substr_len > 32-1) 
            TODO();
          // Remove leading '$'
          memcpy(tokens[nr_token].str, substr_start+1, substr_len-1);
          tokens[nr_token].str[substr_len] = '\0'; // Add terminated char
          tokens[nr_token++].type = rules[i].token_type;
          break;
        case TK_NUM:
        case TK_HEX_NUM:
          if (substr_len > 32-1) 
            TODO();
          if (nr_token > 0 && tokens[nr_token-1].type == TK_DEREF) {
            // Dereference with 十进制数字并不常见吧？
            // 暂时认为都是十进制，以后再优化
            assert(rules[i].token_type == TK_HEX_NUM);
            memcpy(tokens[nr_token-1].str, substr_start, substr_len);
            tokens[nr_token-1].str[substr_len] = '\0'; // Add terminated char
          }
          else {
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0'; // Add terminated char
            tokens[nr_token++].type = rules[i].token_type;
          }
          break;
        case TK_MUL:
          // 特殊处理 dereference
          if (!nr_token || tokens[nr_token-1].type == TK_LEFT_PARENTS 
                        || tokens[nr_token-1].type == TK_RIGHT_PARENTS
                        || (tokens[nr_token-1].type > TK_OP_BEGIN && tokens[i-1].type <TK_OP_END)) {
            Log("Dereference detected!!\n");
            rules[i].token_type = TK_DEREF;
          }
        case TK_ADD:
        case TK_SUB:
        case TK_DIV:
        case TK_EQ:
        case TK_LEFT_PARENTS:
        case TK_RIGHT_PARENTS:
          tokens[nr_token++].type = rules[i].token_type;
        case TK_NOTYPE:
          break;
        default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}


bool 
check_parentheses(Token *p, Token *q)
{
  int diff = 0;

  if (p->type != TK_LEFT_PARENTS || q->type != TK_RIGHT_PARENTS)
    return false;

  p += 1, q -= 1;
  while (p <= q) {
    if (p->type == TK_LEFT_PARENTS) diff += 1;
    else if (p->type == TK_RIGHT_PARENTS) diff -= 1;
    if (diff < 0) 
      return false;
    p += 1;
  }
  return (diff == 0)? true : false;
}

word_t 
eval(Token *p, Token *q, bool *success)
{
  if (p > q) {
    *success = false;
    assert(0);
  }
  else if (p == q) {
    word_t result;
    assert(p->type == TK_NUM 
            || p->type == TK_HEX_NUM
            || p->type == TK_DEREF
            || p->type == TK_REG);
    // Log("p->type: %d, val: %s\n", p->type, p->str);
    if (p->type == TK_NUM) 
      result = strtoul(p->str, NULL, 10);
    else if (p->type == TK_HEX_NUM)
      result =  strtoul(p->str, NULL, 16);
    else if (p->type == TK_DEREF) {
      uint32_t deref_addr = strtoul(p->str, NULL, 16);
      result = *(uint32_t *)guest_to_host(deref_addr);
    }
    else if (p->type == TK_REG) {
      bool reg_success = false;
      result = isa_reg_str2val(p->str, &reg_success);
      assert(reg_success == true);
    }
    else {
      assert(0);
    }
    *success = true;
    return result;
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p+1, q-1, success);
  }
  else {
    Token *t, *main_op;
    int in_parents=0;
    word_t val1, val2;

    // Init main_op to TK_NUM which priv > TK_OP_END,
    // must be refilled at follow
    main_op = p;
    for (t=p; t<=q; t++) {
      if (t->type == TK_LEFT_PARENTS) {
        in_parents = 1;
      } else if (t->type == TK_RIGHT_PARENTS) {
        in_parents = 0;
      }       
      if (!in_parents 
          && (t->type > TK_OP_BEGIN && t->type < TK_OP_END) 
          && t->type <= main_op->type)
        main_op = t;
    }
    Log("main_op->type: %d\n", main_op->type);
    val1 = eval(p, main_op-1, success);
    val2 = eval(main_op+1, q, success);
    Log("val1: %d, val2: %d\n", val1, val2);
    *success = true;
    switch (main_op->type) {
    case TK_ADD: return val1+val2;
    case TK_SUB: return val1-val2;
    case TK_MUL: return val1*val2;
    case TK_DIV: return val1/val2;
    case TK_EQ: return val1 == val2;
      
    default: assert(0);
    }
  }
}
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  return eval(tokens, tokens+nr_token-1, success);
}
