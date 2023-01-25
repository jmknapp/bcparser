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
    int i, j, nrecs ;
    char txhexfile[MAXFILEPATH] ;
    struct txindexrecord *txrecs ;
    int nread ;
    long hexlen ;

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

    fseek(txhexfd,0,SEEK_END) ;
    hexlen = ftell(txhexfd) ;
    fseek(txhexfd,0,SEEK_SET) ;
    nrecs = hexlen/sizeof(struct txindexrecord) ;

    txrecs = malloc(hexlen) ;

    j = 0 ;
    nread = fread(&txrecs[j], sizeof(struct txindexrecord), 1, txhexfd) ;
    while (nread > 0) {
	//fprintf(stderr,"%s\n", hashstr(txrecs[nrecs].hash)) ;
	j++ ;
        nread = fread(&txrecs[j], sizeof(struct txindexrecord), 1, txhexfd) ;
    }

#if 1
    txhexfd = fopen(txhexfile, "w") ;
    if (txhexfd == NULL) {
        fprintf(stderr,"error: can't open %s for writing\n", txhexfile) ;
        exit(1) ;
    }
    fprintf(stderr, "opened txhexfile %s for writing\n", txhexfile) ;

    for (i = nrecs-1 ; i >= 0 ; i--)
        fwrite(&txrecs[i], sizeof(struct txindexrecord), 1, txhexfd) ;

    fclose(txhexfd) ;
    fprintf(stderr,"wrote %s\n", txhexfile) ;
    free(txrecs) ;
#endif
}
