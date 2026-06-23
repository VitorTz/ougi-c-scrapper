#ifndef IMAGE_INFO_H
#define IMAGE_INFO_H

/*
 * image_info.h — header-only, sem dependências externas.
 *
 * Detecta formato (PNG, JPEG, WEBP, GIF) e dimensões (width/height) de uma
 * imagem a partir de um buffer cru em memória, lendo apenas os headers —
 * não decodifica pixels.
 *
 * C não tem exceções: falha é reportada via `image_info_t.ok` /
 * `image_info_t.error`, nunca via longjmp ou abort.
 */

#include <stddef.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    IMAGE_FORMAT_UNKNOWN = 0,
    IMAGE_FORMAT_PNG,
    IMAGE_FORMAT_JPEG,
    IMAGE_FORMAT_WEBP,
    IMAGE_FORMAT_GIF,
} image_format_t;

typedef struct {
    image_format_t format;
    uint32_t width;
    uint32_t height;
    bool ok;
    const char* error; /* mensagem estática, válida se !ok */
    const char* extension;
} image_info_t;


/* ---- leitura de inteiros com endianness explícita ---- */

static inline uint16_t img__read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static inline uint32_t img__read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

static inline uint16_t img__read_le16(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static inline uint32_t img__read_le24(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

static inline uint32_t img__read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* ---- detectores por formato ---- */

static bool img__try_png(const uint8_t *d, size_t n, image_info_t *out) {
    static const uint8_t sig[8] = {0x89,'P','N','G','\r','\n',0x1A,'\n'};
    if (n < 24 || memcmp(d, sig, 8) != 0) return false;

    out->format = IMAGE_FORMAT_PNG;
    out->width  = img__read_be32(d + 16); /* chunk IHDR: width, height */
    out->height = img__read_be32(d + 20);
    out->extension = ".png";
    return true;
}

static bool img__try_gif(const uint8_t *d, size_t n, image_info_t *out) {
    if (n < 10) return false;
    if (memcmp(d, "GIF87a", 6) != 0 && memcmp(d, "GIF89a", 6) != 0) return false;

    out->format = IMAGE_FORMAT_GIF;
    out->width  = img__read_le16(d + 6); /* logical screen descriptor */
    out->height = img__read_le16(d + 8);
    out->extension = ".gif";
    return true;
}

static bool img__try_webp(const uint8_t *d, size_t n, image_info_t *out) {
    if (n < 30 || memcmp(d, "RIFF", 4) != 0 || memcmp(d + 8, "WEBP", 4) != 0)
        return false;

    const uint8_t *chunk   = d + 12; /* fourCC do sub-chunk */
    const uint8_t *payload = d + 20; /* dados do sub-chunk */

    if (memcmp(chunk, "VP8 ", 4) == 0) {
        /* lossy: 3 bytes de start code + duas dimensões de 14 bits */
        if (payload[0] != 0x9d || payload[1] != 0x01 || payload[2] != 0x2a)
            return false;
        out->width  = img__read_le16(payload + 3) & 0x3FFF;
        out->height = img__read_le16(payload + 5) & 0x3FFF;
    } else if (memcmp(chunk, "VP8L", 4) == 0) {
        /* lossless: 1 byte de assinatura + dimensões empacotadas em 32 bits */
        if (payload[0] != 0x2F) return false;
        uint32_t bits = img__read_le32(payload + 1);
        out->width  = (bits & 0x3FFF) + 1;
        out->height = ((bits >> 14) & 0x3FFF) + 1;
    } else if (memcmp(chunk, "VP8X", 4) == 0) {
        /* extended: 4 bytes de flags + duas dimensões de 24 bits (canvas) */
        out->width  = img__read_le24(payload + 4) + 1;
        out->height = img__read_le24(payload + 7) + 1;
    } else {
        return false;
    }

    out->format = IMAGE_FORMAT_WEBP;
    out->extension = ".webp";
    return true;
}

static bool img__try_jpeg(const uint8_t *d, size_t n, image_info_t *out) {
    if (n < 4 || d[0] != 0xFF || d[1] != 0xD8) return false;

    size_t pos = 2;
    while (pos + 4 <= n) {
        if (d[pos] != 0xFF) { pos++; continue; } /* ressincroniza em fill bytes */

        uint8_t marker = d[pos + 1];
        pos += 2;

        if (marker == 0xD8 || marker == 0xD9 || marker == 0x01 ||
            (marker >= 0xD0 && marker <= 0xD7))
            continue; /* marcadores sem payload (SOI/EOI/RST/TEM) */

        if (pos + 2 > n) break;
        uint16_t seg_len = img__read_be16(d + pos);

        bool is_sof = marker >= 0xC0 && marker <= 0xCF &&
                      marker != 0xC4 && marker != 0xC8 && marker != 0xCC;

        if (is_sof) {
            if (pos + 7 > n) break;
            out->format = IMAGE_FORMAT_JPEG;
            out->height = img__read_be16(d + pos + 3);
            out->width  = img__read_be16(d + pos + 5);
            out->extension = ".jpg";
            return true;
        }

        if (marker == 0xDA) break; /* start of scan: nenhum SOF encontrado antes */
        if (seg_len < 2) break;    /* segmento malformado */
        pos += seg_len;
    }
    return false;
}

/*
 * Inspeciona `data` (tamanho `size`) e preenche `out` com formato e
 * dimensões. Retorna o struct por valor (sem alocação).
 *
 * Em caso de falha (formato não reconhecido ou buffer truncado),
 * `out.ok == false` e `out.error` aponta para uma mensagem estática —
 * esse é o "lançamento de exceção" em termos de C.
 */
static inline image_info_t image_get_info(const uint8_t *data, const size_t size) {
    image_info_t out = {
        .format = IMAGE_FORMAT_UNKNOWN,
        .width = 0,
        .height = 0,
        .ok = false,
        .error = NULL,
        .extension = ""
    };
    
    bool found = data && size > 0 &&
        (
            img__try_webp(data, size, &out) ||
            img__try_jpeg(data, size, &out) ||
            img__try_png(data, size, &out)  ||
            img__try_gif(data, size, &out)
        );

    if (found) {
        out.ok = true;
    } else {
        out.ok    = false;
        out.error = "image_get_info: formato nao reconhecido ou buffer truncado";
    }
    return out;
}

#endif /* IMAGE_INFO_H */