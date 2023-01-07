#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    bool rev = false ;
    bool str = true ;
    char outstring[1000] ;
    char instring[1000] ;
    char buf[1000] ;
    int n ;
    char *instr ;

    if (argc != 4) {
	fprintf(stderr, "usage %s <str|buf> <fwd|rev> <data>\n", argv[0]) ;
	exit(1) ;
    }

    strcpy(instring, argv[3]) ;

    if (strcmp(argv[1], "buf") == 0)
	str = false ;

    if (strcmp(argv[2], "rev") == 0)
	rev = true ;

    n = str2buf(instring, buf) ;

    instr = bufstr(buf, n, rev) ;

    n = str2buf(instr, buf) ;

    sha256_string(buf, n, outstring) ;

    printf("%s -> %s\n", instr, outstring) ;
}
