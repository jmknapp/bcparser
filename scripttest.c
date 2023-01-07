#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <byteswap.h>
#include <time.h>
#include <mysql.h>
#include "bitcoin.h"
#include "ripemd160.h"
#include "script.h"

int main(int argc, char **argv) {
    int i, j;
    char txid[HASHSTRLEN] ;
    struct transaction tx ;
    struct scriptstackitem item[20] ;
    struct xscript script ;
    struct scriptstackitem n ;

    if (argc != 1) {
	fprintf(stderr,"usage: %s <txid>\n", argv[0]) ;
	exit(1) ;
    }

    minit() ;
    sinit() ;
    script.code = malloc(MAXSCRIPT) ;

    item[0].len = 1 ;
    item[1].len = 1 ;
    item[2].len = 1 ;
    item[3].len = 1 ;
    item[4].len = 1 ;
    item[5].len = 1 ;
    item[0].data[0] = 1 ;
    item[1].data[0] = 2 ;
    item[2].data[0] = 3 ;
    item[3].data[0] = 4 ;
    item[4].data[0] = 5 ;
    item[5].data[0] = 6 ;

    n.len = 1 ;
    n.data[0] = 3 ;

    spush(&item[0]) ;
    spush(&item[1]) ;
    spush(&item[2]) ;
    spush(&item[3]) ;
    spush(&item[4]) ;
    spush(&item[5]) ;
    spush(&n) ;

    script.len = 1 ;
    script.code[0] = OP_ROLL ;

    process_script(&script, (struct transaction *)NULL, 0, (char *)NULL) ;
    printf("RESULT:\n") ;
    print_scriptstack() ;
    mfree() ;
}
