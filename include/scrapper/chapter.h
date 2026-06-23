#ifndef CHAPTER_H
#define CHAPTER_H
#include "../structure/path.h"
#include "../structure/string_t.h"
#include "manhwa.h"


typedef struct  {
    path_t path;
    string_t url;
    ManhwaScrap* manhwa;
    float num;
} ChapterScrap;


typedef struct {
    const ChapterScrap* chapter;
    size_t index;
    string_t url;
} ChapterImage;


#endif