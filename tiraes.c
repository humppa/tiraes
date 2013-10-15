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

#include "tiraes.h"

void *copy = NULL;

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

uint8_t* galois_precalc(uint8_t n) {
    uint8_t *G_table = (uint8_t*) malloc_or_die(256);

    for (int i=0; i<256; i++) {
        G_table[i] = galois(i, n);
    }

    return G_table;
}

void sub_bytes(void *state) {
    uint8_t *s = (uint8_t*) state;

    for (int i=0; i<16; i++) {
        s[i] = S_box[s[i]];
    }
}

void inv_sub_bytes(void *state) {
    uint8_t *s = (uint8_t*) state;

    for (int i=0; i<16; i++) {
        s[i] = S_box_inv[s[i]];
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

void inv_shift_rows(void *state) {
    uint8_t buf;
    uint8_t *s = (uint8_t*) state;

    buf = s[13];
    s[13] = s[9];
    s[9] = s[5];
    s[5] = s[1];
    s[1] = buf;

    buf = s[2];
    s[2] = s[10];
    s[10] = buf;

    buf = s[6];
    s[6] = s[14];
    s[14] = buf;

    buf = s[3];
    s[3] = s[7];
    s[7] = s[11];
    s[11] = s[15];
    s[15] = buf;
}

void mix_columns(void *state) {
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

void inv_mix_columns(void *state) {
    uint8_t *c = (uint8_t*) copy;
    uint8_t *s = (uint8_t*) state;

    memcpy(c, s, 16);

    for (int i=0; i<16; i+=4) {
        s[i]   = Ge[c[i]] ^ Gb[c[i+1]] ^ Gd[c[i+2]] ^ G9[c[i+3]];
        s[i+1] = G9[c[i]] ^ Ge[c[i+1]] ^ Gb[c[i+2]] ^ Gd[c[i+3]];
        s[i+2] = Gd[c[i]] ^ G9[c[i+1]] ^ Ge[c[i+2]] ^ Gb[c[i+3]];
        s[i+3] = Gb[c[i]] ^ Gd[c[i+1]] ^ G9[c[i+2]] ^ Ge[c[i+3]];
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
        out = G2[out];
    }

    return out;
}

void *key_expansion(void *key) {
    void *keysched;
    uint8_t i = 0;
    uint32_t *tmp;
    uint32_t *key_w = (uint32_t*) key;

    tmp = malloc_or_die(4);
    keysched = malloc_or_die(4*Nb*(Nr+1));

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

    free(tmp);

    return keysched;
}

void encrypt_cipher(void *keysched, void *state) {
    add_round_key(state, keysched, 0);

    for (int round=1; round<Nr; round++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, keysched, round);
    }

    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, keysched, Nr);
}

void decrypt_cipher(void *keysched, void *state) {
    add_round_key(state, keysched, Nr);

    for (int round=Nr-1; round>=1; round--) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, keysched, round);
        inv_mix_columns(state);
    }

    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, keysched, 0);
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
        errx(EXIT_FAILURE, E_KEYCHR, chr);
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
        errx(EXIT_FAILURE, "%s", E_KEYLEN);
    }
}

void *key_from_file(char *keyfile) {
    int chr;
    int bytes = 0;
    FILE *fp = NULL;
    void *key = malloc_or_die(KEY_MAX);
    uint8_t *key_b = (uint8_t*) key;

    if ((fp = fopen(keyfile, "r")) == NULL) {
        err(EXIT_FAILURE, "%s", E_FILE);
    }

    while ((chr = getc(fp)) != EOF && bytes < KEY_MAX) {
        key_b[bytes] = (uint8_t) chr;
        bytes++;
    }

    fclose(fp);

    set_cipher_options(bytes);

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
            case ' ':
            case '.':
            case ':':
            case '-':
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

    set_cipher_options(bytes);

    key = realloc(key, 4*Nk);

    return key;
}

/* Input and output.
 *
 * Since AES ciphers only data in 16 byte blocks, encrypted
 * output is padded conforming to ISO/IEC 9797-1 method 2.
 *
 * http://en.wikipedia.org/wiki/ISO/IEC_9797-1#Padding
 */
void crypto_io(opmode_t mode, void *key) {
    int i = 0;
    int chr = '\0';
    void *state = NULL;
    void *keysched = NULL;
    void (*cipher)(void*, void*);

    state = malloc_or_die(16);
    keysched = key_expansion(key);
    cipher = mode == ENCRYPT? &encrypt_cipher: &decrypt_cipher;

    uint8_t *state_b = (uint8_t*) state;

    /* Main IO loop:
     *  1. Fill state with input.
     *  2. Use cipher.
     *  3. Flush out.
     */
    while ((chr = getchar()) != EOF) {
        if (i == 16) {
            i = 0;
            (*cipher)(keysched, state);
            while (i != 16) putchar(state_b[i++]);
            i = 0;
        }

        state_b[i++] = (uint8_t) chr;
    }

    if (mode == ENCRYPT) {
        /* If state is full, flush it.
         */
        if (i == 16) {
            i = 0;
            (*cipher)(keysched, state);
            while (i != 16) putchar(state_b[i++]);
            i = 0;
        }

        /* Fill the state with padding.
         */
        state_b[i++] = ISO_9797_1_ONE_BIT;
        while (i != 16) state_b[i++] = 0;

        /* Encrypt and flush the last state.
         */
        i = 0;
        encrypt_cipher(keysched, state);
        while (i != 16) putchar(state_b[i++]);
    }
    else /* mode == DECRYPT */ {
        if (i != 16) errx(EXIT_FAILURE, "%s", E_INPUT);

        /* Decrypt the last block.
         */
        decrypt_cipher(keysched, state);

        /* Find the start of the padding and
         * flush out preceding data.
         */
        while (state_b[--i] != ISO_9797_1_ONE_BIT);
        for (int j=0; j<i; j++) putchar(state_b[j]);
    }

    free(keysched);
    free(state);
}

int main(int argc, char *argv[]) {
    int  opt;
    void *key = NULL;
    char *keyfile = NULL;
    char *keyopt = NULL;
    opmode_t mode = ENCRYPT;

    /* tiraes [-e | -d] (-k <hexkey> | -f <filekey>)
     */
    while ((opt = getopt(argc, argv, "def:hk:")) != -1) {
        switch (opt) {
            case 'd':
                mode = DECRYPT;
                break;
            case 'e':
                mode = ENCRYPT;
                break;
            case 'f':
                keyfile = optarg;
                break;
            case 'h':
                fprintf(stderr, "Usage: tiraes %s\n", USAGE);
                exit(EXIT_SUCCESS);
                break;
            case 'k':
                keyopt = optarg;
                break;
            case '?':
                errx(EXIT_FAILURE, "%s", USAGE);
                break;
        }
    }

    copy = malloc_or_die(16);

    G2 = galois_precalc(2);
    G3 = galois_precalc(3);
    G9 = galois_precalc(9);
    Gb = galois_precalc(11);
    Gd = galois_precalc(13);
    Ge = galois_precalc(14);

    if (keyopt != NULL) {
        key = key_from_opt(keyopt);
    }
    else if (keyfile != NULL) {
        key = key_from_file(keyfile);
    }
    else {
        errx(EXIT_FAILURE, "%s", E_NOKEY);
    }

    crypto_io(mode, key);

    free(copy);
    free(key);
    free(Ge);
    free(Gd);
    free(Gb);
    free(G9);
    free(G3);
    free(G2);

    return 0;
}
