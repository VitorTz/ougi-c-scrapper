#ifndef REQUESTS_H
#define REQUESTS_H
#include "structure/cstring.h"
#include <stdbool.h>


CString fetch_html(
    const char* url,
    const char* referer,
    bool add_delay
);


#endif