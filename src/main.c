#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "database.h"
#include "constants.h"
#include "helpers.h"

void print_help_message(void);


int main(int argc, char *argv[])
{   
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
                printf("Not implemented yet\n");
                break;

            case 'S':
                ret = print_db_contents();
                break;

            case 'V':
                printf("Not implemented yet\n");
                break;

            case 'R':
                printf("Not implemented yet\n");
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