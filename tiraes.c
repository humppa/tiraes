/* tiraes.c
 */

#include "tiraes.h"

#if 0
  Nb [= 4]         Number of columns (32-bit words) comprising the State.
  Nk [= 4,6,8]     Number of 32-bit words comprising the Cipher Key.
  Nr [= 10,12,14]  Number of rounds.
#endif

void pprint(const char *msg, uint8_t *data) {
    printf("<<< %s\n", msg);
    for (int r=0; r<4; r++) {
        for (int c=0; c<4; c++) {
            printf(" %c", data[4*r+c]);
        }
        printf("\n");
    }
    printf(">>>\n");
}

void sub_byte(void *state) {
    uint8_t *state_b = (uint8_t*) state;

    for (int i=0; i<16; i++) {
        state_b[i] = S_box[state_b[i]];
    }
}

void shift_rows(void *state) {
    uint8_t  buf_b;
    uint16_t buf_h;

    uint8_t  *state_b = (uint8_t*)  state;
    uint16_t *state_h = (uint16_t*) state;

    buf_b      = state_b[4];
    state_b[4] = state_b[5];
    state_b[5] = state_b[6];
    state_b[6] = state_b[7];
    state_b[7] = buf_b;

    buf_h      = state_h[4];
    state_h[4] = state_h[5];
    state_h[5] = buf_h;

    buf_b       = state_b[15];
    state_b[15] = state_b[14];
    state_b[14] = state_b[13];
    state_b[13] = state_b[12];
    state_b[12] = buf_b;
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

void mix_columns(void *state) {
    uint8_t *s = (uint8_t*) state;

    for (int i=0; i<4; i++) {
        s[i]    = Galois2[s[i]] ^ Galois3[s[i+4]] ^ s[i+8] ^ s[i+12];
        s[i+4]  = s[i] ^ Galois2[s[i+4]] ^ Galois3[s[i+8]] ^ s[i+12];
        s[i+8]  = s[i] ^ s[i+4] ^ Galois2[s[i+8]] ^ Galois3[s[i+12]];
        s[i+12] = Galois3[s[i]] ^ s[i+4] ^ s[i+8] ^ Galois2[s[i+12]];
    }
}

void add_round_key(void *state) {
    return;
}

void cipher(void *key, void *state, int Nr) {
    uint64_t *key_d   = (uint64_t*) key;
    uint64_t *state_d = (uint64_t*) state;

    /* FIXME: Tarkista, että tässä on tarkoitus
     * käyttää salausavainta sellaisenaan ja
     * tee key expansion -juttu.
     */
    state_d[0] ^= key_d[0];
    state_d[1] ^= key_d[1];

    for (int i=0; i<(Nr-1); i++) {
        sub_byte(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state);

        return; /// DEBUG \\\;
    }

    sub_byte(state);
    shift_rows(state);
    add_round_key(state);
}

int main(int argc, char *argv[]) {
    void *key_ptr;
    void *state_ptr;

    key_ptr   = malloc(16);
    state_ptr = malloc(16);

    if (key_ptr == NULL || state_ptr == NULL) {
        errx(EXIT_FAILURE, "failed to reserve memory");
    }

    uint8_t  *key_b = (uint8_t*)  key_ptr;
    uint8_t  *state_b = (uint8_t*)  state_ptr;

    /* Initialize cipher key to be 1010..1010b.
     */
    for (int i=0; i<16; i++) {
        key_b[i] = 170;
    }

    for (int c=0; c<4; c++) {
        for (int r=0; r<4; r++) {
            if ((state_b[4*r+c] = (unsigned char)getchar()) == EOF) {
                printf("done at %i,%i\n", c, r);
                c = 4;
                break;
            }
        }
    }

    pprint("cipher key", key_b);
    pprint("initial state", state_b);

    cipher(key_ptr, state_ptr, 10);

    pprint("final state", state_b);

    printf("galois {53} {ca}: %u (should be 1)\n", galois(0x53, 0xca));
    printf("galois {57} {13}: %u (should be 254)\n", galois(0x57, 0x13));

    free(key_ptr);
    free(state_ptr);

    key_ptr = NULL;
    state_ptr = NULL;

    return 0;
}


#if 0

#endif
