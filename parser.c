#include <regex.h>
#include <stdlib.h>

#include "parser.h"


char* get_client_request(char* buffer) {
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    char *url_encoded;
    if (regexec(&regex, buffer, 2, matches, 0) == 0) {
        buffer[matches[1].rm_eo] = '\0';
        url_encoded = buffer + matches[1].rm_so;
    }
    regfree(&regex);
    return url_encoded;
}


char* extract_filename(const char *input) {
    regex_t regex;
    regmatch_t match[2];
    char *pattern_filename = "([^./]+)\\.";
    char *filename = NULL;

    regcomp(&regex, pattern_filename, REG_EXTENDED);
    if (regexec(&regex, input, 2, match, 0) == 0) {
        filename = strndup(input + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    }

    regfree(&regex);
    return filename;
}


char* extract_extension(const char *input) {
    regex_t regex;
    regmatch_t match[2];
    char *pattern_extension = "\\.(\\w+)";
    char *extension = NULL;

    regcomp(&regex, pattern_extension, REG_EXTENDED);
    if (regexec(&regex, input, 2, match, 0) == 0) {
        extension = strndup(input + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    }

    regfree(&regex);
    return extension;
}


char* extract_params(const char *input) {
    regex_t regex;
    regmatch_t match[2];
    char *pattern_params = "\\?(.+)";
    char *params = NULL;

    regcomp(&regex, pattern_params, REG_EXTENDED);
    if (regexec(&regex, input, 2, match, 0) == 0) {
        params = strndup(input + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    }

    regfree(&regex);
    return params;
}
