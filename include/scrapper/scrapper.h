#ifndef SCRAPPER_H
#define SCRAPPER_H
#include "../../include/structure/vector.h"
#include "../structure/string_t.h"
#include "../structure/vector.h"
#include "../structure/path.h"
#include "../parser.h"
#include "source.h"
#include "manhwa.h"
#include "chapter.h"


typedef struct Scrapper Scrapper;
typedef void (*ScrapChapterFunc)(Scrapper* self);
typedef void (*ScrapImagesFunc)(Scrapper* self, const ChapterScrap* chapter);


struct Scrapper {
    SourceID source;
    ManhwaScrap* manhwa;
    ChapterScrap* chapters;
    ChapterImage* images;
    ScrapChapterFunc scrap_chapter;
    ScrapImagesFunc scrap_images;
};


Scrapper scrapper_create(
    ManhwaScrap* manhwa, 
    SourceID source_id,
    ScrapChapterFunc scrap_chapter,
    ScrapImagesFunc scrap_images
);

Scrapper scrapper_manga_district(ManhwaScrap* manhwa);

Scrapper scrapper_toondex(ManhwaScrap* manhwa);

Scrapper scrapper_omega_scans(ManhwaScrap* manhwa);

void scrapper_process_manhwa(ManhwaScrap* manhwa);


#endif