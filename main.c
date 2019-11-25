/** 
 * Ez Task - a personal task-management system by Khanh Nguyen
 */

#include "ui.h"

int log_in(char *usrn);

int main() {
    char username[32];
    
    if(log_in(username) == UNSUCCESSFUL) return -1;
    
    main_menu(username);
    
    return 0;
}

int log_in(char *usrn) {
    int reenter;
    do {
        system("cls");
        printf("Username: "); gets(usrn);
        if(!isalpha(*usrn)) {
            display_error("Usernames must start with alphabetical character",
                          "retry");
            reenter = 1;
            continue;
        }
        reenter = 0;
        for(int i = 0; i < strlen(usrn); i++) {
            if(!isalnum(usrn[i]) && usrn[i] != '_' && usrn[i] != '-') {
                display_error("Invalid username", "retry");
                reenter = 1;
                break;
            }
        }
    } while(reenter);
    
    char *file_name = username2datafilename(usrn, "");
    
    FILE *fp;
    fp = fopen(file_name, "rb");
    if(fp == NULL) {
        if(input_yes_no("Account does not exist. Create account?")) {
            fp = fopen(file_name, "wb");
            if(fp == NULL) {
                display_error("Unable to create file", "exit");
                return UNSUCCESSFUL;
            }
            fclose(fp);
        } else {
            display_error("Log in cancelled", "exit");
            return UNSUCCESSFUL;
        }
    }
    fclose(fp);
    return SUCCESSFUL;
}

// ---------------------------------------------------------------------------
