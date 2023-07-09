#ifndef LOG_H
#define LOG_H

#include <stdio.h>

/* LOG macro */
#define ERR(x, ...) \
do { \
printf("- \x1b[36m[ERR] \x1b[39m\x1b[31m"x" \x1b[39m\n", ##__VA_ARGS__); \
} while(0)

#ifdef DEVBUILD
#define DEVLOG(x, ...) \
do { \
printf("- \x1b[36m[DEV] \x1b[39m\x1b[34m"x" \x1b[39m\n", ##__VA_ARGS__); \
} while(0)
#else
#define DEVLOG(x, ...) 
#endif

#define LOG(x, ...) \
do { \
printf("- \x1b[36m[LOG] \x1b[39m\x1b[32m"x" \x1b[39m\n", ##__VA_ARGS__); \
} while(0)

#define LOG2(x, ...) \
do { \
printf(""x"\n", ##__VA_ARGS__); \
} while(0)

#endif
