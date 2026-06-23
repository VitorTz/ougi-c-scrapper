#include "../../include/scrapper/scrapper.h"


static void scrap_chapters(Scrapper* self) {

}


static void scrap_images(Scrapper* self, const ChapterScrap* chapter) {
        
}


Scrapper scrapper_toondex(ManhwaScrap* manhwa) {
    return scrapper_create(
        manhwa, 
        MANGA_DISTRICT, 
        scrap_chapters, 
        scrap_images
    );
}