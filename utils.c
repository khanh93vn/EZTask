#include "utils.h"

// ---------------------------------------------------------------------------
// Utility functions

/**
 * Convert time_t into string of format %H:%M %d/%m/%Y.
 * @param t Seconds passed from January 1st 1900 to a specific time.
 * @return The output string contain date-time of the specific time from
 *         the input.
 */

const char *time2str(const time_t *t) {
    char s[17];
    strftime(s, 17, "%H:%M %d/%m/%Y", localtime(t));
    return s;
}


/**
 * Take a string, add extension behind it, return the result.
 * Remember to free memory of the returned string.
 */

char *username2datafilename(const char *username, const char *postfix) {
    char *dfn = (char *)malloc(
        sizeof(char)
        * (strlen(username)
           + strlen(postfix)
           + strlen(DATAFILE_EXTENSION)));
    strcpy(dfn, username);
    strcat(dfn, postfix);
    strcat(dfn, DATAFILE_EXTENSION);
    return dfn;
}

time_t get_midnight(time_t t) {
    struct tm time_info = *localtime(&t);
    
    time_info.tm_mday += 1;
    time_info.tm_hour = 0;
    time_info.tm_min = 0;
    time_info.tm_sec = 0;
    
    return mktime(&time_info);
}
time_t get_weekend_midnight(time_t t) {
    struct tm time_info = *localtime(&t);
    
    time_info.tm_mday += 8 - time_info.tm_wday;
    time_info.tm_hour = 0;
    time_info.tm_min = 0;
    time_info.tm_sec = 0;
    
    return mktime(&time_info);
}