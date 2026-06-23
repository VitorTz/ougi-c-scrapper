#include <sched.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <webp/encode.h>
#include <webp/decode.h>
#include <webp/types.h>

#define IMAGE_IMPL
#include "../include/image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "../include/stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include "../include/stb/stb_image_resize2.h"

#include "../include/structure/vector.h"
#include "../include/structure/path.h"
#include "../include/constants.h"
#include "../include/globals.h"
#include "../include/convert.h"


static bool is_webp(const path_t *path) {
    FILE* f = fopen(path_c_str(path), "rb");
    if (!f) { return false; }

    char buf[12] = {0};
    const size_t readed_bytes = fread(buf, 1, sizeof(buf), f);
    
    fclose(f);
    return (
        readed_bytes == 12
        && memcmp(buf, "RIFF", 4) == 0 
        && memcmp(buf + 8, "WEBP", 4) == 0
    );
}


static PixelBuffer load_webp(const path_t* path) {
    PixelBuffer buf = {0};
    int w, h;

    Read read = path_read_bytes(path);
    if (!read.success) {
        fprintf(stderr, "[Error] WebP read failed: %s\n", path_c_str(path));
        return buf;
    }
    
    buf.info = image_get_info(read.data, read.bytes);

    uint8_t* px = WebPDecodeRGBA(read.data, read.bytes, &w, &h);
    free(read.data);

    if (!px) {
        fprintf(stderr, "[Error] WebPDecodeRGBA failed: %s\n", path_c_str(path));
        return buf;
    }

    buf.data = px;
    buf.w = w;
    buf.h = h;
    buf.ch = 4;
    buf.out_ch = 4;
    buf.num_bytes = (size_t) w * (size_t) h * 4;
    return buf;
}


static PixelBuffer load_non_webp(const path_t* path) {
    PixelBuffer buf = {0};
    int w, h, ch;

    Read read = path_read_bytes(path);
    if (!read.success) {
        fprintf(stderr, "[Error] File read failed: %s\n", path_c_str(path));
        return buf;
    }

    buf.info = image_get_info(read.data, read.bytes);
    uint8_t* px = stbi_load_from_memory(read.data, read.bytes, &w, &h, &ch, 0);

    free(read.data);

    if (!px) {
        const char* r = stbi_failure_reason();
        fprintf(stderr, "[Error] stb_image error: %s — %s\n", (r ? r : "?"), path_c_str(path));
        return buf;
    }

    buf.data = px; 
    buf.w = w;
    buf.h = h;
    buf.ch = ch;
    buf.out_ch = (ch == 4 || ch == 2) ? 4 : 3;
    buf.num_bytes = (size_t)w * (size_t)h * (size_t)ch;
    
    return buf;
}


PixelBuffer load_img_from_disk(const path_t* path) {
    return is_webp(path) ? load_webp(path) : load_non_webp(path);
}

/* Helper function to convert the image format enum to a readable string */
static const char* get_image_format_name(image_format_t format) {
    switch (format) {
        case IMAGE_FORMAT_PNG:  return "PNG";
        case IMAGE_FORMAT_JPEG: return "JPEG";
        case IMAGE_FORMAT_WEBP: return "WEBP";
        case IMAGE_FORMAT_GIF:  return "GIF";
        case IMAGE_FORMAT_UNKNOWN: 
        default:                return "UNKNOWN";
    }
}


/* Prints the contents of a PixelBuffer in a structured format */
void print_pixel_buffer(const PixelBuffer* pb) {
    if (!pb) {
        printf("[PixelBuffer] Error: Pointer is NULL\n");
        return;
    }

    printf("========================================\n");
    printf(" PixelBuffer Dump\n");
    printf("========================================\n");
    
    /* Print nested image_info_t struct */
    printf(" [image_info_t]\n");
    printf("   Status     : %s\n", pb->info.ok ? "OK" : "ERROR");
    if (!pb->info.ok && pb->info.error) {
        printf("   Error Msg  : %s\n", pb->info.error);
    }
    printf("   Format     : %s\n", get_image_format_name(pb->info.format));
    if (pb->info.extension) {
        printf("   Extension  : %s\n", pb->info.extension);
    }
    printf("   Dimensions : %u x %u\n", pb->info.width, pb->info.height);
    printf("----------------------------------------\n");

    /* Print PixelBuffer specific fields */
    printf(" [Buffer Metadata]\n");
    printf("   Dimensions : %d x %d\n", pb->w, pb->h);
    printf("   Channels   : %d (in) / %d (out)\n", pb->ch, pb->out_ch);
    printf("   Data Size  : %zu bytes\n", pb->num_bytes);
    printf("   Data Ptr   : %p\n", (void*)pb->data);
    printf("----------------------------------------\n");

    /* Print a hex preview of the first 16 bytes to help debugging */
    printf(" [Data Preview (up to 16 bytes)]\n   ");
    if (pb->data && pb->num_bytes > 0) {
        size_t preview_size = pb->num_bytes < 16 ? pb->num_bytes : 16;
        for (size_t i = 0; i < preview_size; i++) {
            printf("%02X ", pb->data[i]);
            
            /* Add a space halfway through for better readability */
            if (i == 7) printf(" "); 
        }
        printf("\n");
    } else {
        printf("(Buffer is empty or NULL)\n");
    }
    
    printf("========================================\n");
}


Read download_img(
    const char *url, 
    const char *referer, 
    ScratchArena* arena
) {

    if (!arena || !url) {
        return (Read){0};
    }

    const char* ref = (referer == NULL) ? DEFAULT_REFERER : referer;
    char cmd_buffer[4096];
    
    snprintf(cmd_buffer, sizeof(cmd_buffer),
        "curl_chrome116 --silent --compressed -A '' "
        "-H 'Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8' "
        "-H 'Accept-Language: pt-BR,pt;q=0.9,en-US;q=0.8' "
        "-H 'Sec-Fetch-Dest: image' "
        "-H 'Sec-Fetch-Mode: no-cors' "
        "-H 'Sec-Fetch-Site: same-origin' "
        "--referer '%s' "
        "-b cookies.txt -c cookies.txt --location --max-redirs 10 '%s'",
        ref, url);

    printf("[IMG DOWNLOADING] %s\n", url);
    
    FILE* pipe = popen(cmd_buffer, "r");

    if (!pipe) {
        fprintf(stderr, "[Error] Failed to execute curl for: %s\n", url);
        return (Read){0};
    }

    /* Store the starting pointer in the arena where this image will begin */
    uint8_t* image_start_ptr = arena->memory + arena->used;
    size_t total_downloaded = 0;
    
    uint8_t chunk[4096];
    size_t bytes_read;
    
    /* Read the binary output from curl directly into the arena */
    while ((bytes_read = fread(chunk, 1, sizeof(chunk), pipe)) > 0) {
        if (!arena_push_block(arena, chunk, bytes_read)) {
            fprintf(stderr, "[Error] Failed to copy chunk to arena. Download aborted: %s\n", url);
            pclose(pipe);
            return (Read){0};
        }
        
        total_downloaded += bytes_read;
    }
    
    pclose(pipe);

    /* Validate if we actually downloaded anything */
    if (total_downloaded == 0) {
        fprintf(stderr, "[Error] Downloaded buffer is empty: %s\n", url);
        return (Read){0};
    }

    printf("[IMG DOWNLOADED] %s (%zu bytes)\n", url, total_downloaded);

    return (Read){
        .data = image_start_ptr,
        .bytes = total_downloaded,
        .success = true
    };
}



/* Numerically sorts file paths to prevent "10.jpg" from appearing before "2.jpg" */
static int sort_imgs_numeric(const void* a, const void* b) {
    char* n1 = path_stem((const path_t*) a);
    char* n2 = path_stem((const path_t*) b);
    
    int result = 0;
    
    if (n1 && n2) {
        int val1 = atoi(n1);
        int val2 = atoi(n2);
        
        if (val1 < val2) result = -1;
        else if (val1 > val2) result = 1;
        else result = 0;
    } else if (n1) {
        result = 1;
    } else if (n2) {
        result = -1;
    }

    free(n1);
    free(n2);
    
    return result;
}

static bool is_img_extension(const char* extension) {
    return (
        strcmp(".webp", extension)
        || strcmp(".jpg", extension)
        || strcmp(".jpeg", extension)
        || strcmp(".png", extension)
        || strcmp(".gif", extension)
    );
}


static int filter_img(const void* item) {
    const path_t* path = (path_t*) item;
    char* extension = path_extension(path);
    const int r = path_is_regular_file(path) && is_img_extension(extension);
    free(extension);
    return r;
}


bool encode_webp_slice(
    const uint8_t* pixels, 
    int width, 
    int height,
    int stride, 
    int channels, 
    float quality,
    uint8_t** out_data, 
    size_t* out_size
) {
    WebPConfig config;
    if (!WebPConfigInit(&config)) {
        return false;
    }

    // WEBP_PRESET_DRAWING tunes filter/SNS strength for flat-color artwork
    // with sharp edges (comics, manhwa, line art) instead of photographs.
    if (!WebPConfigPreset(&config, WEBP_PRESET_DRAWING, quality)) {
        return false;
    }

    config.method = 6;        // maximum compression effort: smaller files at the same quality
    config.use_sharp_yuv = 1; // sharper text/line edges during the RGB->YUV conversion
    config.lossless = 0;

    if (!WebPValidateConfig(&config)) {
        return false;
    }

    WebPPicture picture;
    if (!WebPPictureInit(&picture)) {
        return false;
    }
    picture.width = width;
    picture.height = height;

    // ImportRGB/RGBA also sets picture.use_argb internally, which is what
    // makes use_sharp_yuv take effect during WebPEncode().
    const int imported = (channels == 4)
        ? WebPPictureImportRGBA(&picture, pixels, stride)
        : WebPPictureImportRGB(&picture, pixels, stride);

    if (!imported) {
        WebPPictureFree(&picture);
        return false;
    }

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    picture.writer = WebPMemoryWrite;
    picture.custom_ptr = &writer;

    const int encoded = WebPEncode(&config, &picture);
    WebPPictureFree(&picture); // safe regardless of encoded; writer.mem is independent of picture

    if (!encoded) {
        WebPMemoryWriterClear(&writer);
        return false;
    }

    *out_data = writer.mem;
    *out_size = writer.size;
    return true;
}

bool encode_webp(
    const uint8_t *pixels, 
    const int width,
    const int height,
    const int channels,
    uint8_t **out_data, 
    size_t *out_size
) {
    return encode_webp_slice(
        pixels, 
        width, 
        height, 
        width * channels,
        channels, 
        DEFAULT_IMG_QUALITY, 
        out_data, out_size
    );
}


void download_imgs(const path_t dir, string_t* urls, const char* referer) {
    const char* ref = !referer ? DEFAULT_REFERER : referer;
    path_create_directories(&dir);
    path_delete_recursive(&dir, true);
    ScratchArena* arena = globals_get_arena(true);

    // download
    {
        size_t i = 0;
        vec_foreach(string_t, item, urls) {
            arena_reset(arena);            
            // download
            const Read read = download_img(string_cstr(item), ref, arena);
            
            // filepath
            path_t output = path_create_copy(&dir);
            path_append(&output, size_t_to_string(i++));
            const image_info_t info = image_get_info(read.data, read.bytes);
            path_change_extension(&output, info.extension);

            // write
            path_write_bytes(&output, read.data, read.bytes);
        }
    }

    // resize -> convert to webp -> compress
    path_t tmp = path_tmp();
    path_create_directories(&tmp);
    path_delete_recursive(&tmp, true);
    {
        size_t i = 0;
        path_t* entries = path_dir_iterator(&dir, sort_imgs_numeric, filter_img);
        vec_foreach(path_t, item, entries) {
            // 1. load the raw pixel data from disk
            PixelBuffer buf = load_img_from_disk(item);
            if (!buf.data) {
                // skip files that failed to decode instead of aborting the whole batch
                continue;
            }

            int width = buf.w;
            int height = buf.h;
            int channels = buf.out_ch; // actual channel count stored in buf.data (3 or 4)
            stbir_pixel_layout layout = (channels == 4) ? STBIR_RGBA : STBIR_RGB;

            uint8_t* pixels = buf.data;
            bool owns_pixels = false; // becomes true once we allocate a resized buffer

            // 2a. Downscale if the width exceeds the maximum allowed width,
            // preserving the original aspect ratio.
            if (width > IMAGE_MAX_WIDTH) {
                int new_width = IMAGE_MAX_WIDTH;
                int new_height = (int)((double)height * ((double)new_width / (double)width));
                if (new_height < 1) new_height = 1;

                uint8_t* resized = stbir_resize_uint8_linear(
                    pixels, 
                    width, 
                    height, 
                    0,
                    NULL, 
                    new_width, 
                    new_height, 
                    0,
                    layout
                );

                if (resized) {
                    pixels = resized;
                    owns_pixels = true;
                    width = new_width;
                    height = new_height;
                }
            }

            // 2b. If the (possibly resized) image is still taller than the
            // maximum allowed height, slice it into multiple sub-images
            // stacked vertically
            int row_stride = width * channels;
            int remaining_height = height;
            int y_offset = 0;

            do {
                int slice_height = (remaining_height > IMAGE_MAX_HEIGHT) ? IMAGE_MAX_HEIGHT : remaining_height;
                const uint8_t* slice_pixels = pixels + (size_t) y_offset * row_stride;

                // 3. Encode the slice as a lossy WebP image at DEFAULT_IMG_QUALITY,
                // using the advanced encoder settings tuned for line art.
                uint8_t* webp_data = NULL;
                size_t webp_size = 0;
                bool encoded = encode_webp_slice(
                    slice_pixels, 
                    width, 
                    slice_height, 
                    row_stride,
                    channels, 
                    DEFAULT_IMG_QUALITY, 
                    &webp_data, 
                    &webp_size
                );

                if (encoded) {
                    path_t img_path = path_create_copy(&tmp);
                    path_append(&img_path, size_t_to_string(i++));
                    path_change_extension(&img_path, ".webp");
                    path_write_bytes(&img_path, webp_data, webp_size);
                    WebPFree(webp_data);
                }
                y_offset += slice_height;
                remaining_height -= slice_height;
            } while (remaining_height > 0);

            // Release whatever buffers we own for this image
            if (owns_pixels) {
                free(pixels);
            }
            free(buf.data);
        }

        path_dir_iterator_free(entries);
    }
    path_delete_recursive(&dir, true);
    path_move_directory_contents(&tmp, &dir);
}


uint8_t* resize_pixels(
    uint8_t* pixels, 
    const size_t channels, 
    int* width, 
    int* height,
    const int max_width, 
    const int max_height
) {
    if (!pixels || !width || !height || *width <= 0 || *height <= 0) {
        return NULL;
    }

    stbir_pixel_layout layout = (channels == 4) ? STBIR_RGBA : STBIR_RGB;

    int w = *width;
    int h = *height;
    uint8_t* current = pixels;
    bool owns_current = false; // becomes true once we allocate our own buffer

    // Step 1: constrain by width (height follows proportionally)
    if (max_width > 0 && w > max_width) {
        int new_w = max_width;
        int new_h = (int)((double)h * ((double)new_w / (double)w));
        if (new_h < 1) new_h = 1;

        uint8_t* out = stbir_resize_uint8_linear(current, w, h, 0, NULL, new_w, new_h, 0, layout);
        if (out) {
            if (owns_current) free(current);
            current = out;
            owns_current = true;
            w = new_w;
            h = new_h;
        }
    }

    // Step 2: constrain by height (width follows proportionally)
    if (max_height > 0 && h > max_height) {
        int new_h = max_height;
        int new_w = (int)((double)w * ((double)new_h / (double)h));
        if (new_w < 1) new_w = 1;

        uint8_t* out = stbir_resize_uint8_linear(current, w, h, 0, NULL, new_w, new_h, 0, layout);
        if (out) {
            if (owns_current) free(current);
            current = out;
            owns_current = true;
            w = new_w;
            h = new_h;
        }
    }

    if (!owns_current) {
        return NULL;
    }

    *width = w;
    *height = h;
    return current;
}