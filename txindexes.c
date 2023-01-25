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
    char hash[65] ;
    struct block *blk ;
    int i, j, k ;
    int blockfilenum ;
    int nptrs ;
    char txindexfile[MAXFILEPATH] ;
    FILE *txindexfd ;
    MYSQL *con = mysql_init(NULL);
    MYSQL_RES *res ;
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

    sprintf(txindexfile, "%s/%04d.dat", TXDATADIR, blockfilenum) ;
    txindexfd = fopen(txindexfile, "w") ;
    if (txindexfd == NULL) {
	fprintf(stderr,"error: can't open txdatafile %s for writing\n", txindexfile) ;
	exit(1) ;
    }

    debug_print("blkdmp(): opened txindexfile %s for writing\n", txindexfile) ;
    i = 0 ;
    blk = nextblock(blockfd, blockfilenum, i) ;
    while (blk != NULL) {

	do_txindexes(blockfd, blockfilenum, i) ;
	// free the block
	nptrs = mfree() ;
	debug_print("freed %d pointers\n", nptrs) ;

	// get next block
	i++ ;
        blk = nextblock(blockfd, blockfilenum, i) ;

    }

    mysql_close(con) ;
    fclose(blockfd) ;
    fclose(txindexfd) ;
}
