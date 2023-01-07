#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bitcoin.h"

int main(int argc, char **argv) {
    char buf[MAX58LEN] ;
    char key[HASHLEN+5] ;
    char key58[MAX58LEN] ;
    char csum[RIPEMD160_DIGEST_LENGTH] ;
    int keylen, n ;
    size_t key58len = MAX58LEN;
    //char *sha ;
    char *obuf ;
    char sha[1000] ;
    char *shabuf ;

    if (argc != 2) {
	fprintf(stderr, "usage %s <str>\n", argv[0]) ;
	exit(1) ;
    }

    n = str2bufrev(argv[1], buf) ;

    obuf = hash160(buf, n) ;
    memcpy(key+1, obuf, RIPEMD160_DIGEST_LENGTH) ;
    keylen = RIPEMD160_DIGEST_LENGTH ;
    key[0] = '\0' ;
    keylen++ ;
    shabuf = double_sha256(key, keylen, sha) ;
    memcpy(key+keylen, shabuf, 4) ;
    keylen += 4 ;
    b58enc(key58, &key58len, key, keylen) ;
    printf("key=%s\n", key58) ;
}
