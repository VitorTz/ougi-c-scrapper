#include <raylib.h>
#include "../include/pool/texture_pool.h"


int main() {
    InitWindow(1280, 720, "Raylib");

    texture_pool_init();

    Texture2D t = texture_pool_load("res/img-test/colorfly/0dae9f62-2d7e-4a7a-b76e-8f94b1c15e1c.webp");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(t, 0, 0, WHITE);
        EndDrawing();
    }

    texture_pool_close();

    CloseWindow();
}