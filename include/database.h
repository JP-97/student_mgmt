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


int print_db_contents();

/**
* Get the contents of the student database
*
* @return A string containing the DB contents, otherwise NULL.
*/
char *get_db_contents();

#endif