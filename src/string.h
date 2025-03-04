#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void* malloc(size_t size);

int strlen(const char* str)
{
        int l;
        for (l = 0; str[l] != '\0'; ++l);
        return l;
}

int strcmp(const char *s1, const char *s2)
{
        while (*s1 == *s2++)
        {
                if (*s1++ == '\0')
                        return (0);
        }
        return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

const char* strrchr(const char* str, int ch)
{
        const char* result = NULL;
        while (*str)
        {
                if (*str == ch)
                {
                        result = str;
                }
                str++;
        }
        return result;
}

void memset(void* des, int value, uint32_t size) {
        if (des == NULL) return;
        for (uint32_t x = 0; x < size; ++x)
                ((char*)des)[x] = value;
}

void memcpy(void* des, const void* src, uint32_t size) {
        if (des == NULL || src == NULL) return;
        for (uint32_t x = 0; x < size; ++x)
                ((char*)des)[x] = ((char*)src)[x];
}

int memcmp(const void *str1, const void *str2, uint32_t n) {
        const unsigned char *p1 = str1;
        const unsigned char *p2 = str2;

        for (uint32_t i = 0; i < n; i++) {
                if (p1[i] != p2[i]) {
                        return p1[i] - p2[i];
                }
        }
        return 0;
}

void clsbuf(char* buf, int size) {
        for (int i = 0; i < size; i++) {buf[i] = 0;}
}

int chex(unsigned char c) {
        if (c >= '0' && c <= '9') {
                return c - '0';
        } else if (c >= 'A' && c <= 'F') {
                return c - ('A'-10);
        } else if (c >= 'a' && c <= 'f') {
                return c - ('a'-10);
        }
        return -1;
}

int strhex(char* s) {
        int a=0;
        for (int i = 0; s[i] != '\0'; i++) {
                a<<=4;
                char c = chex(s[i]);
                if (c == -1) return a;
                a+=c;
        }
        return a;
}

uint8_t hchar(uint8_t c) {
        c&=15;
        if (c <= 9) {
                return c + '0';
        } else if (c >= 0xA && c <= 0xF) {
                return c + ('A'-10);
        }
        return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
        while (n > 0 && *s1 && *s2 && *s1 == *s2) {
                s1++;
                s2++;
                n--;
        }
        if (n == 0) return 0;
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

char* strncpy(char *dest, const char *src, size_t n) {
        char *d = dest;
        const char *s = src;
        size_t i;

        for (i = 0; i < n && *s != '\0'; i++) {
                *d++ = *s++;
        }
        for (; i < n; i++) {
                *d++ = '\0';
        }
        return dest;
}

char* strpbrk(const char* s, const char* accept) {
        if (s == NULL || accept == NULL)
                return NULL;

        while (*s != '\0') {
                const char* p = accept;
                while (*p != '\0') {
                        if (*s == *p)
                                return (char*)s;
                        p++;
                }
                s++;
        }

        return NULL;
}

char* strtok(char* str, const char* delim) {
        static char* token = NULL;
        if (str != NULL) {
                token = str;
        } else if (token == NULL) {
                return NULL;
        }

        char* start = token;
        char* end = strpbrk(token, delim);
        if (end != NULL) {
                *end = '\0';
                token = end + 1;
        } else {
                token = NULL;
        }

        return start;
}

char* strcpy(register char *to, register const char *from) {
        char *save = to;
        for (int k = 0; from[k] != '\0'; ++k) to[k] = from[k];
        return(save);
}

char* strdup(const char* s) {
        if (s == NULL)
                return NULL;

        size_t len = strlen(s);
        char* dup = (char*)malloc(len + 1);
        if (dup == NULL)
                return NULL;

        strcpy(dup, s);
        dup[len]=0;
        return dup;
}

size_t numlen(const char* str) {
        size_t i = 0;
        while (*str >= '0' && *str <= '9') {
                str++; i++;
        }
        return i;
}

int atoi(const char* str) {
        int result = 0;
        int sign = 1;

        // Handle leading whitespace
        while (*str == ' ' || *str == '\t') {
                str++;
        }

        // Handle sign
        if (*str == '-') {
                sign = -1;
                str++;
        } else if (*str == '+') {
                str++;
        }

        // Convert digits to integer
        while (*str >= '0' && *str <= '9') {
                result = result * 10 + (*str - '0');
                str++;
        }

        // Apply sign
        return result * sign;
}

char *strchr(const char *str, int c) {
        while (*str != '\0') {
                if (*str == c) {
                        return (char *)str; // Found the character, return its address
                }
                str++; // Move to the next character in the string
        }
        if (c == '\0') {
                return (char *)str; // Return the null terminator if c is '\0'
        }
        return NULL; // If the character is not found, return NULL
}

size_t wstrlen(const wchar_t* str) {
        size_t len = 0;
        while (str[len])
                ++len;
        return len;
}

void wstrcpy(wchar_t* dst, const wchar_t* src) {
        for (size_t x=0; src[x] != '\0'; ++x)
                dst[x]=src[x];
}

char * strcat(const char* a, const char* b) {
        char* s = malloc(strlen(a) + strlen(b) + 8);
        if (s == NULL) {
                return NULL;
        }
        memcpy(s, a, strlen(a));
        memcpy(s + strlen(a), b, strlen(b) + 1);
        return s;
}

char * concat(const char* str1, const char* str2)
{
        size_t len1 = strlen(str1);
        size_t len2 = strlen(str2);
        size_t total_len = len1 + len2;

        char* result = (char*)malloc(total_len + 1);
        if (result == NULL)
        {
                return NULL;
        }
        strcpy(result, str1);
        strcpy(result+len1, str2);
        result[total_len-1]=0;
        return result;
}

void reverse(char* str, int length) {
        int start = 0;
        int end = length - 1;
        while (start < end) {
                // Swap characters
                char temp = str[start];
                str[start] = str[end];
                str[end] = temp;
                start++;
                end--;
        }
}

char* itoa(int num, char* str, int base) {
        int i = 0;
        int is_negative = 0;

        // Handle 0 explicitly
        if (num == 0) {
                str[i++] = '0';
                str[i] = '\0';
                return str;
        }

        // Handle negative numbers for base 10
        if (num < 0 && base == 10) {
                is_negative = 1;
                num = -num;
        }

        // Process each digit
        while (num != 0) {
                int rem = num % base;
                str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
                num = num / base;
        }

        // If the number is negative, append '-'
        if (is_negative)
                str[i++] = '-';

        // Null-terminate the string
        str[i] = '\0';

        // Reverse the string to get the correct result
        reverse(str, i);

        return str;
}

#endif