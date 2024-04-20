#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t keyval = inl(KBD_ADDR);
  kbd->keydown = (keyval & 0x8000)? 1: 0;
  kbd->keycode = keyval &~0x8000;
}
