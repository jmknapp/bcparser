#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include <mysql.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    FILE* txfd ;
    char txlistfile[80] ;
    struct block *blk ;
    int i, j ;
    char txhashstr[HASHSTRLEN] ;
    int nread ;
    struct transaction *tx ;

    if (argc != 2) {
	fprintf(stderr,"usage: %s <list of txs in text file>\n", argv[0]) ;
	exit(1) ;
    }

    strcpy(txlistfile, argv[1]) ;
    txfd = fopen(txlistfile, "r") ;

    if (txfd == NULL) {
        fprintf(stderr,"error: can't open %s for reading\n",txlistfile) ;
        exit(1) ;
    }

    nread = fscanf(txfd, "%s", txhashstr) ;
    while (nread != EOF) {
        //printf("%s\n", txhashstr) ;
	tx = txlookup(txhashstr) ;
	if (tx == NULL) {
	    fprintf(stderr, "error: can't find tx %s\n", txhashstr) ;
	}
        nread = fscanf(txfd, "%s", txhashstr) ;
    }

    fclose(txfd) ;
}
