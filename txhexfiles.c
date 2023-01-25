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

    if (argc != 2) {
	fprintf(stderr,"usage: %s <txhexfile path>\n", argv[0]) ;
	exit(1) ;
    }

    strcpy(txhexfile, argv[1])  ;

    txhexfd = fopen(txhexfile, "r") ;
    if (txhexfd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n", txhexfile) ;
        exit(1) ;
    }
    fprintf(stderr, "opened txhexfile %s for reading\n", txhexfile) ;

    nrecs = 0 ;
    nread = fread(&txrecs[nrecs], sizeof(struct txindexrecord), 1, txhexfd) ;
    while (nread > 0) {
	printf("%4d %5d hash %s\n", txrecs[nrecs].blockfilenum, txrecs[nrecs].blocknum, hashstr(txrecs[nrecs].hash)) ;
	nrecs++ ;
        nread = fread(&txrecs[nrecs], sizeof(struct txindexrecord), 1, txhexfd) ;
    }
}
