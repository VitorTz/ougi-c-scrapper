#include "../../include/scrapper/scrapper.h"
#include "../../include/request.h"
#include "../../include/parser.h"
#include "../../include/convert.h"



static ChapterScrap create_chapter_scrap(
    ManhwaScrap* manhwa,
    const string_t* link, 
    const float chapter_num
) {
    path_t chap_path = path_create_copy(&manhwa->path);
    path_append(&chap_path, float_to_string(chapter_num));
    path_create_directories(&chap_path);
    const ChapterScrap ch = {
        .manhwa = manhwa,
        .url = string_clone(link),
        .path = chap_path,
        .num = chapter_num
    };
    return ch;
}


static void scrap_chapters(Scrapper* self) {
    string_t html = fetch_html(
        string_cstr(&self->manhwa->url),
        NULL, 
        true
    );
    og_node_t* root = og_parse_html(string_cstr(&html));

    string_t* links = extract_links(root, "li", NULL, "wp-manga-chapter");
    vec_reverse(links);
    
    float chapter_num = 0.0f;
    vec_clear(self->chapters);
    vec_foreach(string_t, item, links) {
        vec_push_back(self->chapters, create_chapter_scrap(
            self->manhwa, 
            item, 
            chapter_num++
        ));
    }
}


static void scrap_images(Scrapper* self, const ChapterScrap* chapter) {
    string_t html = fetch_html(chapter->url.data, chapter->manhwa->url.data, true);
    og_node_t* root = og_parse_html(string_cstr(&html));
    string_t* images = extract_imgs(root, "img", NULL, "wp-manga-chapter-img");
    
    size_t i = 0;
    vec_clear(self->images);
    vec_foreach(string_t, item, images) {
        ChapterImage image = {
            .chapter = chapter,
            .index = i++,
            .url = *item
        };
        vec_push_back(self->images, image);
    }          
}


Scrapper scrapper_manga_district(ManhwaScrap* manhwa) {
    return scrapper_create(
        manhwa, 
        MANGA_DISTRICT, 
        scrap_chapters, 
        scrap_images
    );
}
