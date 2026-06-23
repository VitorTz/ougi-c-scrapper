#include "../../include/scrapper/scrapper.h"
#include "../../include/request.h"
#include "../../include/parser.h"
#include "../../include/type_to_str.h"


static void scrap_chapters(Scrapper* self) {
    string_t html = fetch_html(
        string_cstr(&self->manhwa->url),
        NULL, 
        true
    );
    og_node_t* root = og_parse_html(string_cstr(&html));

    string_t* links = extract_links(root, "li", NULL, "wp-manga-chapter");
    vec_reverse(links);
    
    float i = 0.0f;
    vec_clear(self->chapters);
    vec_foreach(string_t, item, links) {
        float num = i++;
        path_t chap_path = path_create_copy(&self->manhwa->folderPath);
        path_append(&chap_path, float_to_string(i));
        path_create_directories(&chap_path);
        const ChapterScrap ch = {
            .manhwa = self->manhwa,
            .url = string_clone(item),
            .path = chap_path,
            .num = num
        };
        vec_push_back(self->chapters, ch);
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
