#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    char buf[1000] ;
    char *obuf ;
    int n ;

    if (argc != 2) {
	fprintf(stderr, "usage %s <str>\n", argv[0]) ;
	exit(1) ;
    }

    n = str2bufrev(argv[1], buf) ;

    obuf = hash160(buf, n) ;
    printf("%s\n", bufstr(obuf, RIPEMD160_DIGEST_LENGTH , false)) ;
}
