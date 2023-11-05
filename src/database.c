#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <json-c/json.h>
#include "student.h"
#include "constants.h"
#include "helpers.h"

#define MAX_NAME_SIZE 20


// Helper function definitions
static int get_student_information(uint_32 *student_id, char *first_name, char *last_name);
static void get_user_input(const char *prompt, char *result, int max_result_size);
static int add_student_to_db(Student student);
static FILE *get_db_file_handler();
static bool does_db_exist();
static struct json_object *get_db_contents();
static void overwrite_database_contents(const char *db_contents);


int init_student_database(){
    FILE *db_ptr = NULL;
    struct json_object *db_template;

    if(does_db_exist()){
        printf("Student database already exists!\n");
        return RC_SUCCESS;
    }

    printf("Student database does not yet exist... Creating DB...\n");
    db_template = json_object_new_object();
    json_object_object_add(db_template, "students", json_object_new_array());
    
    db_ptr = fopen(DB_PATH, "w");
    if(db_ptr == NULL){
        printf("Something went wrong creating the database file!\n");
        return RC_FAILED;
    }
    
    fprintf(db_ptr, "%s", json_object_get_string(db_template));
    fclose(db_ptr);
    return RC_SUCCESS;
}


int add_student(){
    Student s;
    uint_32 student_id=0;
    char first_name[MAX_NAME_SIZE], last_name[MAX_NAME_SIZE];

    if (!does_db_exist()){
        printf("Student database is not yet initialized!\n");
        return RC_FAILED;
    }
    
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
    json_object *db_contents, *ID, *first_name, *last_name, *student, *students;
    int num_students; 
    const char *seperator = "=========================\n";

    if (!does_db_exist()){
        printf("Student database is not yet initialized!\n");
        return RC_FAILED;
    }

    db_contents = get_db_contents();

    if(db_contents == NULL){
        printf("Failed to get the datbase contents!\n");
        return RC_FAILED;
    }

    json_object_object_get_ex(db_contents, "students", &students);
    num_students = json_object_array_length(students);

    if(!num_students){
        printf("The student database is currently empty\n");
        return RC_SUCCESS;
    }

    printf("\nThe following students were found in the database:\n");

    for(int i=0; i<num_students; i++){
        student = json_object_array_get_idx(students, i);

        json_object_object_get_ex(student, "ID", &ID);
        json_object_object_get_ex(student, "first_name", &first_name);
        json_object_object_get_ex(student, "last_name", &last_name);

        printf("%s", seperator);
        printf("ID: %s\n", json_object_get_string(ID));
        printf("first_name: %s\n", json_object_get_string(first_name));
        printf("last_name: %s\n", json_object_get_string(last_name));
        printf("%s", seperator);
    }

    return RC_SUCCESS;
}


// Database Helpers Below //

static int get_student_information(uint_32 *student_id, char *first_name, char *last_name){

    printf("Please enter your student ID: ");
    scanf("%u", student_id);
    clear_input_buffer();

    get_user_input("Please enter your first name: ", first_name, MAX_NAME_SIZE);
    get_user_input("Please enter your last name: ", last_name, MAX_NAME_SIZE);
    

    /*
    * Validate user input was valid
    */
    if (*student_id < 0){ // TODO FIX THIS CHECK
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
* Add the provided Student to the student database. 
*
* @param student Structure containing student info to be added to db
* @return RC_SUCCESS if the write succeeds, otherwise RC_FAILED
*/
static int add_student_to_db(Student s){
    FILE *db_ptr = NULL;
    int i = 0, c;
    struct json_object *db_contents, *students, *new_student;

    // Convert student to JSON
    new_student = json_object_new_object();
    json_object_object_add(new_student, "ID", json_object_new_int(s.student_id));
    json_object_object_add(new_student, "first_name", json_object_new_string(s.first_name));
    json_object_object_add(new_student, "last_name", json_object_new_string(s.last_name));

    // Append student to the student database
    db_contents = get_db_contents();
    json_object_object_get_ex(db_contents, "students", &students);

    if(db_contents == NULL || students == NULL){
        printf("Failed to get the database contents!\n");
        return RC_FAILED;
    }

    // TODO Add some validation logic here to make sure we're not at the max DB size
    json_object_array_add(students, new_student);


    // Write the updated JSON back to the database 
    db_ptr = db_ptr = fopen(DB_PATH, "w+"); // TODO abstract this to helper function
    if(db_ptr == NULL){
        printf("Failed to open the database to add the student!\n");
        return RC_FAILED;
    }

    fprintf(db_ptr, "%s", json_object_get_string(db_contents));
    fclose(db_ptr);

    return RC_SUCCESS;

}


/**
 *  @return A file handler for the student database.
*/
static FILE *get_db_file_handler(){
    FILE *db_ptr = NULL;

    if(!does_db_exist()){
        printf("The student database is not yet initialized!");
        return db_ptr;
    }
    
    db_ptr = fopen(DB_PATH, "a+");
    return db_ptr;
}


/**
 * Overwrite the contents of the database with the provided contents.
 * 
 * @param db_contents pointer to string of characters to write to db.
*/
static void overwrite_database_contents(const char *db_contents){
    ;
}


/**
 * @return a pointer to json_object containing db contents.
*/
static struct json_object *get_db_contents(){
    FILE *db_ptr;
    char buffer[MAX_DB_SIZE];

    // Read raw DB contents into buffer
    db_ptr = get_db_file_handler();

    if(db_ptr == NULL){
        printf("Something went wrong trying to read the database!\n");
        return NULL;
    }

    fread(buffer, MAX_DB_SIZE, 1, db_ptr);
    fclose(db_ptr);

    return json_tokener_parse(buffer);
}


bool does_db_exist(){
    return access(DB_PATH, F_OK) == 0 ? true : false; 
}