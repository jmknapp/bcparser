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
    struct block *blk ;
    int i, j ;
    int blockfilenum ;
    int ntrans ;
    int nptrs ;
    char txdatafile[MAXFILEPATH] ;
    FILE *txdatafd ;

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

    sprintf(txdatafile, "%s/%04d.dat", TXDATADIR, blockfilenum) ;
    txdatafd = fopen(txdatafile, "w") ;
    if (txdatafd == NULL) {
	fprintf(stderr,"error: can't open txdatafile %s for writing\n", txdatafile) ;
	exit(1) ;
    }

    debug_print("blkdmp(): opened txdatafile %s for writing\n", txdatafile) ;
    i = 0 ;
    blk = nextblock(blockfd, blockfilenum, i) ;
    ntrans = 0 ;
    while (blk != NULL) {
	ntrans += blk->transactionCounter ;

	for (j = 0 ; j < blk->transactionCounter ; j++) {
	    printf("%s\n", hashstr(blk->transactions[j].hash)) ;
	}

	// free the block
	nptrs = mfree() ;
	debug_print("freed %d pointers\n", nptrs) ;

	// get next block
	i++ ;
        blk = nextblock(blockfd, blockfilenum, i) ;

    }

    fclose(blockfd) ;
}
