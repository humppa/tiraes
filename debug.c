/* debug.c
 *
 * Copyright (c) 2013 Tuomas Starck <dev@starck.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "debug.h"

void pprint(const char *msg, void *data) {
    uint8_t *byte = (uint8_t*) data;
    printf("<<< %s\n", msg);
    for (int r=0; r<4; r++) {
        for (int c=0; c<4; c++) {
            printf("  %02x (%02i)", byte[4*c+r], 4*c+r);
        }
        printf("\n");
    }
    printf(">>>\n");
}

void ppkeysched(const char *msg, uint8_t *exp, uint8_t Nb, uint8_t Nr) {
    printf("[[[ %s\n", msg);
    for (int i=0; i<Nb*(Nr+1); i++) {
        for (int j=0; j<4; j++) {
            printf(" %02x", exp[4*i+j]);
        }
        printf("\n");
    }
    printf("]]]\n");
}
