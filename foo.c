#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include <mysql.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    FILE* blockfd ;
    char blkfile[80] ;
    char buf[1000000] ;
    struct block *blk ;
    int i ;
    int blockfilenum ;
    int nptrs ;
    MYSQL *con = mysql_init(NULL);
    MYSQL_RES *res, *res2 ;
    char query[MAXQUERY] ;
    int n ;

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, SQLHOST, SQLUSER, SQLPASS, SQLDB, 0, NULL, 0) == NULL) {
      finish_with_error(con);
    }

    if (argc != 2) {
	fprintf(stderr,"usage: %s <blockfile number>\n", argv[0]) ;
	exit(1) ;
    }
    blockfilenum = atoi(argv[1]) ;
    sprintf(blkfile, "%s/blk%05d.dat", BLOCKDIR, blockfilenum) ;

    blockfd = fopen(blkfile, "r") ;

    if (blockfd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n",blkfile) ;
        exit(1) ;
    }

    i = 0 ;
    blk = nextblock(blockfd, blockfilenum, i) ;
    while (blk != NULL) {
        sprintf(query,"select timestamp from blocktimestamps where blockfilenum=%d and blocknum=%d", blockfilenum, i) ;
        if (mysql_query(con, query)) {
            finish_with_error(con);
        }
        res2 = mysql_store_result(con);
        n = mysql_num_rows(res2) ;

	if (n == 0) {
	    sprintf(query,"insert into blocktimestamps (blockfilenum,blocknum,ntx,unixtime,timestamp) VALUES(%d, %d, %lu, %u,\"%s\")", blockfilenum, i, blk->transactionCounter, blk->blkhdr->timestamp, dtime(blk->blkhdr->timestamp)) ;

            if (mysql_query(con, query)) {
	        debug_print("block (%d,%d) already in blocktimestamps",blockfilenum, i) ;
            }
	}
	else {
            sprintf(query, "update blocktimestamps set ntx=%lu where blockfilenum=%d and blocknum=%d", blk->transactionCounter, blockfilenum, i) ;
            printf("%s\n", query) ;
            if (mysql_query(con, query)) {
                finish_with_error(con);
            }
        }

	// free the block
	nptrs = mfree() ;
	debug_print("freed %d pointers\n", nptrs) ;

	// get next block
	i++ ;
        blk = nextblock(blockfd, blockfilenum, i) ;

    }

    mysql_close(con) ;
    fclose(blockfd) ;
}
