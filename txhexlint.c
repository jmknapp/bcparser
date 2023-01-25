#define MAXTXHEXRECS 100000
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    FILE *txhexfd ;
    int i, nrecs ;
    char txhexfile[MAXFILEPATH] ;
    struct txindexrecord txrecs[MAXTXHEXRECS] ;
    int nread ;
    char hex1[3], hex2[3] ;
    int lastblockfilenum, lastblocknum ;

    if (argc != 2) {
	fprintf(stderr,"usage: %s <4 char hex code>\n", argv[0]) ;
	exit(1) ;
    }

    strncpy(hex1, argv[1], 2)  ;
    strncpy(hex2, argv[1]+2, 2)  ;
    hex1[2] = '\0' ;
    hex2[2] = '\0' ;

    sprintf(txhexfile, "%s/%s/%s/%s%s.dat", TXHEXDIR, hex1, hex2, hex1, hex2) ;

    txhexfd = fopen(txhexfile, "r") ;
    if (txhexfd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n", txhexfile) ;
        exit(1) ;
    }
    fprintf(stderr, "opened txhexfile %s for reading\n", txhexfile) ;

    nrecs = 0 ;
    nread = fread(&txrecs[nrecs], sizeof(struct txindexrecord), 1, txhexfd) ;
    while (nread > 0) {
	struct transaction *tx ;
	//tx = txlookup(hashstr(txrecs[nrecs].hash)) ;
	//if (tx == NULL)
	if (tx_in_hexdb(&txrecs[nrecs]) == false) {
	    printf("%4d %5d hash %s NOT FOUND\n", txrecs[nrecs].blockfilenum, txrecs[nrecs].blocknum, hashstr(txrecs[nrecs].hash)) ;
	    exit(1) ;
	}
	//else
	    //printf("%4d %5d hash %s FOUND\n", txrecs[nrecs].blockfilenum, txrecs[nrecs].blocknum, hashstr(txrecs[nrecs].hash)) ;
	nrecs++ ;
	lastblockfilenum = txrecs[nrecs].blockfilenum ;
	lastblocknum = txrecs[nrecs].blocknum ;
        nread = fread(&txrecs[nrecs], sizeof(struct txindexrecord), 1, txhexfd) ;
	if (txrecs[nrecs].blockfilenum < lastblockfilenum || ((txrecs[nrecs].blockfilenum == lastblockfilenum) && txrecs[nrecs].blocknum < lastblocknum)) {
	    fprintf(stderr, "TXHEXLINT: %s sequence error\n", txhexfile) ;
	}
    }
}
