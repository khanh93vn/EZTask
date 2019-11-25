/**
 * Provide utilities for task-managment system.
 * @author Khanh Nguyen
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

// ---------------------------------------------------------------------------
// Module constants

#define DATAFILE_EXTENSION ".dat"
#define TIME_T_MAX ~(1<<(4*sizeof(time_t)-1))
#define DAYS_PER_WEEK 7
#define HOURS_PER_DAY 24
#define MINS_PER_HOUR 60
#define SECS_PER_MIN 60
#define SECS_PER_DAY 86400
#define SECS_PER_WEEK 604800

#define SUCCESSFUL 0
#define UNSUCCESSFUL -1

// ---------------------------------------------------------------------------
// Functions Prototypes

const char *time2str(const time_t *t);
char *username2datafilename(const char *username, const char *postfix);
time_t get_midnight(time_t t);
time_t get_weekend_midnight(time_t t);

#endif
