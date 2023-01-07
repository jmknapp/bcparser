#define BLOCKDIR "/hyper/.bitcoin/blocks"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    uint32_t foo ;
    uint8_t foo8 ;
    uint16_t foo16 ;
    FILE* blockfd ;
    char blkfile[80] ;
    char buf[1000000] ;
    time_t utime ;
    char hash[65] ;
    struct block *blk ;
    int i ;
    int blockfilenum ;
    int ntrans ;
    double ts ;

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
    while (blk != NULL) {
	ntrans += blk->transactionCounter ;
        free(blk->transactions) ;
	i++ ;
        blk = nextblock(blockfd, blockfilenum, i) ;
    }
    printf("total blocks: %d, total transactions: %d\n", i, ntrans) ;

    fclose(blockfd) ;
}
