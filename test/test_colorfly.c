#include "../include/structure/vector.h"
#include "../include/structure/path.h"
#include "../include/pool/texture_pool.h"
#include "../include/colorfly.h"
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct ImagePair {
    Texture2D texture;
    string_t hex_color;
} ImagePair;

// Helper structure to store precalculated grid layout positions and dimensions
typedef struct RenderInfo {
    Vector2 position;
    Vector2 imageSize;
    float cardHeight;
} RenderInfo;

// Utility function to parse a Hexadecimal color string into a Raylib Color object
Color HexToColor(const char* hex) {
    if (!hex) return WHITE;
    
    // Skip the '#' character if it is present at the beginning
    if (hex[0] == '#') {
        hex++;
    }
    
    unsigned int r = 0, g = 0, b = 0;
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        return (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
    }
    
    return WHITE; // Return white as a fallback default
}

int main() {
    path_t root = path_create("res/img-test/colorfly");

    InitWindow(1280, 720, "Colorfly");
    SetTargetFPS(30);
    texture_pool_init();

    ImagePair* images = NULL;
    int image_count = 0;

    path_t* entries = path_dir_iterator(&root, NULL, NULL);
    vec_foreach(path_t, path, entries) {
        // Load the texture from the path as before
        Texture2D t = texture_pool_load(path_c_str(path));

        // -- NEW CODE ADDED HERE FOR SMOOTHING AND QUALITY --
        
        // 1. Generate mipmaps for this texture. This creates pre-scaled versions
        //    of the image, which is crucial for reducing aliasing (jaggies) 
        //    when the texture is drawn smaller than its native resolution.
        GenTextureMipmaps(&t); 

        // 2. Set the texture filter to a high-quality combination of trilinear
        //    and anisotropic filtering. Anisotropic filtering (8x) greatly 
        //    improves sharpness and reduces shimmering on the downscaled 
        //    images, particularly on edges.
        SetTextureFilter(t, TEXTURE_FILTER_ANISOTROPIC_8X);

        // ----------------------------------------------------

        // Extract the dominant color
        const string_t hex_color = extract_dominant_color(path, 8, 64);
        
        // Pack into the pair
        const ImagePair pair = (ImagePair){t, hex_color};
        vec_push_back(images, pair);
        image_count++;
    }
    path_dir_iterator_free(entries);

    // Grid layout configuration constants
    const int columns = 3;
    const int paddingX = (1280 - (columns * 320)) / (columns + 1); // Centers cells perfectly: (1280 - 960) / 4 = 80px
    const int paddingY = 40;
    const float infoPanelHeight = 60.0f; // Reserved space at the bottom of each card for text and color preview

    // Allocate memory to precalculate layout properties for faster rendering
    RenderInfo* layouts = (RenderInfo*)malloc(image_count * sizeof(RenderInfo));
    float currentY = (float)paddingY;

    // Process the elements row by row to account for variable heights cleanly
    for (int i = 0; i < image_count; i += columns) {
        float maxRowHeight = 0.0f;
        
        // Pass 1: Determine the layout sizes and find the maximum card height in the current row
        for (int j = 0; j < columns && (i + j) < image_count; j++) {
            int idx = i + j;
            Texture2D t = images[idx].texture;
            
            // Limit width to 320px maximum and scale the height proportionally to maintain aspect ratio
            float scale = 320.0f / (float)t.width;
            float imgH = (float)t.height * scale;
            float cardH = imgH + infoPanelHeight;
            
            layouts[idx].imageSize = (Vector2){ 320.0f, imgH };
            layouts[idx].cardHeight = cardH;
            
            if (cardH > maxRowHeight) {
                maxRowHeight = cardH;
            }
        }
        
        // Pass 2: Map positions for the row items based on the calculated maximum heights
        for (int j = 0; j < columns && (i + j) < image_count; j++) {
            int idx = i + j;
            float posX = (float)(paddingX + j * (320 + paddingX));
            layouts[idx].position = (Vector2){ posX, currentY };
        }
        
        // Move vertical offset down by the row's maximum height plus padding
        currentY += maxRowHeight + paddingY;
    }
    
    // Total vertical space used by the grid content
    float totalContentHeight = currentY;
    float scrollOffset = 0.0f;
    const float scrollSpeed = 35.0f;

    while (!WindowShouldClose()) {
        // Capture mouse wheel inputs to alter scroll offset
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scrollOffset += wheel * scrollSpeed;
        }

        // Clamp the scrollOffset so the user cannot scroll infinitely out of bounds
        float maxScroll = totalContentHeight - 720.0f;
        if (maxScroll < 0.0f) maxScroll = 0.0f;
        
        if (scrollOffset > 0.0f) scrollOffset = 0.0f;
        if (scrollOffset < -maxScroll) scrollOffset = -maxScroll;

        BeginDrawing();
        ClearBackground(GetColor(0x0F0F0FFF)); // Dark background theme for high contrast visual presentation

        // Draw the image grid with frustum culling optimization
        for (int i = 0; i < image_count; i++) {
            float renderY = layouts[i].position.y + scrollOffset;
            
            // Frustum culling check: only render cards currently visible within the 720px height view
            if (renderY + layouts[i].cardHeight > 0 && renderY < 720) {
                // Background container for the image card
                DrawRectangle(layouts[i].position.x, renderY, 320, layouts[i].cardHeight, GetColor(0x1E1E1EFF));
                
                // Draw texture matching the fixed 320px width rule
                // Drawing will be much smoother now thanks to Mipmaps and Anisotropic filtering.
                DrawTexturePro(
                    images[i].texture,
                    (Rectangle){ 0, 0, (float)images[i].texture.width, (float)images[i].texture.height },
                    (Rectangle){ layouts[i].position.x, renderY, layouts[i].imageSize.x, layouts[i].imageSize.y },
                    (Vector2){ 0, 0 },
                    0.0f,
                    WHITE
                );
                
                // Set coordinate targets for color info inside the panel area
                float infoY = renderY + layouts[i].imageSize.y + 15;
                
                // Cast string_t wrapper to const char*. Adjust if string_t is a struct (e.g., images[i].hex_color.data)
                Color extractedColor = HexToColor((const char*) images[i].hex_color.data);
                
                // Draw the visual color solid box block representation
                DrawRectangle(layouts[i].position.x + 15, infoY, 40, 25, extractedColor);
                DrawRectangleLines(layouts[i].position.x + 15, infoY, 40, 25, GRAY);
                
                // Draw the alphanumeric string value representation of the color
                DrawText((const char*)images[i].hex_color.data, layouts[i].position.x + 70, infoY + 4, 18, LIGHTGRAY);
            }
        }

        EndDrawing();
    }

    // Free the layout buffer resources
    free(layouts);

    texture_pool_close();
    vec_free(images);

    CloseWindow();
    return 0;
}