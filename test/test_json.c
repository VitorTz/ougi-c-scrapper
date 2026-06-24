#include "../include/scrapper/manhwa.h"
#include "../include/pool/manhwa_pool.h"
#include "../include/structure/map.h"



int main() {
    manhwa_pool_init();
    ManhwaMapItem* map = manhwa_pool_map();

    hashmap_foreach(ManhwaScrap, manhwa, map) {
        manhwa_print(manhwa);
    }

    manhwa_pool_close();

    return 0;
}