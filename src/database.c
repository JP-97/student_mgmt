#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <json-c/json.h>

#include "database.h"
#include "student.h"
#include "constants.h"


#define MAX_NAME_SIZE 20
const char *STUDENT_FORMAT_OUT = "{\"ID\":%d, \"first_name\":\"%s\", \"last_name\":\"%s\"}\n";
const char *STUDENT_FORMAT_IN = "{\"ID\":%d, \"first_name\":[^\"], \"last_name\":[^\"]}\n";


// Helper function definitions
static int get_student_information(uint_32 *student_id, char *first_name, char *last_name);
static void clear_input_buffer(void);
static void get_user_input(const char *prompt, char *result, int max_result_size);
static int add_student_to_db(Student student);
static FILE *get_db_file_handler();


int add_student(){
    Student s;
    uint_32 student_id=0;
    char first_name[MAX_NAME_SIZE], last_name[MAX_NAME_SIZE];
    
    int got_info = get_student_information(&student_id, first_name, last_name);

    if(got_info != RC_SUCCESS){
        printf("Failed in adding provided student to the database\n");
        return RC_FAILED;
    }
    
    s.student_id = student_id;
    s.first_name = first_name;
    s.last_name = last_name;

    if(add_student_to_db(s) != RC_SUCCESS){
        printf("Failed in adding provided student to the database\n");
        return RC_FAILED;
    }

    printf("\n\nAdded a new student to the database with the following attributes:\n");
    printf("ID: %d\n", s.student_id);
    printf("First Name: %s\n", s.first_name);
    printf("Last Name: %s\n", s.last_name);

    return RC_SUCCESS;
}


int print_db_contents(){
    char *t = NULL; 
    t = get_db_contents();
    return RC_SUCCESS;
}


char *get_db_contents(){
    FILE *db_ptr = NULL;
    char buffer[MAX_DB_SIZE];
    json_object *db_contents;
    json_object *ID;
    json_object *first_name;
    json_object *last_name;

    db_ptr = get_db_file_handler();

    if(db_ptr == NULL){
        return NULL;
    }

    fread(buffer, MAX_DB_SIZE, 1, db_ptr);
    fclose(db_ptr);

    // TODO need to restructure db.json so it's a flat json file with a single array
    // Each array element will represent a student
    // Then, we iterate over the students, 1-by-1, printing them with logic below


    db_contents = json_tokener_parse(buffer);
    json_object_object_get_ex(db_contents, "ID", &ID);  // Each key will return a json object
    json_object_object_get_ex(db_contents, "first_name", &first_name);
    json_object_object_get_ex(db_contents, "last_name", &last_name);

    printf("ID: %s\n", json_object_get_string(ID));
    printf("first_name: %s\n", json_object_get_string(first_name));
    printf("last_name: %s\n", json_object_get_string(last_name));

}


// Helpers Below //

static int get_student_information(uint_32 *student_id, char *first_name, char *last_name){

    char *empty_space;

    printf("Please enter your student ID: ");
    scanf("%u", student_id);

    clear_input_buffer();
    get_user_input("Please enter your first name: ", first_name, MAX_NAME_SIZE);
    get_user_input("Please enter your last name: ", last_name, MAX_NAME_SIZE);
    

    /*
    * Validate user input was valid
    */
    if (student_id < 0){ // TODO FIX THIS CHECK
        printf("Student ID must be non-zero number\n");
        return RC_FAILED;
    }

    if (!strcmp(first_name,"") || !strcmp(last_name,"")){
        printf("\nYou must provide a non-empty first and last name\n");
        return RC_FAILED;
    } 

    else if (strchr(first_name, ' ') || strchr(last_name, ' ')){
        printf("\nPlease ensure first and last name don't contain a space\n");
        return RC_FAILED;
    }

    return RC_SUCCESS;
}


static void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Discard character by doing nothing
    }
}


/**
 * Prompt the user of the program for their input and update in provided result.
 * 
 * @param prompt A pointer to the prompt to be issued to the user.
 * @param result A pointer to the result received from the user.
 * @param max_result_size Int describing max length of the result (ie. user input)
 * 
*/
static void get_user_input(const char *prompt, char *result, int max_result_size){
    printf("%s", prompt);
    fgets(result, max_result_size, stdin);

    // Strip off trailing newline
    size_t len = strlen(result);
    if(len > 0 && result[len-1] == '\n'){
        result[len-1] = '\0';
    }
}


/**
* Create the student database if it doesn't already exist 
* and add the provided student.
*
* @param student Structure containing student info to be added to db
* @return RC_SUCCESS if the write succeeds, otherwise RC_FAILED
*/
static int add_student_to_db(Student s){
    FILE *db_ptr = NULL;
    char c;
    int i = 0;
    
    db_ptr = get_db_file_handler();

    if(db_ptr == NULL){
        printf("Something went wrong trying to access the database\n");
        return RC_FAILED;
    }
    // TODO Add validation logic

    // Student first name and last name have MAX_NAME_SIZE. ID has max int size (4B)

    //Each student takes 3 "lines" in the DB

    fprintf(db_ptr, STUDENT_FORMAT_OUT, s.student_id, s.first_name, s.last_name);

    fclose(db_ptr);
    return RC_SUCCESS;

}


/**
 *  @return A file handler for the student database.
*/
static FILE *get_db_file_handler(){
    FILE *db_ptr = NULL;

    if(access(DB_PATH, F_OK) != 0){
        printf("Student database does not yet exist... Creating DB...\n");
        db_ptr = fopen(DB_PATH, "w+");
    }
    else
        db_ptr = fopen(DB_PATH, "a+");

    return db_ptr;
}