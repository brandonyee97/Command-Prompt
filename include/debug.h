#ifndef DEBUG_H
#define DEBUG_H
#include <stdlib.h>
#include <stdio.h>

#ifdef COLOR
#define KNRM "\x1B[0m"
#define KRED "\x1B[1;31m"
#define KGRN "\x1B[1;32m"
#define KYEL "\x1B[1;33m"
#define KBLU "\x1B[1;34m"
#define KMAG "\x1B[1;35m"
#define KCYN "\x1B[1;36m"
#define KWHT "\x1B[1;37m"
#define KBWN "\x1B[0;33m"
#else
/* Color was either not defined or Terminal did not support */
#define KNRM ""
#define KRED ""
#define KGRN ""
#define KYEL ""
#define KBLU ""
#define KMAG ""
#define KCYN ""
#define KWHT ""
#define KBWN ""
#endif

#ifdef DEBUG
#define debug(S, ...)   do{fprintf(stderr, KMAG "DEBUG: %s:%s:%d " KNRM S,   __FILE__, __extension__ __FUNCTION__, __LINE__, __VA_ARGS__  );}while(0)
#define error(S, ...)   do{fprintf(stderr, KRED "ERROR: %s:%s:%d " KNRM S,   __FILE__, __extension__ __FUNCTION__, __LINE__, __VA_ARGS__  );}while(0)
#define warn(S, ...)    do{fprintf(stderr, KYEL "WARN: %s:%s:%d " KNRM S,    __FILE__, __extension__ __FUNCTION__, __LINE__, __VA_ARGS__  );}while(0)
#define info(S, ...)    do{fprintf(stderr, KBLU "INFO: %s:%s:%d " KNRM S,    __FILE__, __extension__ __FUNCTION__, __LINE__, __VA_ARGS__  );}while(0)
#define success(S, ...) do{fprintf(stderr, KGRN "SUCCESS: %s:%s:%d " KNRM S, __FILE__, __extension__ __FUNCTION__, __LINE__, __VA_ARGS__  );}while(0)
#else
#define debug(S, ...)
#define error(S, ...)
#define warn(S, ...)
#define info(S, ...)
#define success(S, ...)
#endif


#endif /* DEBUG_H */
