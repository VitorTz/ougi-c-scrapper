#ifndef REQUESTS_H
#define REQUESTS_H

#include "structure/string_t.h"
#include <stdbool.h>


string_t fetch_html(
    const char* url,
    const char* referer,
    bool add_delay
);


#endif