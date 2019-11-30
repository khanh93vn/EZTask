#include "ui.h"

// ---------------------------------------------------------------------------
// Data I/O sub-functions

/**
 * Display error text, wait for enter key. 
 */
 
void display_error(const char *error_text, const char *action) {
    printf("\n%s... Press any key to %s...\n", error_text, action);
    getch();
}


/**
 * Display a string, read from keyboard and return an integer.
 * @param format choices for selection.
 * @return integer read from keyboard.
 */
 
int input_integer(const char *format, ...) {
    int choice;
    char text[256];
    va_list args;
    va_start(args, format);
    vsprintf(text, format, args);
    printf("\n%s", text); scanf("%d", &choice);
    va_end(args);
    return choice;
}


/**
 * Display a string, read a character from key board and return an integer.
 * @param question a yes/no question.
 * @return 1 if entered character is "Y" or "y", else 0.
 */

int input_yes_no(const char *format, ...) {
    char question[256];
    char answer;
    char text[256];
    va_list args;
    va_start(args, format);
    vsprintf(question, format, args);
    printf("%s (Y/N) ", question);
    fflush(stdin); scanf("%c", &answer);
    va_end(args);
    return answer=='y'||answer=='Y';
}


/**
 * Enter task information from keyboard.
 * @param task where entered data reside.
 * @return 0 if input is successful else -1.
 */

int input_task_ui(Task *task) {
    task->t_repeat_cnt = 0;
    task->flags = 0x00; // reset flags
    task->flags |= FLAG_ACTIVE; // activate
    
    fflush(stdin); // remove left-overs inputs from buffer
    printf("Please enter the information below:\n");
    printf("Task: "); gets(task->t_name);
    task->t_importance_rtn =
        (uint8_t)input_integer("Importance rating (0-255): ");
    if(input_date_time(&task->t_time) == UNSUCCESSFUL)
        return UNSUCCESSFUL;
    
    task->t_duration_in_mins = (uint16_t)input_integer("Duration (minutes): ");
    
    // Recurrence flags:
    if(input_yes_no("Will it be repeated?"))
        if(input_yes_no("Daily?"))
            task->flags |= FLAG_DAILY;
        else if(input_yes_no("Weekly?"))
            task->flags |= FLAG_WEEKLY;
            
    // Collision warning flag:
    if(input_yes_no("Would you like to be warned when this task "
          "collides with other tasks?"))
        task->flags |= FLAG_COLLISION_WARNING;
    
    return SUCCESSFUL;
}


/**
 * Enter date-time data via keyboard, return a time_t value.
 * @return date-time entered if successful, else -1.
 */

time_t input_date_time(time_t *t) {
    
    struct tm t_time;
    time_t now;
    int choice, temp;
    
    choice = input_integer(
        "Please enter target date, choose your method:\n"
        "[1] Based on current date\n"
        "[2] Manual entry\n"
        "[0] Cancel\n"
        "Your choice: ");
    switch(choice) {
        case 0:
            return UNSUCCESSFUL;
        case 1:
            time(&now);
            t_time = *localtime(&now);
            printf("-Days from now: "); scanf("%d", &temp);
            t_time.tm_mday += temp;
            printf("-Months from now: "); scanf("%d", &temp);
            t_time.tm_mon += temp;
            printf("-Years from now: "); scanf("%d", &temp);
            t_time.tm_year += temp;

            break;
        case 2:
            printf("-Day: "); scanf("%d", &t_time.tm_mday);
            printf("-Month: "); scanf("%d", &t_time.tm_mon);
            t_time.tm_mon -= 1;
            printf("-Year: "); scanf("%d", &t_time.tm_year);
            t_time.tm_year -= 1900; // year 1900 ~ tm_year = 0
            
            break;
        default:
            display_error("Invalid input", "exit");
            return UNSUCCESSFUL;
    }
    
    printf("Time of day:\n");
    printf("-Hour: "); scanf("%d", &t_time.tm_hour);
    printf("-Minute: "); scanf("%d", &t_time.tm_min);
    
    *t = mktime(&t_time);
    
    return *t;
}


/**
 * Read tasks from file, display them in a table, return a long integer.
 *
 * @param page_number_ptr Pointer of page number.
 * @param file_name name of the file containing data of tasks.
 * @param as_choice determine whether to display items as choices for inputs.
 * @return number displayed items on page.
 */

long int display_tasks(long int *page_number_ptr,
                       const char *file_name,
                       int is_choice) {
    FILE *fp;
    Task *task;
    long int task_cnt, page_cnt;
    int item_cnt;
    
    // Display table headers:
    printf(TABLE_FORMAT,
           "Id", "Task", "Date", "Active", "Recurrent", "Repeated");
    
    // Check if there's any item to display:
    task_cnt = get_task_cnt(file_name);
    if(task_cnt<1) {
        printf("\n(There is nothing to display)\n\n");
        return 0;
    }
    
    page_cnt = (task_cnt-1)/ITEMS_PER_PAGE;
    
    // Clamp page_number into [0..page_cnt]:
    if(*page_number_ptr<0) *page_number_ptr = 0;
    if(*page_number_ptr>page_cnt) *page_number_ptr = page_cnt;
    
    // Open file and allocate memory for buffer:
    fp = fopen(file_name, "rb");
    task = malloc(sizeof(Task));
    
    // Move cursor to target page:
    fseek(fp, *page_number_ptr*ITEMS_PER_PAGE*sizeof(Task), SEEK_SET);
    
    // Read and display items:
    for(item_cnt = 1;
        item_cnt<=ITEMS_PER_PAGE && fread(task, sizeof(Task), 1, fp);
        item_cnt++) {
        char index[10];
        char repeated[10];
        if(is_choice)
            sprintf(index, "[%d]", item_cnt);
        else
            sprintf(index, "%d", item_cnt);
        sprintf(repeated, "%d", task->t_repeat_cnt+1);
        printf(
            TABLE_FORMAT,
            index, task->t_name,
            time2str(&task->t_time),
            (task->flags & FLAG_ACTIVE)?"Yes":"No",
            (task->flags & (FLAG_DAILY | FLAG_WEEKLY))?"Yes":"No",
            repeated
        );
    }

    printf("(%d - %d item(s) out of %d)\n\n",
           *page_number_ptr*ITEMS_PER_PAGE + 1,
           *page_number_ptr*ITEMS_PER_PAGE + item_cnt - 1,
           task_cnt);
    
    // Close file, free memory:
    fclose(fp);
    free(task);
    
    return item_cnt;
}


// ---------------------------------------------------------------------------
// Menus

void main_menu(const char *user_name) {
    Task *current_tasks = (Task *)malloc(sizeof(Task));
    Task *next_task = (Task *)malloc(sizeof(Task));
    char *file_name = username2datafilename(user_name, "");
    char *file_name_day = username2datafilename(user_name, "_day");
    char *file_name_week = username2datafilename(user_name, "_week");
    uint8_t threshold_for_next_task = 0;
    int choice,
        current_tasks_cnt,
        weeks_til_next_task,
        days_til_next_task,
        hours_til_next_task,
        minutes_til_next_task;
    
    do {
        system("cls");
        update_all_tasks(file_name);
        printf("Welcome to EZ Task, %s!\n\n", user_name);
        
        // Display current tasks:
        current_tasks_cnt = get_current_tasks(&current_tasks, file_name);
        printf("You have %d on going task%s%s\n",
               current_tasks_cnt,
               current_tasks_cnt>1?"s":"", // display in plural if true
               current_tasks_cnt>0?":":".");
        for(int i = 0; i < current_tasks_cnt; i++)
            printf("-%s\n", (current_tasks+i)->t_name);
        
        // Display next tasks:
        printf("\nNext task: (with threshold %d)\n", threshold_for_next_task);
        minutes_til_next_task = get_next_task(next_task,
                                              threshold_for_next_task,
                                              file_name);
        if(minutes_til_next_task >= 0) {
            
            hours_til_next_task = minutes_til_next_task / MINS_PER_HOUR;
            minutes_til_next_task %= MINS_PER_HOUR;
            days_til_next_task = hours_til_next_task / HOURS_PER_DAY;
            hours_til_next_task %= HOURS_PER_DAY;
            weeks_til_next_task = days_til_next_task / DAYS_PER_WEEK;
            days_til_next_task %= DAYS_PER_WEEK;
            
            printf("%s, coming in ", next_task->t_name);
            if(weeks_til_next_task)
                printf("%d week%s ",
                       weeks_til_next_task,
                       weeks_til_next_task>1?"s":"");
            if(days_til_next_task)
                printf("%d day%s ",
                       days_til_next_task,
                       days_til_next_task>1?"s":"");
            if(hours_til_next_task)
                printf("%d hour%s ",
                       hours_til_next_task,
                       hours_til_next_task>1?"s":"");
            printf("%d minute%s.\n",
                   minutes_til_next_task,
                   minutes_til_next_task>1?"s":"");
        } else printf("None\n");
        
        // Display and read choices:
        choice = input_integer(
            "[1] Manage tasks\n"
            "[2] Today's tasks\n"
            "[3] This week's important tasks (rating >= 10)\n"
            "[4] Threshold +\n"
            "[5] Threshold -\n"
            "[0] Exit\n"
            "\nPlease enter your choice: "
        );
        switch(choice) {
            case 0: // exit
                printf("See you soon!\n");
                getch();
                break;
            case 1: // all task
                task_menu(file_name);
                break;
            case 2: // day's task
                subset_task_menu("Today's tasks",
                                 file_name,
                                 file_name_day,
                                 get_day_tasks);
                break;
            case 3: // week's task
                subset_task_menu("This week important tasks",
                                 file_name,
                                 file_name_week,
                                 get_week_tasks);
                break;
            case 4: // increase importance threshold
                threshold_for_next_task++;
                break;
            case 5: // decrease importance threshold
                threshold_for_next_task--;
                break;
            default:
                display_error("Invalid input", "continue");
                break;
        }
    } while(choice);
    
    free(current_tasks);
    free(next_task);
    free(file_name);
    free(file_name_day);
    free(file_name_week);
}


void task_menu(const char *file_name) {
    int choice;
    long int page_number = 0;
    
    do {
        update_all_tasks(file_name);
        system("cls");
        printf("All tasks:\n\n");
        display_tasks(&page_number, file_name, 0);
        choice = input_integer(
            "[1] Next page\n"
            "[2] Previous page\n"
            "[3] Add\n"
            "[4] View\n"
            "[5] Remove\n"
            "[0] Back\n"
            "\nPlease enter your choice: "
        );
        switch(choice) {
            case 0: // back to main menu
                break;
            case 1:
                page_number++;
                break;
            case 2:
                page_number--;
                break;
            case 3:
                add_task_menu(file_name);
                break;
            case 4: // view item, need exact position
                view_task_menu(&page_number, file_name);
                break;
            case 5: // remove item, need exact position
                remove_task_menu(&page_number, file_name);
                break;
            default:
                display_error("Invalid input", "continue");
                break;
        }
    } while(choice);
}


void subset_task_menu(const char *title,
                      const char *file_name,
                      const char *tmp_file_name,
                      int (*filter_func)(const char *, const char *)) {
    int choice;
    long int page_number = 0;
    
    do {
        system("cls");
        update_all_tasks(file_name);
        (*filter_func)(tmp_file_name, file_name);
        printf("%s:\n\n", title);
        display_tasks(&page_number, tmp_file_name, 0);
        choice = input_integer(
            "[1] Next page\n"
            "[2] Previous page\n"
            "[3] View\n"
            "[0] Back\n"
            "\nPlease enter your choice: "
        );
        switch(choice) {
            case 0: // back to main menu
                break;
            case 1:
                page_number++;
                break;
            case 2:
                page_number--;
                break;
            case 3: // view item, need exact position
                view_task_menu(&page_number, tmp_file_name);
                break;
            default:
                display_error("Invalid input", "continue");
                break;
        }
    } while(choice);
    
    remove(tmp_file_name);
}

void add_task_menu(const char *file_name) {
    Task *task = (Task *)malloc(sizeof(Task));
    
    system("cls");
    if(input_task_ui(task) == UNSUCCESSFUL)
        display_error("Task entry has been cancelled", "go back");
    else
        save_task(task, file_name);
    free(task);
}

void view_task_menu(long int *page_number_ptr, const char *file_name) {
    int choice;
    int item_cnt;
    Task *task = (Task *)malloc(sizeof(Task));
    
    do {
        system("cls");
        printf("View task: \n\n");
        if(get_task_cnt(file_name) < 1) {
            display_error("Nothing to view", "go back");
            break;
        }
        
        item_cnt = display_tasks(page_number_ptr, file_name, 1);
        choice = input_integer(
            "[%d] Next page\n"
            "[%d] Prev page\n"
            "[0] Back\n"
            "Please select one: ",
            ITEMS_PER_PAGE+1, ITEMS_PER_PAGE+2
        );
        
        // Check if choice falls in range:
        if(0 < choice && choice < item_cnt) {
            system("cls");
            read_task(task,
                      *page_number_ptr*ITEMS_PER_PAGE + choice - 1,
                      file_name);
            print_task(task);
            getch();
        } else switch(choice) {
            case 0:
                break;
            case ITEMS_PER_PAGE+1:
                *page_number_ptr++;
                break;
            case ITEMS_PER_PAGE+2:
                *page_number_ptr--;
                break;
            default:
                display_error("Invalid input", "continue");
                break;
        }
        
    } while(choice);
    
    free(task);
}

void remove_task_menu(long int *page_number_ptr, const char *file_name) {
    int choice;
    int item_cnt;
    
    do {
        system("cls");
        printf("Remove task: \n\n");
        if(get_task_cnt(file_name) < 1) {
            display_error("Nothing to remove", "go back");
            break;
        }
        
        item_cnt = display_tasks(page_number_ptr, file_name, 1);
        choice = input_integer(
            "[%d] Next page\n"
            "[%d] Prev page\n"
            "[0] Back\n"
            "Please select one: ",
            ITEMS_PER_PAGE+1, ITEMS_PER_PAGE+2
        );
        
        // Check if choice falls in range:
        if(0 < choice && choice < item_cnt)
            delete_task(*page_number_ptr*ITEMS_PER_PAGE + choice - 1,
                        file_name);
        else switch(choice) {
            case 0:
                break;
            case ITEMS_PER_PAGE+1:
                *page_number_ptr++;
                break;
            case ITEMS_PER_PAGE+2:
                *page_number_ptr--;
                break;
            default:
                display_error("Invalid input", "continue");
                break;
        }
        
    } while(choice);
}
