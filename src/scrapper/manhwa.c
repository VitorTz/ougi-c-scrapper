#include "../../include/scrapper/manhwa.h"


void manhwa_scrap_free(ManhwaScrap* m) {
    string_free(&m->title);
    string_free(&m->slug);
    string_free(&m->descr);
    string_free(&m->cover);
    string_free(&m->id);
    string_free(&m->hex_color);
    string_free(&m->url);
    string_free(&m->status);

    string_free(&m->slug);
    string_vec_free(m->alternative_names);
    string_vec_free(m->authors);
    string_vec_free(m->artists);
    string_vec_free(m->genres);
    string_vec_free(m->tags);
    string_vec_free(m->scans);

    path_destroy(&m->path);
    path_destroy(&m->img_path);
}

void manhwa_print(const ManhwaScrap* manhwa) {
    if (!manhwa) {
        printf("[Error] ManhwaScrap pointer is NULL\n");
        return;
    }

    printf("============================================================\n");
    printf(" Manhwa Scrap Details\n");
    printf("============================================================\n");

    /* Core Information */
    printf(" ID               : %s\n", manhwa->id.data ? manhwa->id.data : "(empty)");
    printf(" Title            : %s\n", manhwa->title.data ? manhwa->title.data : "(empty)");
    printf(" Slug             : %s\n", manhwa->slug.data ? manhwa->slug.data : "(empty)");
    printf(" URL              : %s\n", manhwa->url.data ? manhwa->url.data : "(empty)");
    printf(" Status           : %s\n", manhwa->status.data ? manhwa->status.data : "(empty)");
    printf(" Release Year     : %d\n", manhwa->release);
    printf(" Source ID        : %d\n", (int)manhwa->source);
    
    /* Visuals and Paths */
    printf(" Hex Color        : %s\n", manhwa->hex_color.data ? manhwa->hex_color.data : "(empty)");
    printf(" Cover URL        : %s\n", manhwa->cover.data ? manhwa->cover.data : "(empty)");
    printf(" Local Path       : %s\n", manhwa->path.data ? manhwa->path.data : "(empty)");
    printf(" Local Image Path : %s\n", manhwa->img_path.data ? manhwa->img_path.data : "(empty)");
    
    /* Execution Flags */
    printf(" Should Ignore    : %s\n", manhwa->should_ignore ? "true" : "false");
    printf(" Should Overwrite : %s\n", manhwa->should_overwrite ? "true" : "false");

    /* Helper macro to safely print dynamic string_t arrays (vectors) */
    #define PRINT_STRING_VECTOR(label, vec) \
        do { \
            printf(" %-16s : ", label); \
            size_t _len = (vec) ? vec_len(vec) : 0; \
            if (_len == 0) { \
                printf("(empty)\n"); \
            } else { \
                for (size_t i = 0; i < _len; i++) { \
                    printf("[%s] ", (vec)[i].data ? (vec)[i].data : ""); \
                } \
                printf("\n"); \
            } \
        } while (0)

    /* Metadata Vectors */
    PRINT_STRING_VECTOR("Alt Names", manhwa->alternative_names);
    PRINT_STRING_VECTOR("Authors", manhwa->authors);
    PRINT_STRING_VECTOR("Artists", manhwa->artists);
    PRINT_STRING_VECTOR("Genres", manhwa->genres);
    PRINT_STRING_VECTOR("Tags", manhwa->tags);
    PRINT_STRING_VECTOR("Scans", manhwa->scans);
    
    #undef PRINT_STRING_VECTOR

    /* Description (printed last to preserve readability, as it might be multi-line) */
    printf("------------------------------------------------------------\n");
    printf(" Description:\n%s\n", (manhwa->descr.data && manhwa->descr.length > 0) ? manhwa->descr.data : "(no description)");
    printf("============================================================\n");
}