#include "util/misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void sprintf_alloc(char **out, const char *pattern, ...) {
    size_t size;
    va_list ap, args;

    va_start(ap, pattern);
    va_copy(args, ap);
    size = vsnprintf(NULL, 0, pattern, args);
    *out = malloc((size+1) * sizeof(**out));
    va_end(args);

    va_copy(args, ap);
    vsprintf(*out, pattern, args);
    va_end(args);
    va_end(ap);
}