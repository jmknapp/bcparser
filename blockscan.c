#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include <mysql.h>
#include <getopt.h>
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
    struct txindexrecord txrec ;
    MYSQL *con = mysql_init(NULL);
    MYSQL_ROW row ;
    MYSQL_RES *res ;
    char query[MAXQUERY] ;
    int flags, opt ;
    char *arg ;
    bool inputscripts = false, outputscripts = false, witnessscripts  = false, combinedscripts = false;

    int c;
    int digit_optind = 0;
    int maxblockfilenum, maxblocknum ;
    int iblock ;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"inputscripts",     no_argument, 0,  0 },
            {"outputscripts",  no_argument,       0,  0 },
            {"witnessscripts",  no_argument, 0,  0 },
            {"combined",  no_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "iowc",
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            printf("option %s [%d]", long_options[option_index].name, option_index);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");

	    switch (option_index) {
		case 0:
	            inputscripts = true ;
		    break;
		case 1:
	            outputscripts = true ;
		    break;
		case 2:
	            witnessscripts = true ;
		    break;
		case 3:
	            combinedscripts = true ;
		    break;
	    }
            break;

        case 'i':
	    inputscripts = true ;
            break;

        case 'o':
	    outputscripts = true ;
            break;

        case 'w':
	    witnessscripts = true ;
            break;

        case 'c':
	    combinedscripts = true ;
            break;

        case '?':
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // get blockfilenum (last argument)
    if (argc - optind == 1) {
	blockfilenum = atoi(argv[optind]) ;
    }
    else {
	fprintf(stderr, "usage: %s -iowc <blockfilenumber>\n", argv[0]) ;
	exit(1) ;
    }

    // get number of blocks in this blockfile
    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, SQLHOST, SQLUSER, SQLPASS, SQLDB, 0, NULL, 0) == NULL) {
      finish_with_error(con);
    }

    sprintf(query, "select max(blocknum) from blocktimestamps where blockfilenum=%d", blockfilenum) ;
    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    res = mysql_store_result(con);
    if (mysql_num_rows(res) != 1) {
        fprintf(stderr, "error: got %lu rows from blocktimestamps, wanted 1\n", mysql_num_rows(res)) ;
        exit(1) ;
    }

    row = mysql_fetch_row(res) ;
    maxblocknum = atoi(row[0]) ;
    debug_print("blockfilenum %d has %d blocks\n", blockfilenum, maxblocknum) ;

    // open blockfile
    sprintf(blkfile, "%s/blk%05d.dat", BLOCKDIR, blockfilenum) ;
    blockfd = fopen(blkfile, "r") ;
    if (blockfd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n",blkfile) ;
        exit(1) ;
    }

    for (iblock = 0 ; iblock < maxblocknum ; iblock++) {
	printf("processing block %d\n", iblock) ;
        blk = nextblock(blockfd, blockfilenum, iblock) ;
        ntrans = 0 ;
        totalsatoshis = 0 ;
        if (blk != NULL) {
	    char scriptstr[10000] ;
	    struct transaction *tx = NULL ;
            satoshis = 0 ;
	    ntrans += blk->transactionCounter ;

	    //fprintf(stderr,"%d\t%d\t%u\t\"%s\"\n", blockfilenum, iblock, blk->blkhdr->timestamp, dtime(blk->blkhdr->timestamp)) ;

	    for (j = 0 ; j < blk->transactionCounter ; j++) {
		if (combinedscripts) {
	            for (k = 0 ; k < blk->transactions[j].incounter ; k++) {
		        //process_script(&blk->transactions[j].xinputs[k].script, tx, k, scriptstr) ;
		        //printf("INPUT %d %d %d %d %s\n", blockfilenum, iblock, j, k, scriptstr) ;
	                struct txoutput *txo ;
		        bool scriptresult ;
		        txo = txo_query(hashstr(blk->transactions[j].xinputs[k].prevxhash), blk->transactions[j].xinputs[k].prevxoutindex) ;
		        if (!blk->transactions[j].witnessed && txo != NULL) {
		            scriptresult = process_script(catscripts(&blk->transactions[j].xinputs[k].script, &txo->outscript), tx, k, scriptstr) ;
			    if (scriptresult)
		                printf("COMBINED %d %d %d %d %s TRUE\n", blockfilenum, iblock, j, k, scriptstr) ;
			    else
		                printf("COMBINED %d %d %d %d %s FALSE\n", blockfilenum, iblock, j, k, scriptstr) ;
			 }
		    }
		}

		if (inputscripts) {
	            for (k = 0 ; k < blk->transactions[j].incounter ; k++) {
                        if (blk->transactions[j].xinputs[k].prevxoutindex >= 0) {
		            process_script(&blk->transactions[j].xinputs[k].script, tx, k, scriptstr) ;
		            printf("INPUT %d %d %d %d %s\n", blockfilenum, iblock, j, k, scriptstr) ;
                         }
		     }
		 }

		 if (outputscripts) {
	             for (k = 0 ; k < blk->transactions[j].outcounter ; k++) {
		         process_script(&blk->transactions[j].xoutputs[k].script, tx, k, scriptstr) ;
		         printf("OUTPUT %d %d %d %d %s\n", blockfilenum, iblock, j, k, scriptstr) ;
	             }
		 }

		if (witnessscripts && blk->transactions[j].witnessed) {
	            for (k = 0 ; k < blk->transactions[j].incounter ; k++) {
                        if (blk->transactions[j].xinputs[k].prevxoutindex >= 0) {
		            process_script(&blk->transactions[j].witnesses[k].script, tx, k, scriptstr) ;
		            printf("WITNESS %d %d %d %d %s\n", blockfilenum, iblock, j, k, scriptstr) ;
                         }
		     }
		 }
	    }

	    // free the block
	    nptrs = mfree() ;
	    debug_print("freed %d pointers\n", nptrs) ;
	}
    }

    fclose(blockfd) ;
    mysql_close(con) ;
}
