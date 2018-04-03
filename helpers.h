#ifndef HELPERS_H
#define HELPERS_H

typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

str2int_errno str2int(int* out, char* s);
char* concat(const char* s1, const char* s2);

#endif