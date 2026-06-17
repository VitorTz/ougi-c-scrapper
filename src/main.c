#include "../include/constants.h"
#include "../include/pool/texture_pool.h"
#include <raylib.h>

static const char* IMG = "/mnt/HD/Pictures/YuGiOh/MasterDuel/guardian_chimera_render_by_henukim_dfanj59.png";


int main(void) {
    
    // Configure window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    const int current_monitor = GetCurrentMonitor();
    const int center_x = (GetMonitorWidth(current_monitor) - SCREEN_WIDTH) / 2;
    const int center_y = (GetMonitorHeight(current_monitor) - SCREEN_HEIGHT) / 2;
    SetWindowPosition(center_x, center_y);
    SetTargetFPS(WINDOW_FPS);
    
    texture_pool_init();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WINDOW_BG_COLOR);
        Texture2D texture = texture_pool_load(IMG, "21");
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }
    
    texture_pool_close();
    CloseWindow();

    return 0;
}