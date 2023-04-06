//
// Created by David Lee on 2023-04-05.
//

#include <stdio.h>
#include <stdlib.h>
#include <ndbm.h>
#include <fcntl.h>

#define DB_NAME "file_storage_db"

void print_database_contents(DBM *db) {
    datum key, value;

    printf("Contents of the NDBM database:\n");
    for (key = dbm_firstkey(db); key.dptr != NULL; key = dbm_nextkey(db)) {
        value = dbm_fetch(db, key);
        printf("File path: %s, File size: %zu bytes\n", key.dptr, value.dsize - 1); // Subtract 1 for the null terminator
    }
}

int main() {
    DBM *db = dbm_open(DB_NAME, O_RDONLY, 0666);

    if (!db) {
        perror("Error opening the database");
        exit(EXIT_FAILURE);
    }

    print_database_contents(db);

    dbm_close(db);
    return 0;
}
