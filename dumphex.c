#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    FILE *txhexfd ;
    char hash[HASHLEN] ;
    int i, j, k ;
    int ntrans ;
    char txhexfile[MAXFILEPATH] ;
    struct txindexrecord txrec ;
    int nread ;
    char hex1[3], hex2[3] ;
    char *tag ;
    char buf[1000] ;
    int buflen ;

    if (argc != 2) {
	fprintf(stderr,"usage: %s <txhexfile>\n", argv[0]) ;
	exit(1) ;
    }
    sprintf(txhexfile, "%s", argv[1]) ;

    txhexfd = fopen(txhexfile, "r") ;

    if (txhexfd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n", txhexfile) ;
        exit(1) ;
    }
    fprintf(stderr, "opened txhexfile %s for reading\n", txhexfile) ;

    nread = fread(&txrec, sizeof(struct txindexrecord), 1, txhexfd) ;
    while (nread > 0) {
	fprintf(stderr, "%s %u %u %u %u\n", bufstr(txrec.hash, HASHLEN, true), txrec.blockfilenum, txrec.blocknum, txrec.txnum, txrec.offset) ;

        nread = fread(&txrec, sizeof(struct txindexrecord), 1, txhexfd) ;
    }

    fclose(txhexfd) ;
}
