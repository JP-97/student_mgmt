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

#endif