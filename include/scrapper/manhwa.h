#ifndef MANHWA_H
#define MANHWA_H

#include "../structure/string_t.h"
#include "../structure/path.h"
#include "../cjson/cJSON.h"
#include "source.h"


typedef struct  {
    string_t title;
    string_t slug;
    string_t descr;
    string_t cover;
    path_t path;
    path_t img_path;
    string_t id;
    string_t hex_color;
    string_t url;
    string_t status;
    string_t* alternative_names;
    string_t* authors;
    string_t* artists;
    string_t* genres;
    string_t* tags;
    string_t* scans;
    SourceID source;
    int release;
    bool should_ignore;
    bool should_overwrite;
    string_t local_cover;
} ManhwaScrap;


#endif