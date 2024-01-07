#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "database.h"
#include "constants.h"
#include "helpers.h"

void print_help_message(void);
static int initialize_db_path(void);

char _db_path[50] = "/home/";
const char *username = NULL, *DB_PATH = NULL;

int main(int argc, char *argv[])
{   
    // Determine database location in filesystem
    int got_path = initialize_db_path();
    if (got_path != RC_SUCCESS){
        printf("Something went wrong in determining the expected database path.\n");
        exit(RC_FAILED);
    }

    printf("\n\nWelcome to the student database tool!\n\n");

    // Main Loop
    do
    {
        print_help_message();     
        char user_input = getchar();
        clear_input_buffer();
        int ret;
        user_input = toupper(user_input);

        switch (user_input) {

            case 'I':
                ret = init_student_database();
                break;

            case 'A': 
                ret = add_student();
                break;

            case 'M':
                ret = modify_student();
                break;

            case 'S':
                ret = print_db_contents();
                break;

            case 'V':
                ret = get_student_info();
                break;

            case 'R':
                ret = remove_student();
                break;

            case 'Q':
                printf("Thank you for using student database tool!\n");
                exit(0);

            default:
                printf("You provided an unrecognized input!\n");
                ret = RC_SUCCESS;
                break;
        }

        if(ret != RC_SUCCESS){
            printf("An error occured when trying to execute the selected operation\n");
            exit(RC_FAILED);
        }

    printf("\n\n");

    }
    while (1);

}


/**
* Print a help menu to provide guidance to the users. 
*/
void print_help_message(void)
{
    printf("\nType 'I' to initialize the student database...\n");
    printf("Type 'A' to add a student to the database...\n");
    printf("Type 'M' to modify an existing student in the database...\n");
    printf("Type 'S' to show all existing students in the database...\n");
    printf("Type 'V' to view an individual student in the database...\n");
    printf("Type 'R' to remove a student from the database...\n");
    printf("Type 'Q' to exit...\n\n");
}


/**
 * Determine the path in which the student database will be created.
*/
static int initialize_db_path(void)
{
    username = getenv("USER");
    if (username != NULL){
        // Construct db.json absolute path based on $USER env variable
        strcat(_db_path, username);
        strcat(_db_path, "/db.json");
    } else {
        printf("Something went wrong finding username...\n");
        return RC_FAILED;
    }

    DB_PATH = _db_path;
    return RC_SUCCESS;
}
