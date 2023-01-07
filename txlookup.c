#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include <mysql.h>
#include "bitcoin.h"
#include "ripemd160.h"

int main(int argc, char **argv) {
    int i, j;
    char *txid ;
    struct transaction tx ;
    int flags, opt ;
    char *txfile ;
    bool infile = false ;
    FILE *ifd ;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
	    txfile = optarg ;
	    infile = true ;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-f txfile] [txid]\n", argv[0]);
            exit(1);
        }
    }

    if (infile) {
        ifd = fopen(txfile, "r") ;
	if (ifd == NULL) {
	    fprintf(stderr, "error: can't open %s for reading\n", txfile) ;
	}
    }
    else
	txid = argv[optind] ;

    minit() ;
    tx = *txlookup(txid) ;
    if (&tx != NULL) {
        printtx(&tx) ;
    }
    else
        fprintf(stderr, "transaction not found\n") ;
    mfree() ;
    if (infile)
        fclose(ifd) ;
}
