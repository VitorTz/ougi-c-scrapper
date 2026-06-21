#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

/*
 * image_utils.h
 *
 * Dependencies:
 *   - libcurl       (download)
 *   - libwebp       (encode/compress WebP)
 *   - stb_image     (decode any image format)
 *   - stb_image_resize2 (resize)
 *
 * Linux:   sudo apt install libcurl4-openssl-dev libwebp-dev
 *
 * In exactly ONE .c file, before including this header, define:
 *
 *   #define IMAGE_IMPL
 *   #include "image.h"
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <raylib.h>
#include "types.h"
#include "structure/vector.h"
#include "structure/path.h"
#include "structure/string_t.h"
#include "arena.h"
#include "stb/image_info.h"


typedef struct PixelBuffer {
    image_info_t info;
    uint8_t* data;
    size_t num_bytes;
    int w, h, ch, out_ch;
} PixelBuffer;


PixelBuffer load_img_from_disk(const path_t* path);


/**
 * @brief Downloads an image from a URL and streams it directly into a memory arena.
 *
 * This function spawns a subprocess using a custom curl build (`curl_chrome116`) to 
 * bypass basic bot protections. It streams the binary response directly into the 
 * provided ScratchArena in chunks, avoiding intermediate heap allocations.
 *
 * @note The memory pointed to by the returned `Read.data` is owned by the `ScratchArena`.
 * Do NOT call `free()` on it. The memory will be automatically reclaimed the 
 * next time `arena_reset()` is called.
 * @note This function requires a valid `cookies.txt` file in the current working directory.
 *
 * @param url     The HTTP/HTTPS URL of the target image. Cannot be NULL.
 * @param referer The HTTP Referer to spoof. If NULL, defaults to DEFAULT_REFERER.
 * @param arena   Pointer to the ScratchArena where the data chunks will be appended. Cannot be NULL.
 *
 * @return A `Read` struct containing:
 * - `data`: Pointer to the start of the downloaded image inside the arena.
 * - `bytes`: The exact file size in bytes.
 * - `success`: `true` if the download completed successfully.
 * On network failure, empty response, or buffer overflow, it returns a 
 * zero-initialized struct (`success = false`, `data = NULL`).
 */
Read download_img(
    const char* url,
    const char* referer,
    ScratchArena* arena
);

/**
 * @brief Downloads, resizes, slices, and compresses a batch of manga/webtoon images.
 *
 * This function executes a complete end-to-end image processing pipeline:
 * 1. **Download:** Fetches all provided URLs into memory using a ScratchArena 
 * and saves the raw files to the target directory.
 * 2. **Resize:** Reads the downloaded images in numerical order. If an image's width 
 * exceeds `IMAGE_MAX_WIDTH`, it downscales it linearly while preserving the aspect ratio.
 * 3. **Slice:** If the resulting image is taller than `IMAGE_MAX_HEIGHT` (common in 
 * vertical scrolling manhwas), it slices the image horizontally into smaller, manageable chunks.
 * 4. **Compress:** Encodes each chunk into the highly efficient WebP format.
 * 5. **Cleanup:** Moves the processed WebP slices from a temporary folder back into 
 * the target directory, overwriting the original raw downloads.
 *
 * @note This function is destructive to the target directory. It will recursively delete
 * any existing contents in `dir` before starting the download process.
 *
 * @param dir     The target path where the final processed `.webp` slices will be stored.
 * @param urls    A vector of URLs representing the chapter images to download.
 * @param referer The HTTP Referer to spoof during the network request. Defaults to `DEFAULT_REFERER` if NULL.
 */
void download_imgs(path_t dir, string_t* urls, const char* referer);


/* Encodes a block of raw RGB/RGBA pixels into a lossy WebP image using
 * libwebp's advanced encoding API. The simple WebPEncodeRGB/RGBA helpers
 * only expose the `quality` knob; the advanced API also exposes encoder
 * effort and the sharp YUV conversion, both of which matter a lot for
 * line art and small text (speech bubbles, sound effects) typical of
 * manhwa pages.
 *
 * On success returns true and sets out_data and out_size to a buffer owned
 * by libwebp (must be released with WebPFree by the caller).
 * On failure returns false and leaves *out_data untouched. */
bool encode_webp_slice(
    const uint8_t* pixels, 
    int width, 
    int height,
    int stride, 
    int channels, 
    float quality,
    uint8_t** out_data, 
    size_t* out_size
);

bool encode_webp(
    const uint8_t* pixels,
    int width,
    int height,
    int channels,
    uint8_t** out_data,
    size_t* out_size
);

// Resizes pixel data to fit within max_width and/or max_height, preserving
// aspect ratio on each constrained axis independently.
//
// If max_width == 0, the width constraint is skipped entirely (so height is
// left untouched by this step). The same applies for max_height (width is
// left untouched if max_height == 0).
//
// On success, *width and *height are updated and a newly allocated buffer
// is returned. The caller is responsible for freeing it.
// Returns NULL if no resize was necessary (caller should keep using the
// original `pixels` buffer in that case).
uint8_t* resize_pixels(
    uint8_t* pixels, 
    size_t channels, 
    int* width, 
    int* height,
    int max_width, 
    int max_height
);


#endif
