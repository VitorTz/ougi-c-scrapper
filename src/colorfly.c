#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../include/colorfly.h"
#include "../include/image.h"
#include "../include/structure/vector.h"
#include "../include/structure/string_t.h"
#include "../include/util.h"


typedef struct HSV {
  float h; // [0, 360)
  float s; // [0, 1]
  float v; // [0, 1]
} HSV;

typedef struct XYZ {
  float x;
  float y;
  float z;
} XYZ;

typedef struct LAB {
  float l;
  float a;
  float b;
} LAB;

typedef struct {
    LAB centroid;
    RGB rgb_centroid;
    int count;
    float avg_s; // average HSV saturation of members
    float avg_v; // average HSV value of members
} Cluster;


static string_t rgb_to_hex(const RGB rgb) {
    string_t str = string_new();
    string_append_char(&str, '#');
    char hex_buf[7];
    snprintf(hex_buf, sizeof(hex_buf), "%02X%02X%02X", rgb.r, rgb.g, rgb.b);
    string_append(&str, hex_buf);
    return str;
}


static Color hex_to_color(const string_t* str) {
    if (string_length(str)) {
        return WHITE;
    }

    const char* buffer = str->data + 1;
    const unsigned long hex_val = strtoul(buffer, NULL, 16);

    return (Color){
        .r = (unsigned char)((hex_val >> 16) & 0xFF),
        .g = (unsigned char)((hex_val >> 8) & 0xFF),
        .b = (unsigned char)(hex_val & 0xFF),
        .a = 255
    };
}


static HSV rgb_to_hsv(const float r, const float g, const float b) {
    const float cmax = MAX(r, MAX(g, b));
    const float cmin = MIN(r, MIN(g, b));
    const float delta = cmax - cmin;

    HSV hsv = {0};
    hsv.v = cmax;
    hsv.s = (cmax > 0.0f) ? (delta / cmax) : 0.0f;

    if (delta < 1e-6f) {
        hsv.h = 0.0f;
    } else if (cmax == r) {
        hsv.h = 60.0f * fmod((g - b) / delta, 6.0f);
    } else if (cmax == g) {
        hsv.h = 60.0f * ((b - r) / delta + 2.0f);
    } else {
        hsv.h = 60.0f * ((r - g) / delta + 4.0f);
    }

    if (hsv.h < 0.0f) {
        hsv.h += 360.0f;
    }

    return hsv;
}

// sRGB → linear RGB (gamma expansion)
static float srgb_to_linear(const float c) {
    return (c <= 0.04045f) ? (c / 12.92f) : pow((c + 0.055f) / 1.055f, 2.4f);
}

// linear RGB → XYZ (D65)
static XYZ rgb_to_xyz(const float r, const float g, const float b) {
    const float rl = srgb_to_linear(r);
    const float gl = srgb_to_linear(g);
    const float bl = srgb_to_linear(b);
    return (XYZ){
        .x = rl * 0.4124564f + gl * 0.3575761f + bl * 0.1804375f,
        .y = rl * 0.2126729f + gl * 0.7151522f + bl * 0.0721750f,
        .z = rl * 0.0193339f + gl * 0.1191920f + bl * 0.9503041f
    };
}

static float xyz_f(const float t) {
    static const float delta = 6.0f / 29.0f;
    return (t > delta * delta * delta) ? cbrt(t) : (t / (3.0f * delta * delta) + 4.0f / 29.0f);
}

// XYZ → CIELAB (D65 whitepoint)
static LAB xyz_to_lab(const float x, const float y, const float z) {
    // D65 whitepoint
    const float fx = xyz_f(x / 0.95047f);
    const float fy = xyz_f(y / 1.00000f);
    const float fz = xyz_f(z / 1.08883f);
    return (LAB){
        .l = 116.0f * fy - 16.0f, 
        .a = 500.0f * (fx - fy), 
        .b = 200.0f * (fy - fz)
    };
}

static LAB rgb_to_lab(const float r, const float g, const float b) {
    const XYZ xyz = rgb_to_xyz(r, g, b);
    return xyz_to_lab(xyz.x, xyz.y, xyz.z);
}

// ─── Downsampling (nearest-neighbor) ────────────────────────────────

static RGB* downsample(
    const uint8_t* data,
    const int w,
    const int h,
    const int channels,
    const int target
) {
    RGB* out = NULL;
    vec_reserve(out, target * target);

    for (int py = 0; py < target; ++py) {
        int sy = (py * h) / target;
        for (int px = 0; px < target; ++px) {
            int sx = (px * w) / target;
            int idx = (sy * w + sx) * channels;
            const RGB rgb = (RGB){data[idx], data[idx + 1], data[idx + 2]};
            vec_push_back(out, rgb);
        }
    }

    return out;
}

// ─── Saliency filter ─────────────────────────────────────────────────────────
//
// Spotify heavily prefers saturated, mid-brightness colors.
// These thresholds mirror what the Spotify backend is observed to produce.

static bool is_salient(const HSV* hsv) {
    return hsv->s > 0.15f  // not too gray
        && hsv->v > 0.10f  // not near-black
        && hsv->v < 0.96f; // not near-white
}


static float lab_dist2(const LAB* a, const LAB* b) {
    const float dl = a->l - b->l;
    const float da = a->a - b->a;
    const float db = a->b - b->b;
    return dl * dl + da * da + db * db;
}


/* simple xorshift random number generator to replace std::mt19937.
 * ensures 32-bit randomness regardless of platform RAND_MAX limits. */
static uint32_t xorshift32(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}


static Cluster* kmeans(
    const LAB* pixels,
    const HSV* hsvs,
    const RGB* rgbs, 
    int k,
    const int max_iter
) {
    /* initialize rng state matching mt19937 rng(42) */
    uint32_t rng_state = 42; 
    const size_t n_pixels = vec_len(pixels);
    
    Cluster* clusters = NULL;
    vec_fill(clusters, k, ((Cluster){0}));

    /* k-means++ initialization */
    {
        int* chosen = NULL;
        uint32_t first_idx = xorshift32(&rng_state) % n_pixels;
        vec_push_back(chosen, (int)first_idx);
        clusters[0].centroid = pixels[chosen[0]];

        for (int ci = 1; ci < k; ++ci) {
            float* weights = NULL;
            /* fix: weights needs length, not just capacity, to be iterated/accessed */
            vec_resize(weights, n_pixels);
            
            float total = 0.0f;
            for (size_t i = 0; i < n_pixels; ++i) {
                float min_d = FLT_MAX; /* std::numeric_limits<float>::max() */
                vec_foreach(int, item, chosen) {
                    min_d = MIN(min_d, lab_dist2(&pixels[i], &pixels[*item]));
                }
                weights[i] = min_d;
                total += min_d;
            }
            
            /* uniform_real_distribution mapping */
            float r = ((float)xorshift32(&rng_state) / (float)0xFFFFFFFF) * total;
            float cum = 0.0f;
            int sel = 0;
            
            for (size_t i = 0; i < n_pixels; ++i) {
                cum += weights[i];
                if (cum >= r) {
                    sel = (int)(i);
                    break;
                }
            }
            vec_push_back(chosen, sel);
            clusters[ci].centroid = pixels[vec_back(chosen)];
            
            /* fix: prevent memory leak every iteration */
            vec_free(weights); 
        }
        /* fix: clean up chosen array after initialization */
        vec_free(chosen); 
    }

    int* assignments = NULL;
    vec_fill(assignments, n_pixels, 0);

    /* fix: moved allocations OUTSIDE the max_iter loop.
     * in the original code, they were allocated inside the loop but 
     * vec_free was called outside, causing scope errors and memory leaks. */
    LAB* sums = NULL;
    float* rgb_sums_r = NULL;
    float* rgb_sums_g = NULL;
    float* rgb_sums_b = NULL;
    float* s_sums = NULL;
    float* v_sums = NULL;
    int* counts = NULL;

    for (int iter = 0; iter < max_iter; ++iter) {
        
        /* assignment step */
        for (size_t i = 0; i < n_pixels; ++i) {
            float best = FLT_MAX;
            int best_c = 0;
            for (int c = 0; c < k; ++c) {
                float d = lab_dist2(&pixels[i], &clusters[c].centroid);
                if (d < best) {
                    best = d;
                    best_c = c;
                }
            }
            assignments[i] = best_c;
        }

        /* update step: reset values to zero for the current iteration */
        vec_fill(sums, k, ((LAB){0, 0, 0}));
        vec_fill(rgb_sums_r, k, 0.0f);
        vec_fill(rgb_sums_g, k, 0.0f);
        vec_fill(rgb_sums_b, k, 0.0f);
        vec_fill(s_sums, k, 0.0f);
        vec_fill(v_sums, k, 0.0f);
        vec_fill(counts, k, 0);

        for (size_t i = 0; i < n_pixels; ++i) {
            int c = assignments[i];
            sums[c].l += pixels[i].l;
            sums[c].a += pixels[i].a;
            sums[c].b += pixels[i].b;
            rgb_sums_r[c] += rgbs[i].r;
            rgb_sums_g[c] += rgbs[i].g;
            rgb_sums_b[c] += rgbs[i].b;
            s_sums[c] += hsvs[i].s;
            v_sums[c] += hsvs[i].v;
            ++counts[c];
        }

        bool changed = false;
        for (int c = 0; c < k; ++c) {
            if (counts[c] == 0) continue;
            
            float inv = 1.0f / counts[c];
            LAB new_centroid = (LAB){sums[c].l * inv, sums[c].a * inv, sums[c].b * inv};
            
            if (lab_dist2(&clusters[c].centroid, &new_centroid) > 0.01f) {
                changed = true;
            }
            
            clusters[c].centroid = new_centroid;
            clusters[c].rgb_centroid = (RGB){
                (uint8_t)(rgb_sums_r[c] * inv),
                (uint8_t)(rgb_sums_g[c] * inv),
                (uint8_t)(rgb_sums_b[c] * inv),
            };
            clusters[c].count = counts[c];
            clusters[c].avg_v = v_sums[c] * inv;
            clusters[c].avg_s = s_sums[c] * inv;
        }

        if (!changed) {
            break;
        }
    }

    /* free all allocated resources */
    vec_free(assignments);
    vec_free(sums);
    vec_free(rgb_sums_r);
    vec_free(rgb_sums_g);
    vec_free(rgb_sums_b);
    vec_free(s_sums);
    vec_free(v_sums);
    vec_free(counts);

    return clusters;
}


// ─── Value weight (Spotify prefers mid-range brightness) ─────────────────────
//
// A bell curve centered at ~0.55 – bright enough to show on dark backgrounds,
// not so bright it washes out to white.
static float value_weight(float v) {
    const float center = 0.55f;
    const float sigma = 0.30f;
    const float d = (v - center) / sigma;
    return exp(-0.5f * d * d);
}


string_t extract_dominant_color(
    const path_t* path, 
    int k,
    const int thumb_size
) {
    string_t str = string_new();
    if (!path_exists(path)) {
        return str;
    }
    // 1. Load image
    PixelBuffer buf = load_img_from_disk(path);
    const int channels = 3;
    
    // 2. Downsample
    RGB* pixels = downsample(buf.data, buf.w, buf.h, channels, thumb_size);
    const size_t n_pixels = vec_len(pixels);

    // 3. Convert to HSV and filter for saliency
    RGB* filtered_rgb = NULL;
    HSV* filtered_hsv = NULL;
    LAB* filtered_lab = NULL;

    vec_reserve(filtered_rgb, n_pixels);
    vec_reserve(filtered_hsv, n_pixels);
    vec_reserve(filtered_lab, n_pixels);

    vec_foreach(RGB, px, pixels) {
        const float r = px->r / 255.0f;
        const float g = px->g / 255.0f;
        const float b = px->b / 255.0f;

        HSV hsv = rgb_to_hsv(r, g, b);
        if (!is_salient(&hsv)) {
            continue;
        }

        vec_push_back(filtered_rgb, *px);
        vec_push_back(filtered_hsv, hsv);
        vec_push_back(filtered_lab, rgb_to_lab(r, g, b));
    }

    // Fallback: if too few salient pixels (e.g. black-and-white cover),
    // relax the filter and use the full pixel set.
    if (vec_len(filtered_rgb) < (size_t) (k) * 2) {
        vec_clear(filtered_rgb);
        vec_clear(filtered_hsv);
        vec_clear(filtered_lab);
        vec_foreach(RGB, px, pixels) {
            const float r = px->r / 255.0f;
            const float g = px->g / 255.0f;
            const float b = px->b / 255.0f;
            vec_push_back(filtered_rgb, *px);
            vec_push_back(filtered_hsv, rgb_to_hsv(r, g, b));
            vec_push_back(filtered_lab, rgb_to_lab(r, g, b));
        }
    }

    // 4. K-means clustering in CIELAB space
    k = MIN(k, (int) (vec_len(filtered_rgb)));
    Cluster* clusters = kmeans(filtered_lab, filtered_hsv, filtered_rgb, k, 20);

    // 5. Score and select best cluster
    //    score = population_fraction × avg_saturation × value_weight(avg_value)
    const int total = (int) (vec_len(filtered_rgb));
    int best_idx = 0;
    float best_score = -1.0f;

    for (int i = 0; i < k; ++i) {
        if (clusters[i].count == 0) {
            continue;
        }
        const float pop_fraction = (float) (clusters[i].count) / total;
        const float score = pop_fraction * clusters[i].avg_s * value_weight(clusters[i].avg_v);
        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }

    return rgb_to_hex(clusters[best_idx].rgb_centroid);
}