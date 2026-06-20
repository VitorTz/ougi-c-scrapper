#include "../include/pool/texture_pool.h"
#include "../include/constants.h"
#include "../include/structure/path.h"
#include <raylib.h>
#include <string.h>
#include <webp/types.h>


int main(void) {    
    // Configure window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    const int current_monitor = GetCurrentMonitor();
    const int center_x = (GetMonitorWidth(current_monitor) - SCREEN_WIDTH) / 2;
    const int center_y = (GetMonitorHeight(current_monitor) - SCREEN_HEIGHT) / 2;
    SetWindowPosition(center_x, center_y);
    SetTargetFPS(WINDOW_FPS);
    
    Path path = path_init("/mnt/HD/Documents/Manhwa/Secret_Class/Chapter_1");
    Vector entries = path_dir_iterator(&path);
    
    Path* it = NULL;
    VECTOR_FOREACH(Path, it, &entries) {
        path_print(it);
    }

    path_dir_iterator_free(&entries);

    // Resource management
    texture_pool_init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WINDOW_BG_COLOR);
        EndDrawing();
    }
    
    texture_pool_close();
    CloseWindow();
    return 0;
}
