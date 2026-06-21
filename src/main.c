#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include "../include/pool/texture_pool.h"
#include "../include/constants.h"
#include "../include/globals.h"


const char* urls[] = {
    "https://cdn.mangadistrict.com/assets/publication/media/image/000001.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/01.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/02.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/03.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/04.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/05.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/06.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/07.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/08.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/09.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/10.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/11.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/12.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/13.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/14.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/15.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/16.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/17.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/18.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/19.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/20.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/21.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/22.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/23.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/24.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/25.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/26.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/27.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/28.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/29.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/30.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/31.jpg",
    "https://cdn.mangadistrict.com/publication/manga_6a375de1aa691/chapter-2/32.jpg",
    NULL
};


int main(void) { 
    srand((unsigned int)time(NULL));
    
    // Configure window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    const int current_monitor = GetCurrentMonitor();
    const int center_x = (GetMonitorWidth(current_monitor) - SCREEN_WIDTH) / 2;
    const int center_y = (GetMonitorHeight(current_monitor) - SCREEN_HEIGHT) / 2;
    SetWindowPosition(center_x, center_y);
    SetTargetFPS(WINDOW_FPS);    

    // Resource management init
    globals_init();
    texture_pool_init();    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WINDOW_BG_COLOR);
        EndDrawing();
    }
    
    // Resource management close
    texture_pool_close();
    globals_close();
    CloseWindow();

    return 0;
}

