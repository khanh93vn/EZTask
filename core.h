/**
 * Task structure and related functions.
 * @author Khanh Nguyen
 */

#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "utils.h"

// ---------------------------------------------------------------------------
// Module constants

/**
 * These flags shape the program's behavior on a Task.
 */
#define FLAG_ACTIVE 0x01
#define FLAG_WEEKLY 0x02
#define FLAG_DAILY 0x04
#define FLAG_COLLISION_WARNING 0x08
#define FLAG_RESERVED_1 0x10
#define FLAG_RESERVED_2 0x20
#define FLAG_RESERVED_3 0x40
#define FLAG_RESERVED_4 0x80

/**
 * Tasks with importance rating above this value are considered important.
 */
#define IMPORTANCE_THRESHOLD 10

#define TASK_NAME_MAXLEN 64
#define TMP_FILE_NAME "tmp.bin"

// ---------------------------------------------------------------------------
// Task struct
// Contain basic information of a task, i.e task's name, time, ...

typedef struct {
    char t_name[TASK_NAME_MAXLEN]; // task's name
    time_t t_time; // task's start time
    uint16_t t_duration_in_mins; // task's duration
    uint16_t t_repeat_cnt; // times repeated
    uint8_t t_importance_rtn; // task's importance
    uint8_t flags;
} Task;

// ---------------------------------------------------------------------------
// Functions Prototypes

// Basics
time_t get_end_time(const Task *task);
void input_task(Task *task);
void print_task(const Task *task);
void update_task(Task *task);

// File manipulation
long int get_task_cnt(const char *file_name);
int save_task(Task *task, const char *file_name);
int read_task(Task *task, long int index, const char *file_name);
int read_tasks(Task **tasks,
               long int index,
               int num_to_read,
               const char *file_name);
int get_current_tasks(Task **tasks, const char *file_name);
int get_next_task(Task *task,
                  uint8_t importance_threshold,
                  const char *file_name);
int get_day_tasks(const char *dest_file_name, const char *file_name);
int get_week_tasks(const char *dest_file_name, const char *file_name);
int update_all_tasks(const char *file_name);
int delete_task(long int index, const char *file_name);

#endif
