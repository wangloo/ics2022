#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define PRINT_MAX_SIZE 100

char printbuf[PRINT_MAX_SIZE];
static void print_str(char **out, size_t *left, int *ret, const char *in) {
    int len = strlen(in);
    int i;

    for (i = 0; i < len; i++) {
        if (*left > 1) {
            **out = in[i];
            (*out)++;
            (*left)--;
            (*ret)++;
        }
    }
}

/* 20 is enough for all integer */
#define DEC_MAX_LEN  20
 void print_dec(char **out, size_t *left, int *ret, const int in) {
    unsigned int v;
    int bit = 0;
    char buf[DEC_MAX_LEN];
    int buf_index = 0;
    int i;
    int neg_flag = 0;

    /* Negitive number? */
    if (in < 0) {
        neg_flag = 1;
        v = -in;
    } else
        v = in;
    /* Trans integer to inverted buffer */
    do {
        bit = v % 10;
        buf[buf_index++] = bit + '0';
        v = v / 10;
    } while ( v != 0);
    if (neg_flag == 1) buf[buf_index++] = '-';

    buf[buf_index] = '\0';
    /* Store to output parameter in correct order */
    for (i = buf_index - 1; i >= 0; i--) {
        if (*left > 1) {
            **out = buf[i];
            (*out)++;
            (*left)--;
            (*ret)++;
        }
    }
}


int printf(const char *fmt, ...) {
  panic("Not implemented");
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
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    char *s = out;                                           
    char conv;                                               
    size_t left = n;                                         
    int ret = 0;                                             
                                                             
    while ('\0' != *fmt) {                                   
        /* Copy string before % */                           
        while('\0' != *fmt && '%' != *fmt) {                 
            if (left > 1) {                                  
                *(s++) = *(fmt++);                           
                left--;                                      
            } else {                                         
                /* If not return there, process below must kn ow left=1 */                                                 
                fmt++;                                       
            }                                                
            ret++;                            
        }                                     
                                              
        /* We are at end */                   
        if ('\0' == *fmt) {                   
            *s = '\0';                        
            break;                            
        }                                     
                                              
        /* Jump over % */                     
        fmt++;                                
                                              
        /* Get the conversion */              
        if ('\0' == *fmt) conv = 0;           
        else conv = *(fmt++);                 
                                              
        /* Print that argument designation */ 
        switch (conv) {                                      
            case 's':                                        
                print_str(&s, &left, &ret, va_arg(ap, char*));                                                            
                break;                                       
            case 'd':                                        
                print_dec(&s, &left, &ret, va_arg(ap, int)); 
                break;                                       
            case  0 : break;                                 
            default : break;                                 
        }                                                    
                                                             
        *s = '\0';                                           
    }                                                        
                                                             
  return ret;
}

#endif
