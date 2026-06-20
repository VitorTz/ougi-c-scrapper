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
CString fetch_html(
    const char* url,
    const char* referer,
    const int add_delay
) {
    CString result = string_init(NULL);

    print_log("Fetching...", url);

    if (add_delay) { delay(); }

    CString cmd = string_init("curl_chrome116 --silent --compressed");
    string_append(&cmd, " -A ''");
    string_append(&cmd, " -H 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8'");
    string_append(&cmd, " -H 'Accept-Language: pt-BR,pt;q=0.9,en-US;q=0.8'");
    string_append(&cmd, " -H 'Sec-Fetch-Dest: document'");
    string_append(&cmd, " -H 'Sec-Fetch-Mode: navigate'");
    string_append(&cmd, " -H 'Sec-Fetch-Site: ");
    referer == NULL ? string_append(&cmd, "none'") : string_append(&cmd, "same-origin'");
    string_append(&cmd, " -H 'Sec-Fetch-User: ?1'");
        
    if (referer != NULL) {
        string_append(&cmd, " --referer '");
        string_append(&cmd, referer);
        string_append(&cmd, "'");
    }

    /* Note: Removed the duplicated append call for cookies.txt */
    string_append(&cmd, " -b cookies.txt -c cookies.txt");
    string_append(&cmd, " --location --max-redirs 10");
    
    string_append(&cmd, " '");
    string_append(&cmd, url);
    string_append(&cmd, "'");
    
    /* popen is now properly recognized due to the _POSIX_C_SOURCE macro */
    FILE* pipe = popen(string_c_str(&cmd), "r");
    
    if (pipe == NULL) {
        printf("[Error] Failed to open pipe for url: %s\n", url);
        return result;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        string_append(&result, buffer);
    }

    pclose(pipe);

    print_log("Finished fetching", url);

    if (string_is_empty(&result)) {
        printf("Fetched HTML is empty for: %s\n", url);
    }

    return result;
}