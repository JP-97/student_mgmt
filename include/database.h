/*
* APIs for the student database.
*/

#ifndef DATABASE_H
#define DATABASE_H


/**
* Add a student to a database
*
* @return RC_SUCCESS if student was succesfully added, otherwise RC_FAILED.
*/
int add_student();

/**
* Print the student database contents
*
* @return RC_SUCCESS if database was parsed and all student info was printed, 
          otherwise return RC_FAILED.
*/
int print_db_contents();

/**
 * Initialize the student database
 * 
 * @return RC_SUCCESS if the student database was successfully initialized,
 *         otherwise RC_FAILED. Successfully initialized means that either the
 *         database already exists / is not empty, otherwise a database is generated
 *         with an empty "students" array.
*/
int init_student_database();

#endif