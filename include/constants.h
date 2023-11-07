/*
* A file that contains common constants needed throughout
* the student database source files.
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

typedef unsigned int uint_32;

#define MAX_DB_SIZE 1024
#define DB_PATH "/home/jpoirier/Documents/student_mgmt/db.json"

enum return_codes {
    RC_FAILED,
    RC_SUCCESS
};

#endif