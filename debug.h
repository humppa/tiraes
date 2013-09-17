#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void pprint(const char *msg, void *data);
void ppkeysched(const char *msg, uint8_t *exp, uint8_t Nb, uint8_t Nr);

#endif
