/* Define POSIX C Source to expose POSIX-specific functions (nanosleep, popen, pclose) */
#define _POSIX_C_SOURCE 200809L

#include "../include/structure/cstring.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Halts the execution for a random period between 600ms and 1800ms */
static void delay() {
    int delay_ms = (rand() % (1800 - 600 + 1)) + 600;
    struct timespec ts;
    ts.tv_sec = delay_ms / 1000;
    ts.tv_nsec = (delay_ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/* Formats and prints a log message with the current time */
static void print_log(const char* message, const char* url) {
    time_t now;
    struct tm* local_time;
    char time_str[10];
    
    time(&now);
    local_time = localtime(&now);
    
    if (local_time != NULL) {
        strftime(time_str, sizeof(time_str), "%H:%M:%S", local_time);
    } else {
        snprintf(time_str, sizeof(time_str), "00:00:00");
    }

    printf("[%s] %s: %s\n", time_str, message, url);
}

/* Fetches HTML content from a given URL using a custom curl command */
CString fetch_html(const char* url, const char* referer, const int add_delay) {
    CString result = cstring_create(NULL);

    print_log("Fetching...", url);

    if (add_delay) { delay(); }

    char cmd_buffer[4096];

    /* Format the entire command in a single pass based on the referer's presence */
    if (referer != NULL) {
        snprintf(cmd_buffer, sizeof(cmd_buffer),
            "curl_chrome116 --silent --compressed -A '' "
            "-H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8' "
            "-H 'Accept-Language: pt-BR,pt;q=0.9,en-US;q=0.8' "
            "-H 'Sec-Fetch-Dest: document' "
            "-H 'Sec-Fetch-Mode: navigate' "
            "-H 'Sec-Fetch-Site: same-origin' "
            "-H 'Sec-Fetch-User: ?1' "
            "--referer '%s' "
            "-b cookies.txt -c cookies.txt --location --max-redirs 10 '%s'",
            referer, url);
    } else {
        snprintf(cmd_buffer, sizeof(cmd_buffer),
            "curl_chrome116 --silent --compressed -A '' "
            "-H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8' "
            "-H 'Accept-Language: pt-BR,pt;q=0.9,en-US;q=0.8' "
            "-H 'Sec-Fetch-Dest: document' "
            "-H 'Sec-Fetch-Mode: navigate' "
            "-H 'Sec-Fetch-Site: none' "
            "-H 'Sec-Fetch-User: ?1' "
            "-b cookies.txt -c cookies.txt --location --max-redirs 10 '%s'",
            url);
    }

    /* Pass the stack buffer directly to popen */
    FILE* pipe = popen(cmd_buffer, "r");
    
    if (pipe == NULL) {
        printf("[Error] Failed to open pipe for url: %s\n", url);
        return result;
    }

    cstring_reserve(&result, 4096);    
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        cstring_append(&result, buffer);
    }

    pclose(pipe);

    print_log("Finished fetching", url);

    if (cstring_is_empty(&result)) {
        printf("Fetched HTML is empty for: %s\n", url);
    }

    return result;
}