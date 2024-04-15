#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i;            

  for (i = 0; s[i] != '\0'; i++);
  return i;            
}

char *strcpy(char* dst,const char* src) {
    const char *p = src;
    char *q = dst;

    assert(dst);
    while(*p != '\0') {
        *q = *p;
        p++;
        q++;
    }
    *q = '\0'; // add terminator
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
    const char *p = src;
    char *q = dst;
    int i;

    assert(dst);
    for (i = 0; i < n && *p != '\0'; i++)
        *(q++) = *(p++);
    for (; i < n; i++) 
        *q++ = '\0';
    return dst;
}

char* strcat(char* dst, const char* src) {
    size_t dst_len = strlen(dst);
    size_t i;

    for (i = 0; src[i] != '\0'; i++) {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = '\0';
    
  return dst;
}

int strcmp(const char* s1, const char* s2) { 
    assert(NULL != s1 && NULL != s2);

    const char *p = s1;
    const char *q = s2;
    int res = 0;

    while (*p != '\0' && *q != '\0') {
        if (*p == *q) {
            p++;
            q++;
        } else {
            res = *p - *q;
            return res;
        }
    }

    if (*p == '\0' && *q != '\0') res = -1;
    if (*p != '\0' && *q == '\0') res = 1;

    return res;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    const char *p = s1;
    const char *q = s2;
    int i, rst;

    for (rst = 0, i = 0; i < n; i++) {
        if (*q == '\0' || *p == '\0') break;
        if (*q != *p) {
            return (*p - *q);
        }
        p++; q++;
    }
    if (*p == '\0' && *q != '\0') rst = -1;
    if (*p != '\0' && *q == '\0') rst = 1;
    return rst;
}

void* memset(void* v,int c,size_t n) {
    char *p = (char *)v;
    
    while (n--) {
        *p++ = c;
    }

    return v;
}

void* memmove(void* dst,const void* src,size_t n) {
    unsigned char *pdst = (unsigned char *)dst;
    const unsigned char *psrc = (unsigned char *)src;
    int i;

    assert(dst && src);
    if (pdst < psrc) {
        for (i = 0; i < n; i++) 
            *pdst++ = *psrc++;
    } else {
        // Reserve order
        for (i = n - 1; i >= 0; i--)
            *(pdst + i) = *(psrc + i);
    }
    return dst;
}

void* memcpy(void* out, const void* in, size_t n) {
    unsigned char *p = out;
    const unsigned char *q = in;
    unsigned char *rst = p; // return value
    int i;

    assert(out != NULL && in != NULL);
    for (i = 0; i < n; i++) {
        *p++ = *q++;
    }

    return rst;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    assert(NULL != s1 && NULL != s2);

    const unsigned char *p = s1;
    const unsigned char *q = s2;
    int res = 0;

    while (n--) {
        if (*p == *q) {
            p++;
            q++;
        } else {
            res = *p - *q;
            return res;
        }
    }

    return 0;
}

#endif
