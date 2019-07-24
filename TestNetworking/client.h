#ifndef client_h
#define client_h

#include <stdio.h>

int connectAndRead(char *error, size_t errorLength, void (^onRead)(char *));

#endif
