#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <json-c/json.h>
#include "student.h"
#include "constants.h"
#include "helpers.h"

#define MAX_NAME_SIZE 20


// Helper function definitions
static int get_student_information(uint32_t *student_id, char *first_name, char *last_name);
static int get_student_id(uint32_t *student_id);
static void get_user_input(const char *prompt, char *result, int max_result_size);
static int add_student_to_db(Student student);
static FILE *get_db_file_handler();
static bool does_db_exist();
static struct json_object *get_db_contents();
static size_t get_db_size(const struct json_object *db_contents);
static bool overwrite_database_contents(struct json_object *db_contents);
static void print_student_info(const char *student_id, const char *first_name, const char *last_name);
static void get_student(uint32_t student_id, struct json_object **student, int *student_index);


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
    uint32_t student_id=0;
    char first_name[MAX_NAME_SIZE], last_name[MAX_NAME_SIZE];
    struct json_object *db_contents, *students, *student, *ID;

    if (!does_db_exist()){
        printf("Student database is not yet initialized!\n");
        return RC_FAILED;
    }
    
    // Get information for student being added
    int got_info = get_student_information(&student_id, first_name, last_name);

    if(got_info != RC_SUCCESS){
        printf("Failed in adding provided student to the database\n");
        return RC_FAILED;
    }

    s.student_id = student_id;
    s.first_name = first_name;
    s.last_name = last_name;

    // Validate that student is not already in the database
    db_contents = get_db_contents();

    if (db_contents == NULL){
        printf("Failed to get db contents!\n");
        return RC_FAILED;
    }

    json_object_object_get_ex(db_contents, "students", &students);
    
    for(int i=0; i < json_object_array_length(students); i++){
        student = json_object_array_get_idx(students, i);
        json_object_object_get_ex(student, "ID", &ID);
        if(json_object_get_int(ID) == s.student_id){
            printf("There is already a student in the database with ID %d\n", s.student_id);
            return RC_FAILED;
        }
    }

    // Add student to the database
    if (add_student_to_db(s) != RC_SUCCESS){
        printf("Failed in adding provided student to the database\n");
        return RC_FAILED;
    }

    printf("\n\nAdded a new student to the database with the following attributes:\n");
    printf("ID: %d\n", s.student_id);
    printf("First Name: %s\n", s.first_name);
    printf("Last Name: %s\n", s.last_name);

    return RC_SUCCESS;
}

int modify_student(){

    return RC_SUCCESS;
}


int print_db_contents(){
    json_object *db_contents, *ID, *first_name, *last_name, *student, *students;
    int num_students; 

    if (!does_db_exist()){
        printf("Student database is not yet initialized!\n");
        return RC_FAILED;
    }

    db_contents = get_db_contents();

    if(db_contents == NULL){
        printf("Failed to get the datbase contents!\n");
        return RC_FAILED;
    }

    num_students = get_db_size(db_contents);

    if(!num_students){
        printf("The student database is currently empty\n");
        return RC_SUCCESS;
    }

    // Iterate through and print student info to stdout
    json_object_object_get_ex(db_contents, "students", &students);
    printf("\nThe following students were found in the database:\n");

    for(int i=0; i<num_students; i++){
        student = json_object_array_get_idx(students, i);

        json_object_object_get_ex(student, "ID", &ID);
        json_object_object_get_ex(student, "first_name", &first_name);
        json_object_object_get_ex(student, "last_name", &last_name);

        print_student_info(json_object_get_string(ID),
                           json_object_get_string(first_name), 
                           json_object_get_string(last_name));
    }

    return RC_SUCCESS;
}

int get_student_info(){
    uint32_t student_id = 0, i;
    struct json_object *db_contents, *student, *students, *ID, *first_name, *last_name;

    if(get_student_id(&student_id) != RC_SUCCESS)
        return RC_FAILED;

    db_contents = get_db_contents();

    if(db_contents == NULL){
        printf("Failed to get the datbase contents!\n");
        return RC_FAILED;
    }

    // Iterate through database contents until correct
    // student_id is found
    size_t num_students = get_db_size(db_contents);

    if(!num_students){
        printf("The student database is currently empty\n");
        return RC_SUCCESS;
    }

    json_object_object_get_ex(db_contents, "students", &students);

    for(i = 0; i < num_students; i++){
        student = json_object_array_get_idx(students, i);
        json_object_object_get_ex(student, "ID", &ID);
        if(json_object_get_int(ID) == student_id)
            break;
    }

    if(i == num_students){
        printf("Failed to find student id (%d) in the student database", student_id);
        return RC_SUCCESS;
    }

    printf("\nThe following student was found with ID: %d\n", student_id);

    json_object_object_get_ex(student, "ID", &ID);
    json_object_object_get_ex(student, "first_name", &first_name);
    json_object_object_get_ex(student, "last_name", &last_name);

    print_student_info(json_object_get_string(ID),
                       json_object_get_string(first_name), 
                       json_object_get_string(last_name));

    return RC_SUCCESS;
}


int remove_student(){
    uint32_t student_id, num_students, i=0;
    struct json_object *db_contents, *students, *student, *ID, *first_name, *last_name;

    db_contents = get_db_contents();

    if(db_contents == NULL){
        printf("Failed to get the datbase contents!\n");
        return RC_FAILED;
    }

    if(get_student_id(&student_id) != RC_SUCCESS)
        return RC_FAILED;

    get_student(student_id, &student, &i);

    if(student == NULL)
        return RC_FAILED;

    // Print student info back to user and remove it from the database
    json_object_object_get_ex(student, "ID", &ID);
    json_object_object_get_ex(student, "first_name", &first_name);
    json_object_object_get_ex(student, "last_name", &last_name);

    printf("Removing the following student from the database:\n");
    print_student_info(json_object_get_string(ID),
                       json_object_get_string(first_name), 
                       json_object_get_string(last_name));
            
    json_object_object_get_ex(db_contents, "students", &students);
    json_object_array_del_idx(students, i, 1);

    if(!overwrite_database_contents(db_contents))
        return RC_FAILED;

    printf("\nSuccessfully removed student %d from the database\n", student_id);
    return RC_SUCCESS;
}

// Database Helpers Below //

static int get_student_information(uint32_t *student_id, char *first_name, char *last_name){

    
    if(get_student_id(student_id) != RC_SUCCESS)
        return RC_FAILED;

    get_user_input("Please enter your first name: ", first_name, MAX_NAME_SIZE);
    get_user_input("Please enter your last name: ", last_name, MAX_NAME_SIZE);
    

    /*
    * Validate user input was valid
    */
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
 * Get the student id from the user.
 * 
 * @param student_id pointer to an int where the student number will be stored.
 * @return RC_SUCCESS if a valid (non-zero, non-negative) student ID 
 *         was received, otherwise RC_FAILED.
 * 
*/
static int get_student_id(uint32_t *student_id){

    int tmp = 0;
    printf("Please enter your student ID: ");
    scanf("%d", &tmp);
    clear_input_buffer();

    if(tmp <= 0){
        printf("Student ID must be greater than 0!\n");
        return RC_FAILED;
    }

    *student_id = (uint32_t)tmp;
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

    if(!overwrite_database_contents(db_contents))
        return RC_FAILED;

    printf("Successfully added student to the database!\n");
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
 * Overwrite the contents of the database with the provided db_contents.
 * 
 * @param db_contents pointer to string of characters to write to db.
 * @return true if the contents were successfully updated, otherwise false.
*/
static bool overwrite_database_contents(struct json_object *db_contents){
    FILE *db_ptr;
    db_ptr = fopen(DB_PATH, "w+");

    if(db_ptr == NULL){
        printf("Failed to open the database\n");
        return false;
    }

    fprintf(db_ptr, "%s", json_object_get_string(db_contents));
    fclose(db_ptr);
    return true;
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


/*
*Return true if database exists, otherwise false.
*/
bool does_db_exist(){
    return access(DB_PATH, F_OK) == 0 ? true : false; 
}


/**
 * @param db_contents pointer to json_object struct containing db contents
 * @return an unsigned int containing the number of students in the database.
*/
size_t get_db_size(const struct json_object *db_contents){
    struct json_object *students;

    json_object_object_get_ex(db_contents, "students", &students);
    return json_object_array_length(students);
}

/**
 * Print a particular student's information to the back to the user.
 * 
 * @param student_id string containing the student's id.
 * @param first_name string containing the student's first name.
 * @param last_name string containing the student's last name.
*/
static void print_student_info(const char *student_id, const char *first_name, const char *last_name){
    const char *seperator = "=========================\n";

    printf("%s", seperator);
    printf("ID: %s\n", student_id);
    printf("first_name: %s\n", first_name);
    printf("last_name: %s\n", last_name);
    printf("%s", seperator);
}

/**
 * Return a pointer to the student with the provided student_id, otherwise NULL.
 * 
 * @param student_id The student id for the student you want to look up.
 * @param student A pointer to a struct json_object pointer where the student should be stored.
 * @param student_index A pointer to store the index of the student in the student database.
*/
static void get_student(uint32_t student_id, struct json_object **student, int *student_index){
    struct json_object *db_contents, *students, *s, *ID;
    int i;
    
    *student = NULL;
    db_contents = get_db_contents();

    if(db_contents == NULL){
        printf("Failed to get the datbase contents!\n");
        return;
    }

    // Iterate through database contents until correct
    // student_id is found
    size_t num_students = get_db_size(db_contents);

    if(!num_students){
        printf("The student database is currently empty\n");
        return;
    }

    json_object_object_get_ex(db_contents, "students", &students);

    for(i = 0; i < num_students; i++){
        s = json_object_array_get_idx(students, i);
        json_object_object_get_ex(s, "ID", &ID);
        if(json_object_get_int(ID) == student_id)
            *student_index = i;
            *student = s;
            return;
    }

    printf("Failed to find student id (%d) in the student database", student_id);
    return;
}