#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include <mysql.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    uint64_t satoshis, totalsatoshis ;
    uint32_t foo ;
    uint8_t foo8 ;
    uint16_t foo16 ;
    FILE* blockfd ;
    char blkfile[80] ;
    char buf[1000000] ;
    time_t utime ;
    char hash[65] ;
    struct block *blk ;
    int i, j, k ;
    int blockfilenum ;
    int ntrans ;
    int nptrs ;
    double ts ;
    double btc ;
    struct txindexrecord txrec ;
    MYSQL *con = mysql_init(NULL);
    char query[MAXQUERY] ;
    double btcprice ;
    time_t utmin, utmax ;

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "bitcoin", "P_314159_i", "crypto", 0, NULL, 0) == NULL) {
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
    ntrans = 0 ;
    totalsatoshis = 0 ;
    while (blk != NULL) {
        satoshis = 0 ;
	ntrans += blk->transactionCounter ;
	btcprice = btcquote(blk->blkhdr->timestamp) ;

	for (j = 0 ; j < blk->transactionCounter ; j++) {
	    btc = 0 ;
	    for (k = 0 ; k < blk->transactions[j].outcounter ; k++) {
		totalsatoshis += blk->transactions[j].xoutputs[k].satoshis ;
		satoshis += blk->transactions[j].xoutputs[k].satoshis ;
		btc += blk->transactions[j].xoutputs[k].satoshis ;
	    }
	    btc /= SATOSHIS2BTC ;
	    //printf("TIMESTAMP %u, BTC quote %f\n",blk->blkhdr->timestamp, btcquote(blk->blkhdr->timestamp)) ;
	    fprintf(stderr, "%u %s %.6f %.2f\n", blk->blkhdr->timestamp, bufstr(blk->transactions[j].hash, HASHLEN, true), btc, btc*btcprice) ;
	}

	// free the block
	nptrs = mfree() ;
	debug_print("freed %d pointers\n", nptrs) ;

	// get next block
	i++ ;
        blk = nextblock(blockfd, blockfilenum, i) ;

    }
    debug_print("total transactions: %d, satoshis = %lu (%.6f BTC)\n", ntrans, totalsatoshis, (double)totalsatoshis/1e8) ;

    mysql_close(con) ;
    fclose(blockfd) ;
}
