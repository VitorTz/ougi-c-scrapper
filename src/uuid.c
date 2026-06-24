#include "../include/uuid.h"
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdio.h>


static void rand_bytes(uint8_t *buf, size_t n) {
    FILE *f = fopen("/dev/urandom", "rb");
    fread(buf, 1, n, f);
    fclose(f);
}

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000);
}


string_t uuid7(void) {
    uint8_t b[16] = {0};

    // Bits [0..47]: timestamp em ms (48 bits, big-endian)
    uint64_t ms = now_ms();
    b[0] = (ms >> 40) & 0xFF;
    b[1] = (ms >> 32) & 0xFF;
    b[2] = (ms >> 24) & 0xFF;
    b[3] = (ms >> 16) & 0xFF;
    b[4] = (ms >>  8) & 0xFF;
    b[5] = (ms >>  0) & 0xFF;

    // Bits [48..127]: aleatórios
    rand_bytes(b + 6, 10);

    // Versão: nibble alto do byte 6 = 0x7
    b[6] = (b[6] & 0x0F) | 0x70;

    // Variante: bits [64..65] = 0b10 (RFC 4122)
    b[8] = (b[8] & 0x3F) | 0x80;

    // Formata como "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    char buf[37];
    snprintf(buf, sizeof(buf),
        "%02x%02x%02x%02x-"
        "%02x%02x-"
        "%02x%02x-"
        "%02x%02x-"
        "%02x%02x%02x%02x%02x%02x",
        b[0],  b[1],  b[2],  b[3],
        b[4],  b[5],
        b[6],  b[7],
        b[8],  b[9],
        b[10], b[11], b[12], b[13], b[14], b[15]);

    return string_from(buf);
}