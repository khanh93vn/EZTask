/**
 * Provide text-based user interface for task-managment system.
 * @todo add menus
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdio.h>
#include <conio.h>
#include <stdarg.h>

#include "core.h"
#include "utils.h"

// ---------------------------------------------------------------------------
// Module constants

#define ITEMS_PER_PAGE 8 /* items per page for display_tasks function */
#define TABLE_FORMAT "%-6.4s%-26.24s%-18.16s%-8.6s%-11.9s%-10.8s\n"

// ---------------------------------------------------------------------------
// Function prototypes

// I/O sub-function
void display_error(const char *error_text, const char *action);
int input_integer(const char *format, ...);
int input_yes_no(const char *format, ...);
int input_task_ui(Task *task);
time_t input_date_time(time_t *t);
long int display_tasks(long int *page_number_ptr,
                       const char *file_name,
                       int as_choices);

// Menus
void main_menu(const char *user_name);
void task_menu(const char *file_name);
void subset_task_menu(const char *title,
                      const char *file_name,
                      const char *tmp_file_name,
                      int (*filter_func)(const char *, const char *));
void add_task_menu(const char *file_name);
void view_task_menu(long int *page_number_ptr, const char *file_name);
void remove_task_menu(long int *page_number_ptr, const char *file_name);

#endif
