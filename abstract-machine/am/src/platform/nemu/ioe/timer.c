#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  // 我真是服了，非得折腾你一顿去看源码是吧，
  // 好，timer的数据寄存器是64位的，高低部分怎么放也不说，
  // 设置的规则是还必须要先读高位才能给低位附上值，
  // 这么奇葩的设定也不在指导里说明了，让别人自己看？？
  uint32_t us_h =inl(RTC_ADDR+4);
  uint32_t us_l =inl(RTC_ADDR);

  uptime->us = (uint64_t)us_h << 32 | (uint64_t)us_l;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
