#ifndef COLORFLY_H
#define COLORFLY_H
#include "structure/string_t.h"
#include "structure/path.h"

/**
 * extract_dominant_color
 *
 * Loads `image_path` (PNG, JPEG, or WebP) and returns the RGB color that
 * best represents the album art in the same way Spotify does.
 *
 * @param image_path  Path to the local image file.
 * @param k           Number of K-means clusters (default: 8).
 * @param thumb_size  Downscale target in pixels (default: 64×64).
 * @returns           string_t with the dominant color.
 */
string_t extract_dominant_color(
    const path_t* path,
    int k,
    int thumb_size
);


#endif