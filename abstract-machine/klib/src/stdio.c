#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define PRINT_MAX_SIZE 512

char printbuf[PRINT_MAX_SIZE];
static const char xdigitmap[] = "0123456789abcdef";
static const char Xdigitmap[] = "0123456789ABCDEF";

static char *fmt_d(char *buf, const char *bufend, 
                    int64_t val, int minw, int minus,
                    int zeropad, char is_64)
{
  char revnum[32] = {0}; // enough for store 64-bit decimal number
  uint64_t uval = val;
  int idx = 0;
  int pad, is_neg = 0;

  if (0 == is_64 && (int32_t)val < 0) {
    is_neg = 1;
    uval = (uint32_t)-val;
  }   
  if (1 == is_64 && val < 0) {
    is_neg = 1;
    uval = (uint64_t)-val;
  }

  do {
    revnum[idx++] = xdigitmap[uval % 10];
    uval = uval / 10; 
  } while (uval != 0);

  if (is_neg) 
    revnum[idx++] = '-';

  pad = idx < minw ? minw-idx : 0;
  if (minus) {
    memmove(revnum+pad, revnum, idx);
    memset(revnum, zeropad ? '0' : ' ', pad);
  }
  else {
    memset(revnum+idx, zeropad ? '0' : ' ', pad);
  }
  idx += pad;
  
  while (--idx >= 0 && buf < bufend) {
    *buf++ = revnum[idx];
  }   
  return buf;
}

static char *fmt_u(char *buf, const char *bufend, 
                    uint64_t val, int minw, int minus,
                    int zeropad, char is_64)
{
  char revnum[32] = {0}; // enough for store 64-bit decimal number
  uint64_t uval = val;
  int  idx = 0;
  int pad;

  do {
    revnum[idx++] = xdigitmap[uval % 10];
    uval = uval / 10; 
  } while (uval != 0);

  pad = idx < minw ? minw-idx : 0;
  if (minus) {
    memmove(revnum+pad, revnum, idx);
    memset(revnum, zeropad ? '0' : ' ', pad);
  }
  else {
    memset(revnum+idx, zeropad ? '0' : ' ', pad);
  }
  idx += pad;

  while (--idx >= 0 && buf < bufend) {
    *buf++ = revnum[idx];
  }   
  return buf;
}

static char* fmt_x(char *buf, char *bufend, 
                    unsigned int val, int minw, int minus,
                    int zeropad, char is_upper)
{
  char revhex[32] = {0}; // enough for store 64-bit hex
  int  idx = 0;
  int pad;
  const char *map;

  map = (is_upper == 0) ? xdigitmap : Xdigitmap;
  do {
    revhex[idx++] = map[val & 0xf];
    val = val >> 4;
  } while (val != 0);

  pad = idx < minw ? minw-idx : 0;

  if (minus) {
    memmove(revhex+pad, revhex, idx);
    memset(revhex, zeropad ? '0' : ' ', pad);
  }
  else {
    memset(revhex+idx, zeropad ? '0' : ' ', pad);
  }

  idx += pad;
  while (--idx >= 0 && buf < bufend) {
    *buf++ = revhex[idx];
  }  
  return buf;
}

static char* fmt_lx(char *buf, char *bufend, 
                      unsigned long val, int minw, int minus, 
                      int zeropad, char is_upper) 
{
  char revhex[32] = {0}; // enough for store 64-bit hex
  int  idx = 0;
  int pad;
  const char *map;

  map = (is_upper == 0) ? xdigitmap : Xdigitmap;
  do {
    revhex[idx++] = map[val & 0xf];
    val = val >> 4;
  } while (val != 0);

  pad = idx < minw ? minw-idx : 0;

  if (minus) {
    memmove(revhex+pad, revhex, idx);
    memset(revhex, zeropad ? '0' : ' ', pad);
  }
  else {
    memset(revhex+idx, zeropad ? '0' : ' ', pad);
  }

  idx += pad;
  while (--idx >= 0 && buf < bufend) {
    *buf++ = revhex[idx];
  }
  return buf;
}

static char* fmt_p(char *buf, char *bufend, unsigned long val)
{
  char revhex[32] = {0}; // enough for store 64-bit hex
  int idx = 0;
  const char *map;

  map = xdigitmap;  // lower for 'p'?

  // put '0x' in front of the buffer result
  *buf++ = '0';
  *buf++ = 'x';

  do {
    revhex[idx++] = map[val & 0xf];
    val = val >> 4;
  } while (val != 0);

  while (--idx >= 0 && buf < bufend) {
    *buf++ = revhex[idx];
  }  
  return buf;  
} 

static char* fmt_s(char *buf, const char *const bufend, 
                      const char* s, int minw, int minus)
{
  size_t len = strlen(s);
  int pad;

  assert(len > 0);

  pad = len < minw ? minw-len : 0;

  if (minus) {
    while (buf < bufend && *s != '\0')
      *buf++ = *s++;
    while (pad-- > 0 && buf < bufend)
      *buf++ = ' ';
  }
  else {
    while (pad-- > 0 && buf < bufend)
      *buf++ = ' ';
    while (buf < bufend && *s != '\0')
      *buf++ = *s++;   
  }
  return buf;
}


int printf(const char *fmt, ...)
{
  char buf[128]; 
  va_list ap;
  int count;
  char *cur = buf;  

  va_start(ap, fmt);
  count = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  while(count > 0)
  {
    putch(*cur);
    ++cur, --count;
  }
  return cur - buf; 
}

int vsprintf(char *out, const char *fmt, va_list ap) {       
    /* size_t belongs to unsigned, so it is vaild.           
     * The value also could be INT_MAX, depends on implement.
     */                                                      
    return vsnprintf(out, -1, fmt, ap);                      
}                                                            

int sprintf(char *out, const char *fmt, ...) {
    int r;                                    
    va_list args;                             
    va_start(args, fmt);                      
    r = vsprintf(out, fmt, args);             
    va_end(args);                             
    return r;                                 
}                                             

int snprintf(char *out, size_t n, const char *fmt, ...) {
    int r;
    va_list args;
    va_start(args, fmt);
    r = vsnprintf(out, n, fmt, args);
    va_end(args);
    return r;
}

// Supported format specifiers:
//  %s %d                   
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
  char * const bufstart = buf;
  char * const bufend = buf + size - 1; // reserved for '\0'
  int minw, zeropad, minus;

  while(buf < bufend && *fmt != '\0') {
    if(*fmt == '%') {
      ++fmt;
      
      minw = 0;
      zeropad = 0;
      minus = 0;

      for (;;) {
        if (*fmt == '0')
          zeropad = 1;
        else if (*fmt == '-')
          minus = 1;
        else 
          break;
        fmt++;
      }

      while (*fmt >= '0' && *fmt <= '9') {
        minw = minw*10 + ((*fmt) - '0');
        ++fmt;
      }

      switch(*fmt) {
        case 'c':
          *buf++ = (char)va_arg(ap, int);
          break;
        case 'l':
          ++fmt;
          if('d' == *fmt)
            buf = fmt_d(buf, bufend, va_arg(ap, int64_t), minw, minus, zeropad, 1);
          else if ('x' == *fmt) 
            buf = fmt_lx(buf, bufend, va_arg(ap, unsigned long), minw, minus, zeropad, 0);  
          else if ('X' == *fmt) 
            buf = fmt_lx(buf, bufend, va_arg(ap, unsigned long), minw, minus, zeropad, 1);   
          else if ('u' == *fmt)
            buf = fmt_u(buf, bufend, va_arg(ap, uint64_t), minw, minus, zeropad, 1);           
          break;                   
        case 'd':
          buf = fmt_d(buf, bufend, va_arg(ap, int), minw, minus, zeropad, 0);           
          break;
        case 'u':
          buf = fmt_u(buf, bufend, va_arg(ap, unsigned int), minw, minus, zeropad, 0);           
          break;
        case 'x':
          buf = fmt_x(buf, bufend, va_arg(ap, unsigned int), minw, minus, zeropad, 0);
          break;        
        case 'X':
          buf = fmt_x(buf, bufend, va_arg(ap, unsigned long), minw, minus, zeropad, 1);
          break;      
        case 'p':
          buf = fmt_p(buf, bufend, va_arg(ap, unsigned long));
          break;       
        case 's':
          buf = fmt_s(buf, bufend, va_arg(ap, char*), minw, minus);
          break;
        case 'b':
          // buf = fmt_b(buf, bufend, va_arg(ap, int64_t));
          break;                   
        default:
          goto label_ret;
      }
    }
    else
    {
      *buf = *fmt, ++buf;
      /* Add <CR> after <LF> automatically, DOS OS may need this */
      // if(*fmt == '\n')  *buf = '\r', ++buf;  
    }
    ++fmt;
  }
  
label_ret:
  *buf = '\0';
  return buf - bufstart;
}

#endif
