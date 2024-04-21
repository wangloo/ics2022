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

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"
#include "macro.h"

#define RET_FALSE_IF_NEQ(a, b) do {if (a!=b) { Log("REG: %x, DUT: %x\n", a, b); return false;}}while(0)

#define CHECK_DIFF(a, b, name) \
  do { \
    if (a != b) { \
      printf("Check %s false(REF: %x, DUT: %x)\n", name, a, b); \
      return false; \
    }}while(0);

#define CHECK_EQ(a, b) 
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  CHECK_DIFF(ref_r->csr.mtvec, cpu.csr.mtvec, "mtvec");
  CHECK_DIFF(ref_r->pc, cpu.pc, "pc");
  // CHECK_DIFF(ref_r->csr.mstatus.val, cpu.csr.mstatus.val, "mstatus");
  CHECK_DIFF(ref_r->csr.mcause, cpu.csr.mcause, "mcause");
  CHECK_DIFF(ref_r->csr.mepc, cpu.csr.mepc, "mepc");
  for (int i=0; i < ARRLEN(ref_r->gpr); i++) {
    CHECK_DIFF(ref_r->gpr[i], gpr(i), reg_name(i));
  }
  return true;
}

void isa_difftest_attach() {
}
