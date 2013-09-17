/* tiraes.c
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
#include "tiraes.h"

bool verbose = false;

void say(const char *msg) {
    if (verbose) printf("%s\n", msg);
}

void* malloc_or_die(size_t size) {
    void *ptr;

    if ((ptr = malloc(size)) == NULL) {
        err(EXIT_FAILURE, NULL);
    }

    return ptr;
}

/* Multiplication in Rijndael's Galois field
 * using modified peasant's algorithm.
 */
uint8_t galois(uint32_t a, uint8_t b) {
    uint8_t p = 0;

    for (int i=0; i<8 && a!=0 && b!=0; i++) {
        if (b & 1) p ^= a;
        b >>= 1;
        a <<= 1;
        if (a & 0x100) {
            a &= 0xff;
            a ^= 0x1b;
        }
    }

    return p;
}

uint8_t* galois_precalc(uint8_t mul) {
    uint8_t *G_table = (uint8_t*) malloc_or_die(256);

    for (int i=0; i<256; i++) {
        G_table[i] = galois(mul, i);
    }

    return G_table;
}

void sub_byte(void *state) {
    uint8_t *s = (uint8_t*) state;

    for (int i=0; i<16; i++) {
        s[i] = S_box[s[i]];
    }
}

void shift_rows(void *state) {
    uint8_t buf;
    uint8_t *s = (uint8_t*) state;

    buf = s[1];
    s[1] = s[5];
    s[5] = s[9];
    s[9] = s[13];
    s[13] = buf;

    buf = s[2];
    s[2] = s[10];
    s[10] = buf;

    buf = s[6];
    s[6] = s[14];
    s[14] = buf;

    buf = s[15];
    s[15] = s[11];
    s[11] = s[7];
    s[7] = s[3];
    s[3] = buf;
}

void mix_columns(void *state) {
    void *copy = malloc_or_die(16);

    uint8_t *c = (uint8_t*) copy;
    uint8_t *s = (uint8_t*) state;

    memcpy(c, s, 16);

    for (int i=0; i<16; i+=4) {
        s[i]   = G2[c[i]] ^ G3[c[i+1]] ^ c[i+2] ^ c[i+3];
        s[i+1] = c[i] ^ G2[c[i+1]] ^ G3[c[i+2]] ^ c[i+3];
        s[i+2] = c[i] ^ c[i+1] ^ G2[c[i+2]] ^ G3[c[i+3]];
        s[i+3] = G3[c[i]] ^ c[i+1] ^ c[i+2] ^ G2[c[i+3]];
    }
}

void add_round_key(void *state, void *keysched, int round) {
    uint32_t *state_w    = (uint32_t*) state;
    uint32_t *keysched_w = (uint32_t*) keysched;

    state_w[0] ^= keysched_w[Nb*round];
    state_w[1] ^= keysched_w[Nb*round+1];
    state_w[2] ^= keysched_w[Nb*round+2];
    state_w[3] ^= keysched_w[Nb*round+3];
}

void sub_word(uint32_t *word) {
    uint8_t *word_b = (uint8_t*) word;

    word_b[0] = S_box[word_b[0]];
    word_b[1] = S_box[word_b[1]];
    word_b[2] = S_box[word_b[2]];
    word_b[3] = S_box[word_b[3]];
}

void rot_word(uint32_t *word) {
    uint8_t tmp;
    uint8_t *word_b = (uint8_t*) word;

    tmp = word_b[0];
    *word >>= 8;
    word_b[3] = tmp;
}

uint8_t r_con(uint8_t in) {
    uint8_t out;

    if (in == 0) return 0;

    for (out=1; in>1; in--) {
        out = galois(out, 2);
    }

    return out;
}

void *key_expansion(void *key) {
    void *keysched;
    uint8_t i = 0;
    uint32_t *tmp;
    uint32_t *key_w = (uint32_t*) key;

    keysched = malloc_or_die(4*Nb*(Nr+1));
    tmp = malloc_or_die(4);

    uint32_t *sched_w = (uint32_t*) keysched;

    while (i < Nk) {
        sched_w[i] = key_w[i];
        i++;
    }

    while (i < Nb*(Nr+1)) {
        *tmp = sched_w[i-1];
        rot_word(tmp);
        sub_word(tmp);
        *tmp ^= r_con(i/Nk);
        sched_w[i] = sched_w[i-Nk] ^ *tmp;

        i++;

        for (int j=i+3; j>i; i++) {
            sched_w[i] = sched_w[i-Nk] ^ sched_w[i-1];
        }
    }

    /// pexp("expanded key schedule", keysched);

    free(tmp);
    tmp = NULL;

    return keysched;
}

void cipher(void *key, void *state) {
    void *keysched;

    uint64_t *key_d   = (uint64_t*) key;
    uint64_t *state_d = (uint64_t*) state;

    keysched = key_expansion(key);

    state_d[0] ^= key_d[0];
    state_d[1] ^= key_d[1];

    for (int i=1; i<Nr; i++) {
        printf("round %i\n", i);
        pprint("fight!", state);

        sub_byte(state);

        pprint("substituted bytes", state);

        shift_rows(state);

        pprint("shifted rows", state);

        mix_columns(state);

        pprint("mix'd columns", state);

        add_round_key(state, keysched, i);

        pprint("round key'd", state);
    }

    printf("final round\n");
    pprint("fight!", state);

    sub_byte(state);

    pprint("substituted bytes", state);

    shift_rows(state);

    pprint("shifted rows", state);

    add_round_key(state, keysched, Nr);

    free(keysched);
    keysched = NULL;
}

uint8_t hex_val(int chr) {
    if ('0' <= chr && chr <= '9') {
        return chr - '0';
    }
    else if ('A' <= chr && chr <= 'F') {
        return chr - 'F' + 10;
    }
    else if ('a' <= chr && chr <= 'f') {
        return chr - 'a' + 10;
    }
    else {
        errx(EXIT_FAILURE, "incomprehensible input %c", chr);
    }
}

void set_cipher_options(int bytes) {
    if (bytes == 16) {
        Nb = 4;
        Nk = 4;
        Nr = 10;
    }
    else if (bytes == 24) {
        Nb = 4;
        Nk = 6;
        Nr = 12;
    }
    else if (bytes == 32) {
        Nb = 4;
        Nk = 8;
        Nr = 14;
    }
    else {
        errx(EXIT_FAILURE, "key must be exactly 32, 48 or 64 bytes i.e. 128, 192 or 256 bits");
    }
}

/* FIXME: This is probably broken right now.
 */
void *key_from_file(char *keyfile) {
    int chr;
    int i = 0;
    FILE *fp = NULL;
    void *key = malloc_or_die(KEY_MAX);

    uint8_t *key_b = (uint8_t*) key;

    if ((fp = fopen(keyfile, "r")) == NULL) {
        err(EXIT_FAILURE, "unable to open key file");
    }

    while ((chr = getc(fp)) != EOF && i < KEY_MAX) {
        key_b[i] = chr;
        i++;
    }

    printf("luettiin %i tavua\n", i);

    set_cipher_options(i);

    key = realloc(key, 4*Nk);

    return key;
}

void *key_from_opt(char *str) {
    int bytes = 0;
    char prev = '\0';
    void *key = malloc_or_die(KEY_MAX);

    uint8_t *key_b = (uint8_t*) key;

    for (int j=0; j<strlen(str); j++) {
        switch (str[j]) {
            case '.':
            case ':':
            case '-':
            case ' ':
                continue;
        }

        if (prev == '\0') {
            prev = str[j];
        }
        else {
            key_b[bytes] = (hex_val(prev) << 4) ^ hex_val(str[j]);
            prev = '\0';
            bytes++;
        }
    }

    if (verbose) printf(MSG_READ, bytes);

    set_cipher_options(bytes);

    key = realloc(key, 4*Nk);

    return key;
}

int main(int argc, char *argv[]) {
    int  opt;
    void *key = NULL;
    void *state = NULL;
    char *keyfile = NULL;
    char *keyopt = NULL;
    uint8_t *state_b = NULL;
    uint32_t *state_w = NULL;

    /* tiraes [-e | -d] (-h <hexkey> | -f <filekey>) [-v]
     */
    while ((opt = getopt(argc, argv, "def:h:v")) != -1) {
        switch (opt) {
            case 'd':
                break;
            case 'e':
                break;
            case 'f':
                keyfile = optarg;
                break;
            case 'h':
                keyopt = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case '?':
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (verbose) puts(MSG_LOOKUP);

    G2 = galois_precalc(2);
    G3 = galois_precalc(3);

    if (keyopt != NULL) {
        key = key_from_opt(keyopt);
    }
    else if (keyfile != NULL) {
        key = key_from_file(keyfile);
    }
    else {
        errx(EXIT_FAILURE, "%s", E_NOKEY);
    }

    state = malloc_or_die(4*Nk);

    state_b = (uint8_t*) state;
    state_w = (uint32_t*) state;

    pprint("cipher key", key);

    for (int c=0; c<4; c++) {
        for (int r=0; r<4; r++) {
            if ((state_b[4*r+c] = (unsigned char)getchar()) == EOF) {
                printf("done at %i,%i\n", c, r);
                c = 4;
                break;
            }
        }
    }

    state_w[0] = 0xa8f64332;
    state_w[1] = 0x8d305a88;
    state_w[2] = 0xa2983131;
    state_w[3] = 0x340737e0;

    pprint("initial state", state);

    cipher(key, state);

    pprint("final state", state);

    free(G2);
    free(G3);
    free(key);
    free(state);

    return 0;
}
