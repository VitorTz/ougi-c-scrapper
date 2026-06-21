#include "../include/image.h"
#include "../include/globals.h"
#include <stdio.h>

char buffer[32];

const char* referer = "https://mangadistrict.com/series/secret-class/chapter-92/";
const char* urls[] = {
    "https://cdn.mangadistrict.com/assets/publication/media/image/000001.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/01.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/02.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/03.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/04.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/05.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/06.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/07.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/08.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/09.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/10.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/11.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/12.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/13.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/14.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/15.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6017ef9d16a1d/chapter-92/16.jpg",
    NULL
};


int main() {
    globals_init();

    const path_t root = path_create("res/img-test/manhwa");

    string_t* vec = NULL;
    
    for (size_t i = 0; urls[i] != NULL; i++) {
        vec_push_back(vec, string_from(urls[i]));
    }

    download_imgs(root, vec, referer);

    globals_close();

}