#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bitcoin.h"
#include <mysql.h>

int main(int argc, char **argv) {
    time_t utime, gmt0, gmt1 ;
    struct tm ts ;
    int iday ;
    MYSQL *con = mysql_init(NULL);
    char query[MAXQUERY] ;
    time_t timestamp ;
    char coinapifile[80] ;

    if (argc != 3) {
	fprintf(stderr, "usage: %s <filename> <timestamp>\n",argv[0]) ;
	exit(1) ;
    }

    strcpy(coinapifile, argv[1]) ;
    timestamp = atoi(argv[2]) ;

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, SQLHOST, SQLUSER, SQLPASS, SQLDB, 0, NULL, 0) == NULL) {
      finish_with_error(con);
    }

    sprintf(query,"insert into coinapihist (coinapifile, timestamp) VALUES (\"%s\", %lu)", coinapifile, timestamp) ;
    printf("%s\n", query) ;

    if (mysql_query(con, query)) {
      finish_with_error(con);
    }

    mysql_close(con) ;
}
