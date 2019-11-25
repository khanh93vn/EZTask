#include "core.h"

// ---------------------------------------------------------------------------
// Task struct basic functions

/**
 * Compute task's end time by adding duration to start time.
 * @param task the task in question.
 * @return task's end time.
 */
 
time_t get_end_time(const Task *task) {
    return task->t_time + task->t_duration_in_mins*SECS_PER_MIN;
}


/**
 * Task struct basic data entry.
 * @param task where entered data reside.
 */

void input_task(Task *task) {
    int temp;
    printf("Name: "); gets(task->t_name);
    printf("Importance (0-255): "); scanf("%d", &temp);
    task->t_importance_rtn = (uint8_t)temp;
    printf("Repeated: "); scanf("%d", &temp);
    task->t_repeat_cnt = (uint16_t)temp;
    printf("Time (secs from 1/1/1900): "); scanf("%d", &task->t_time);
    printf("Duration (minutes): "); scanf("%d", &temp);
    task->t_duration_in_mins = (uint16_t)temp;
    printf("Flags: "); scanf("%d", &task->flags);
}


/**
 * Print task's fields onto the screen.
 * @param task task to print.
 */

void print_task(const Task *task) {
    time_t t_end = get_end_time(task);
    
    printf("\nTask: %s\n", task->t_name);
    printf("Importance: %d\n", task->t_importance_rtn);
    printf("Time: from %s ", time2str(&task->t_time));
    printf("to %s\n", time2str(&t_end));
    printf("Repeated %d times\n", task->t_repeat_cnt);
    printf("Active: %s\n", (task->flags & FLAG_ACTIVE)?"Yes":"No");
    printf("Daily: %s\n", (task->flags & FLAG_DAILY)?"Yes":"No");
    printf("Weekly: %s\n", (task->flags & FLAG_WEEKLY)?"Yes":"No");
    printf("Collision warning: %s\n",
           (task->flags & FLAG_COLLISION_WARNING)?"Yes":"No");
}


/**
 * Update task based on current time and it's flags.
 * Check if task's time has passed,
 * update it based on whether it's daily, weekly or one-time.
 * @param task the task being checked.
 * @param now time of type time_t used as reference.
 */
 
void update_task(Task *task) {
    time_t now;
    
    time(&now); // get current time
    if(now<get_end_time(task)) // compare with target date
        return;
        
    // Task has passed, check if it is recurrent (daily or weekly):
    
    if(task->flags & FLAG_DAILY)
        while(now>=get_end_time(task)) { // make next time arrangement
            task->t_time += SECS_PER_DAY;
            task->t_repeat_cnt++; // add 1 to repeated times
        }
    else if(task->flags & FLAG_WEEKLY)
        while(now>=get_end_time(task)) {
            task->t_time += SECS_PER_WEEK;
            task->t_repeat_cnt++;
        }
    else // a one-time job, deactivate it
        task->flags &= ~FLAG_ACTIVE; // deactivate task
}

// ---------------------------------------------------------------------------
// File manipulation functions

/**
 * Open tasks file, return an integer.
 * @param file_name name of the file containing data of tasks.
 * @return number of tasks if successful, else -1.
 */
 
long int get_task_cnt(const char *file_name) {
    FILE *fp;
    long int file_size;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
    
    if(file_size%sizeof(Task)) {
        printf("Error: Invalid file structure...\n");
        return UNSUCCESSFUL;
    }
    
    
    return file_size/sizeof(Task);
}


/**
 * Save task onto hard disk, return an integer.
 * @param task task to save.
 * @param file_name name of file to open.
 * @return 0 is successful, else -1.
 */

int save_task(Task *task, const char *file_name) {
    FILE *fp = fopen(file_name, "ab");
    
    if(fp == NULL) {
        fp = fopen(file_name, "wb");
        if(fp == NULL) {
            printf("Error: Unable to open file...\n");
            return UNSUCCESSFUL;
        }
    }
    
    fwrite(task, sizeof(Task), 1, fp);
    fclose(fp);
    
    return SUCCESSFUL;
}


/**
 * Read a task from file, return an integer.
 * @param task place-holder for the task read from file.
 * @param index number of tasks from the beginning of the file to the task
 *              in question.
 * @param file_name name of the file containing data of tasks.
 * @return 0 if successful, else -1.
 */

int read_task(Task *task, long int index, const char *file_name) {
    FILE *fp;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    fseek(fp, index*sizeof(Task), SEEK_SET);
    if(fread(task, sizeof(Task), 1, fp) == NULL) {
        printf("Error: Specified index exceeds file size...\n");
        return UNSUCCESSFUL;
    }
        
    fclose(fp);
    
    return SUCCESSFUL;
}


/**
 * Read a number of consecutive tasks from file, return an integer.
 * @param task place-holder for the task read from file.
 * @param index number of tasks from the beginning of the file to the first
                task read.
 * @param file_name name of the file containing data of tasks.
 * @return number of task read if successful, else -1.
 */

int read_tasks(Task **tasks,
               long int index,
               int num_to_read,
               const char *file_name) {
    FILE *fp;
    int task_cnt;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    *tasks = realloc(*tasks, num_to_read*sizeof(Task));
    fseek(fp, index*sizeof(Task), SEEK_SET);
    for(task_cnt = 0;
        fread(*(tasks+task_cnt), sizeof(Task), 1, fp) != NULL
        && task_cnt < num_to_read;
        task_cnt++);
        
    fclose(fp);
    
    *tasks = realloc(*tasks, task_cnt*sizeof(Task));
    return task_cnt;
}


/**
 * Get on going tasks from file, return an integer.
 * @param tasks place-holder for tasks read from file.
 * @param file_name name of the file containing data of tasks.
 * @return number of on going task if successful, else -1.
 */
int get_current_tasks(Task **tasks, const char *file_name) {
    FILE *fp;
    time_t now;
    long int task_cnt;
    long int task_cnt_max;
    
    task_cnt_max = get_task_cnt(file_name);
    if(task_cnt_max<1) return UNSUCCESSFUL;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    *tasks = realloc(*tasks, task_cnt_max*sizeof(Task));
    time(&now); // get current time
    
    task_cnt = 0;
    while(fread((*tasks+task_cnt), sizeof(Task), 1, fp) != NULL) {
        if((*tasks+task_cnt)->flags & FLAG_ACTIVE
           && (*tasks+task_cnt)->t_time < now
           && get_end_time((*tasks+task_cnt)) > now) {
            task_cnt++;
        }
    }
    
    fclose(fp);
    *tasks = realloc(*tasks, task_cnt*sizeof(Task));
    
    return task_cnt;
}


/**
 * Get next task from file, return an integer.
 * @param task place-holder for the task read from file.
 * @param file_name name of the file containing data of tasks.
 * @return number of minutes until read task start if successful, else -1.
 */

int get_next_task(Task *task,
                  uint8_t importance_threshold,
                  const char *file_name) {
    FILE *fp;
    Task *buffer;
    time_t now;
    time_t min_time = TIME_T_MAX;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    buffer = (Task *)malloc(sizeof(Task));
    time(&now); // get current time
    
    while(fread(buffer, sizeof(Task), 1, fp) != NULL) {
        if(buffer->flags & FLAG_ACTIVE
           && buffer->t_importance_rtn > importance_threshold
           && buffer->t_time > now
           && buffer->t_time - now < min_time) {
            *task = *buffer;
            min_time = task->t_time - now;
        }
    }
    
    fclose(fp);
    free(buffer);
    
    if(buffer == NULL) return UNSUCCESSFUL;
    if(min_time == TIME_T_MAX) return UNSUCCESSFUL;
    
    return min_time/SECS_PER_MIN;
}


/**
 * Read tasks to be completed today from file, save to another file.
 * @param dest_file_name name of the file to save to.
 * @param file_name name of the file containing data of tasks.
 * @return number of tasks read if successful, else -1.
 */

int get_day_tasks(const char *dest_file_name, const char *file_name) {
    FILE *fp;
    FILE *fp_out;
    Task *task;
    time_t now;
    time_t midnight;
    int task_cnt;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    fp_out = fopen(dest_file_name, "wb");
    if(fp == NULL) {
        fclose(fp);
        return UNSUCCESSFUL;
    }
    
    task = (Task *)malloc(sizeof(Task));
    time(&now); // get current time
    midnight = get_midnight(now); // get midnight
    
    task_cnt = 0;
    while(fread(task, sizeof(Task), 1, fp) != NULL) {
        if(task->flags & FLAG_ACTIVE
           && task->t_time > now
           && task->t_time < midnight) {
            task_cnt++;
            fwrite(task, sizeof(Task), 1, fp_out);
        }
    }
    
    fclose(fp);
    fclose(fp_out);
    free(task);
    
    return task_cnt;
}


/**
 * Read important tasks 'til next sunday from file.
 * @param dest_file_name name of the file to save to.
 * @param file_name name of the file containing data of tasks.
 * @return number of tasks read if successful, else -1.
 */

int get_week_tasks(const char *dest_file_name, const char *file_name) {
    FILE *fp;
    FILE *fp_out;
    Task *task;
    time_t now;
    time_t weekend;
    int task_cnt;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    fp_out = fopen(dest_file_name, "wb");
    if(fp == NULL) {
        fclose(fp);
        return UNSUCCESSFUL;
    }
    
    task = (Task *)malloc(sizeof(Task));
    time(&now); // get current time
    weekend = get_weekend_midnight(now); // get weekend midnight
    
    task_cnt = 0;
    while(fread(task, sizeof(Task), 1, fp) != NULL) {
        if(task->flags & FLAG_ACTIVE
           && task->t_time > now
           && task->t_time < weekend
           && task->t_importance_rtn >= IMPORTANCE_THRESHOLD) {
            task_cnt++;
            fwrite(task, sizeof(Task), 1, fp_out);
        }
    }
    
    fclose(fp);
    fclose(fp_out);
    free(task);
    
    return task_cnt;
}


/**
 * Read and update all tasks from file.
 * @param file_name name of file to open.
 * @return 0 is successful, else -1.
 */
 
int update_all_tasks(const char *file_name) {
    FILE *fp;
    FILE *fp_tmp;
    Task *task;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    fp_tmp = fopen(TMP_FILE_NAME, "wb");
    if(fp_tmp == NULL) {
        fclose(fp); // close fp as it has been opened
        printf("Error: Unable to open file...\n");
        return UNSUCCESSFUL;
    }
    task = (Task *)malloc(sizeof(Task));
    
    while(fread(task, sizeof(Task), 1, fp) != NULL) {
        if(task->flags & FLAG_ACTIVE) update_task(task);
        fwrite(task, sizeof(Task), 1, fp_tmp);
    }
    
    fclose(fp);
    fclose(fp_tmp);
    free(task);
    
    remove(file_name);
    rename(TMP_FILE_NAME, file_name);
    
    return SUCCESSFUL;
}


/**
 * Remove a task on file, return an integer.
 * @param index number of tasks from the beginning of the file to the task
 *              in question.
 * @param file_name name of the file containing data of tasks.
 * @return 0 if successful, else -1.
 */

int delete_task(long int index, const char *file_name) {
    FILE *fp;
    FILE *fp_tmp;
    Task *task;
    
    fp = fopen(file_name, "rb");
    if(fp == NULL) return UNSUCCESSFUL;
    
    fp_tmp = fopen(TMP_FILE_NAME, "wb");
    if(fp_tmp == NULL) {
        fclose(fp); // close fp as it has been opened
        printf("Error: Unable to open file...\n");
        return UNSUCCESSFUL;
    }
    task = (Task *)malloc(sizeof(Task));
    
    for(long int i = 0;
        fread(task, sizeof(Task), 1, fp) != NULL;
        i++)
        if(i != index) fwrite(task, sizeof(Task), 1, fp_tmp);
    
    fclose(fp);
    fclose(fp_tmp);
    free(task);
    
    remove(file_name);
    rename(TMP_FILE_NAME, file_name);
    
    return SUCCESSFUL;
}
