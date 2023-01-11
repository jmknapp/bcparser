#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    FILE *txdatafd, *txhexfd ;
    char hash[HASHLEN] ;
    int i, j, k ;
    int ntrans ;
    char txdatafile[MAXFILEPATH] ;
    char txhexfile[MAXFILEPATH] ;
    struct txindexrecord txrec ;
    int txdatafilenumber ;
    int nread ;
    char hex1[3], hex2[3] ;
    char *tag ;
    char buf[1000] ;
    int buflen ;
    char txhashstr[HASHSTRLEN] ;

    if (argc != 2) {
	fprintf(stderr,"usage: %s <txdatafile number>\n", argv[0]) ;
	exit(1) ;
    }
    txdatafilenumber = atoi(argv[1]) ;
    sprintf(txdatafile, "%s/%04d.dat", TXDATADIR, txdatafilenumber) ;

    txdatafd = fopen(txdatafile, "r") ;

    if (txdatafd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n", txdatafile) ;
        exit(1) ;
    }
    fprintf(stderr, "opened txdatafile %s for reading\n", txdatafile) ;

    nread = fread(&txrec, sizeof(struct txindexrecord), 1, txdatafd) ;
    while (nread > 0) {
	strcpy(txhashstr, bufstr(txrec.hash, HASHLEN, true)) ;

	// only process this tx if not already in the database
	if (tx_in_hexdb(&txrec) == false) {
	    debug_print("hash: %s FALSE\n", txhashstr) ;

	    tag = bufstr(txrec.hash+31, 1, false);
	    strcpy(hex1,tag) ;
	    tag = bufstr(txrec.hash+30, 1, false);
	    strcpy(hex2,tag) ;
            sprintf(txhexfile, "%s/%s/%s/%s%s.dat", TXHEXDIR, hex1, hex2, hex1, hex2) ;
	    debug_print("%s %s %s\n", bufstr(txrec.hash, HASHLEN, true), hex1, hex2) ;
	    debug_print("HEX %s\n", txhexfile) ;

            txhexfd = fopen(txhexfile, "a") ;
            if (txhexfd == NULL) {
                fprintf(stderr,"error: can't open %s for writing\n", txhexfile) ;
                exit(1) ;
            }
            debug_print("opened txhexfile %s for writing\n", txhexfile) ;
	    fwrite(&txrec, sizeof(struct txindexrecord), 1, txhexfd) ;
	    fclose(txhexfd) ;
	}

        nread = fread(&txrec, sizeof(struct txindexrecord), 1, txdatafd) ;
    }

    fclose(txdatafd) ;
}
