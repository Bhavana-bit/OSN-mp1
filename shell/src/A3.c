#include "A3.h"

// Implement the compile_syntax_regex function
int compile_syntax_regex(regex_t *regex)
{
    const char *pattern = "^[a-zA-Z0-9_\\- ]+$"; // Example: allow alphanum, _, -, space
    int ret = regcomp(regex, pattern, REG_EXTENDED);
    if (ret != 0)
    {
        char errbuf[128];
        regerror(ret, regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Regex compilation failed: %s\n", errbuf);
        return 1;
    }
    return 0;
}

// Implement the validate_command function
int validate_command(regex_t *regex, const char *input)
{
    int ret = regexec(regex, input, 0, NULL, 0);
    return ret == 0; // 0 means match
}
