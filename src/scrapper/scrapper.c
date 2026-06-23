#include "../../include/scrapper/scrapper.h"
#include "../../include/image.h"


Scrapper scrapper_create(
    ManhwaScrap* manhwa,
    const SourceID source_id,
    ScrapChapterFunc scrap_chapter,
    ScrapImagesFunc scrap_images
) {
    Scrapper sc = {0};
    sc.source = source_id;
    sc.manhwa = manhwa;
    sc.chapters = NULL;
    sc.images = NULL;
    sc.scrap_chapter = scrap_chapter;
    sc.scrap_images = scrap_images;
    vec_reserve(sc.chapters, 64);
    vec_reserve(sc.images, 256);
    return sc;
}


static Scrapper get_scrapper(ManhwaScrap* manhwa) {
    switch (manhwa->source) {
        case MANGA_DISTRICT:
            return scrapper_manga_district(manhwa);
            break;
        case TOONDEX:
            return scrapper_toondex(manhwa);
            break;
        case OMEGA_SCANS:
            return scrapper_omega_scans(manhwa);
            break;
    }
}


static void extract_urls(ChapterImage* images, string_t* urls) {
    vec_foreach(ChapterImage, image, images) {
        vec_push_back(urls, image->url);
    }
}


void scrapper_process_manhwa(ManhwaScrap* manhwa) {
    Scrapper scrapper = get_scrapper(manhwa);
    if (!scrapper.scrap_chapter || !scrapper.scrap_images) {
        return;
    }

    scrapper.scrap_chapter(&scrapper);
    string_t* urls = NULL;
    vec_foreach(ChapterScrap, chapter, scrapper.chapters) {
        scrapper.scrap_images(&scrapper, chapter);
        extract_urls(scrapper.images, urls);
        download_imgs(
            chapter->path,
            urls, 
            string_cstr(&chapter->url)
        );
    }
    vec_free(urls);
    vec_free(scrapper.chapters);
    vec_free(scrapper.images);
}
