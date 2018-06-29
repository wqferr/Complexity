#ifndef MISC_H
#define MISC_H 1

void sprintf_alloc(char **out, const char *pattern, ...);

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif