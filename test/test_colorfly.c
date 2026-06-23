#include "../include/structure/vector.h"
#include "../include/structure/path.h"
#include "../include/pool/texture_pool.h"
#include "../include/colorfly.h"
#include <raylib.h>



typedef struct ImagePair {
    Texture2D texture;
    string_t hex_color;
} ImagePair;


int main() {
    path_t root = path_create("res/img-test/colorfly");

    InitWindow(1280, 720, "Colorfly");
    SetTargetFPS(30);
    texture_pool_init();

    ImagePair* images = NULL;

    path_t* entries = path_dir_iterator(&root, NULL, NULL);
    vec_foreach(path_t, path, entries) {
        const Texture2D t = texture_pool_load(path_c_str(path));
        const string_t hex_color = extract_dominant_color(path, 8, 64);
        const ImagePair pair = (ImagePair){t, hex_color};
        vec_push_back(images, pair);
    }
    path_dir_iterator_free(entries);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    texture_pool_close();
    vec_free(images);

    CloseWindow();

}