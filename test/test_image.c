#include "../include/structure/path.h"
#include "../include/image.h"



int main() {
    path_t path = path_create("res/img-test/misc");
    path_t* entries = path_dir_iterator(&path, NULL, NULL);
    vec_foreach(path_t, item, entries) {
        PixelBuffer buf = load_img_from_disk(item);
        path_print(item);
        print_pixel_buffer(&buf);
        break;
    }
    path_dir_iterator_free(entries);
    return 0;
}