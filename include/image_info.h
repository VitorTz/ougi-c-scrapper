#ifndef IMAGE_INFO_H
#define IMAGE_INFO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

/*
 * Inspeciona `data` (tamanho `size`) e preenche `out` com formato e
 * dimensões. Retorna o struct por valor (sem alocação).
 *
 * Em caso de falha (formato não reconhecido ou buffer truncado),
 * `out.ok == false` e `out.error` aponta para uma mensagem estática —
 * esse é o "lançamento de exceção" em termos de C.
 */
image_info_t image_get_info(const uint8_t *data, size_t size);


#endif /* IMAGE_INFO_H */