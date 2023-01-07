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
    char txdatafile[MAXFILEPATH] ;
    FILE *txdatafd ;
    struct txindexrecord txrec ;
    MYSQL *con = mysql_init(NULL);
    char query[MAXQUERY] ;

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
    totalsatoshis = 0 ;
    while (blk != NULL) {
        satoshis = 0 ;
	ntrans += blk->transactionCounter ;

	//fprintf(stderr,"%d\t%d\t%u\t\"%s\"\n", blockfilenum, i, blk->blkhdr->timestamp, dtime(blk->blkhdr->timestamp)) ;

	sprintf(query,"insert into blocktimestamps (blockfilenum,blocknum,ntx,unixtime,timestamp) VALUES(%d, %d, %lu, %u,\"%s\")", blockfilenum, i, blk->transactionCounter, blk->blkhdr->timestamp, dtime(blk->blkhdr->timestamp)) ;

        //fprintf(stderr, "%s\n", query) ;
        if (mysql_query(con, query)) {
            //finish_with_error(con);
	    debug_print("block (%d,%d) already in blocktimestamps",blockfilenum, i) ;
        }

	for (j = 0 ; j < blk->transactionCounter ; j++) {
	    for (k = 0 ; k < blk->transactions[j].outcounter ; k++) {
		totalsatoshis += blk->transactions[j].xoutputs[k].satoshis ;
		satoshis += blk->transactions[j].xoutputs[k].satoshis ;
	    }
	    // write TXID record to file
            memcpy(txrec.hash, blk->transactions[j].hash, HASHLEN) ;
            txrec.blockfilenum = blk->transactions[j].blockfilenum ;
            txrec.blocknum = blk->transactions[j].blocknum ;
            txrec.txnum = blk->transactions[j].txnum ;
            txrec.offset = blk->transactions[j].offset ;
            fwrite(&txrec, sizeof(struct txindexrecord), 1, txdatafd) ;
            //fprintf(stderr, "%s %d %d %d\n", bufstr(blk->transactions[j].hash, HASHLEN, true), i, blk->transactions[j].txnum, blk->transactions[j].offset);
	}

	ts = (double)blk->blkhdr->timestamp/86400/365.25 + 1970 ;
	//fprintf(stderr,"DATA %04d %d %lu %.6f %d %lu %.6f\n", blockfilenum, i, (unsigned long)blk->blkhdr->timestamp, ts, (int)blk->transactionCounter, satoshis, (double)satoshis/1e8) ;

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
