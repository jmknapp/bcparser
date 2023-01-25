#include "bitcoin.h"
#include "bcsqllib.h"
#include "mysql.h"
#include "script.h"
#include "ripemd160.h"

uint8_t hexchar2int(char hexchar) {
    switch(hexchar) {
	case '0':
	    return 0 ;
	    break;
	case '1':
	    return 1 ;
	    break;
	case '2':
	    return 2 ;
	    break;
	case '3':
	    return 3 ;
	    break;
	case '4':
	    return 4 ;
	    break;
	case '5':
	    return 5 ;
	    break;
	case '6':
	    return 6 ;
	    break;
	case '7':
	    return 7 ;
	    break;
	case '8':
	    return 8 ;
	    break;
	case '9':
	    return 9 ;
	    break;
	case 'a':
	    return 10 ;
	    break;
	case 'b':
	    return 11 ;
	    break;
	case 'c':
	    return 12 ;
	    break;
	case 'd':
	    return 13 ;
	    break;
	case 'e':
	    return 14 ;
	    break;
	case 'f':
	    return 15 ;
	    break;
    }
}

int str2buf(char *str, char *buf) {
    int i, j ;
    int len ;
    unsigned char nib1 ;
    unsigned char nib2 ;

    len = strlen(str) ;
    if (len % 2 != 0) {
	fprintf(stderr, "error: string parameter to str2buf() mush have an even number of hex digits\n") ;
	exit(1) ;
    }

    j = 0 ;
    i = len - 2 ;
    while (i >= 0) {
	nib1 = hexchar2int(str[i]) ;
	nib2 = hexchar2int(str[i+1]) ;
	buf[j] = nib1 << 4 | nib2 ;
	i -= 2 ;
	j++ ;
    }
    return j ;
}

int str2bufrev(char *str, char *buf) {
    int i, j ;
    int len ;
    unsigned char nib1 ;
    unsigned char nib2 ;

    len = strlen(str) ;
    if (len % 2 != 0) {
	fprintf(stderr, "error: string parameter to str2buf() mush have an even number of hex digits\n") ;
	exit(1) ;
    }

    i = 0 ;
    j = 0 ;
    while (i < len) {
	nib1 = hexchar2int(str[i]) ;
	nib2 = hexchar2int(str[i+1]) ;
	buf[j] = nib1 << 4 | nib2 ;
	i += 2 ;
	j++ ;
    }
    return j ;
}

char *bufstr(char *buf, int len, bool reverse) {
    static char bufstring[MAXBUFSTR] ;
    int i ;
    char bufbytestring[3] ;

    bufstring[0] = '\0' ;
    for (i = 0 ; i < len ; i++) {
	if (reverse)
	    sprintf(bufbytestring, "%02x", buf[len - i - 1] & 0xff) ;
        else
	    sprintf(bufbytestring, "%02x", buf[i] & 0xff) ;

	strcat(bufstring,bufbytestring) ; 
    }
    bufstring[2*len+1] = '\0' ;
    return bufstring ;
}

char *hashstr(char *hash) {
    return bufstr(hash, HASHLEN, true) ;
}

void printhash(char *h) {
    int i ;

    for (i = 0 ; i < HASHLEN ; i++)
        printf("%02x",h[i] & 0xff) ;
    printf("\n") ;
}

void printhashrev(char *h) {
    int i ;

    for (i = HASHLEN-1 ; i >= 0 ; i--)
        printf("%02x",h[i] & 0xff) ;
    printf("\n") ;
}

uint64_t varint(FILE *ifd, int *nbytes) {
    uint64_t i64 ;
    uint32_t i32 ;
    uint16_t i16 ;
    uint8_t i8 ;

    fread(&i8, 1, 1, ifd) ;

    if (i8 < 0xfd) {
        *nbytes = 1 ;
        return((uint64_t)i8) ;
    }
    else if (i8 == 0xfd) {
        *nbytes = 3 ;
        fread(&i16, 2, 1, ifd) ;
        return((uint64_t)i16) ;
    }
    else if (i8 == 0xfe) {
        *nbytes = 5  ;
        fread(&i32, 4, 1, ifd) ;
        return((uint64_t)i32) ;
    }
    else if (i8 == 0xff) {
        *nbytes = 9 ;
        fread(&i64, 8, 1, ifd) ;
        return((uint64_t)i64) ;
    }

    *nbytes = 0 ;
    return((uint64_t)0) ;
}

void sha256_string(char *buf, int len, char outputBuffer[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buf, len);
    SHA256_Final(hash, &sha256);
    int i ;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
        //sprintf(outputBuffer + (i * 2), "%02x", hash[SHA256_DIGEST_LENGTH - i - 1]);
    }
    outputBuffer[64] = 0;
}

char *double_sha256_string(char *string, int len, char outputBuffer[65], bool reverse) {
    static unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, len);
    SHA256_Final(hash, &sha256);

    int i ;

    debug_print("double_sha256_string=%s (length %d)\n", string, len) ;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
	if (reverse)
            sprintf(outputBuffer + (i * 2), "%02x", hash[SHA256_DIGEST_LENGTH - i - 1]);
	else
            sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    debug_print("hash #1: %s\n", outputBuffer) ;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, outputBuffer, HASHSTRLEN);
    SHA256_Final(hash, &sha256);
    //outputBuffer[64] = 0;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
	if (reverse)
            sprintf(outputBuffer + (i * 2), "%02x", hash[SHA256_DIGEST_LENGTH - i - 1]);
	else
            sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    return hash ;
}

char *double_sha256(char *buf, int len, char outputBuffer[65]) {
    int i ;
    static unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buf, len);
    SHA256_Final(hash, &sha256);

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, hash, HASHLEN);
    SHA256_Final(hash, &sha256);

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", hash[SHA256_DIGEST_LENGTH - i - 1]);

    return hash ;
}

char *sha256(char *buf, int len) {
    int i ;
    static unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buf, len);
    SHA256_Final(hash, &sha256);

    return hash ;
}

struct blockheader *getblockheader(FILE *blockfd) {
    static struct blockheader blkhdr;
    int nread = 0 ;

    nread += fread(&blkhdr.version, 4, 1, blockfd) ;
    nread += fread(&blkhdr.hashPrevBlock, 32, 1, blockfd) ;
    nread += fread(&blkhdr.hashMerkleRoot, 32, 1, blockfd) ;
    nread += fread(&blkhdr.timestamp, 4, 1, blockfd) ;
    nread += fread(&blkhdr.bits, 4, 1, blockfd) ;
    nread += fread(&blkhdr.nonce, 4, 1, blockfd) ;

    if (nread == 6) 
        return &blkhdr ;
    else {
	debug_print("error: getblockheader(), wanted %d elements, got %d\n", 6, nread) ;
        return (struct blockheader *) NULL ;
    }
}

// return next block or NULL if no more data or error
struct block *nextblock(FILE *blockfd, int blockfilenum, int blocknum) {
    static struct block blk ;
    static uint32_t blockcounter = 0 ;
    int nbytes ;
    int nread ;
    int i, j, k ;
    uint64_t i64 ;
    uint32_t i32 ;
    uint16_t i16 ;
    uint8_t i8 ;
    uint64_t  nwitnesselements ;
    uint64_t  witnesslen ;
    char *sx ;
    int ninbytes ;
    int ntxbytes ;
    char buf[3000000] ;
    char txstring[3000000] ;
    long tx0 ;
    long segwit0 ;
    long segwit1 ;
    long currbyte ;
    char txid[65] ;

    blockcounter++ ;   // increment block counter

    // first uint32 should be the magic number
    nread = fread(&blk.magic, 4, 1, blockfd) ;
    if (nread == 0) {
	// no more data
        return (struct block *)NULL ;
    }
    if (blk.magic != MAGIC) {
        debug_print("block %d, error: bad magic\n", blockcounter) ;
        return (struct block *)NULL ;
    }

    // blocksize is the number of bytes following up to the end of the block
    fread(&blk.blocksize, 4, 1, blockfd) ;

    // read header
    blk.blkhdr = getblockheader(blockfd) ;
    if (blk.blkhdr == NULL) {
        debug_print("block %d, error: couldn't get blockheader\n", blockcounter) ;
        return (struct block *)NULL ;
    }

    // get number of transactions in this block
    blk.transactionCounter = varint(blockfd, &nbytes) ;
    debug_print("block %d: %lu transactions\n", blockcounter, blk.transactionCounter) ;
    blk.transactions = malloc(blk.transactionCounter * sizeof(struct transaction)) ;
    if (blk.transactions == NULL) {
	debug_print("error: could not malloc transactions for block %d\n", blockcounter) ;
        return (struct block *)NULL ;
    }
    mpush(blk.transactions) ;

    // process all transactions
    for (i = 0 ; i < blk.transactionCounter ; i++) {
	blk.transactions[i].blockfilenum = blockfilenum ;
	blk.transactions[i].txnum = i ;
	blk.transactions[i].blocknum = blocknum ;
	debug_print("blockfilenum %d, txnum=%d\n", blockfilenum, i) ;
	if (!nexttransaction(&blk.transactions[i],blockfd)) {
	    fprintf(stderr, "error: premature end of tranactions\n") ;
            exit(1) ;
	}
    }

    return &blk ;
}

void printblockinfo(struct block *blk) {
    time_t utime ;
    char hash[65] ;

    printf("magic=%04x\n",blk->magic) ;
    printf("blocksize=%d\n",blk->blocksize) ;
    printf("version=%x\n",blk->blkhdr->version) ;
    printf("hashPrevBlock: ") ;
    printhash(blk->blkhdr->hashPrevBlock) ;
    printhashrev(blk->blkhdr->hashPrevBlock) ;

    printf("hashMerkleRoot: ") ;
    printhash(blk->blkhdr->hashMerkleRoot) ;

    utime = (time_t)blk->blkhdr->timestamp ;
    printf("timestamp=%s",ctime(&utime)) ;

    printf("bits: %u\n", blk->blkhdr->bits) ;
    printf("nonce: %u\n", blk->blkhdr->nonce) ;

    printf("transactionCounter: %lu\n", blk->transactionCounter) ;

    sha256_string((char *)blk->blkhdr, sizeof(struct blockheader), hash) ;
    printf("hashBlkhdr=%s\n", hash) ;
    printf("==================================================================\n") ;
}

void hexdump(FILE *ifd, int len) {
    int i ;
    char buf[10000] ;

    fread(buf, 1, len, ifd) ;
    fseek(ifd, -len, SEEK_CUR) ;

    fprintf(stderr, "HEXDUMP:\n") ;
    for (i = 0 ; i < len ; i++) {
	if (i == 0) {
	    fprintf(stderr, "%02x ", buf[i] & 0xff) ;
	}
	else {
	    fprintf(stderr, "%02x ", buf[i] & 0xff) ;
	}
	if (i > 0 && i % 10 == 0)
	    fprintf(stderr,"\n") ;
    }
    fprintf(stderr,"\n") ;
}

void witnessdump(FILE *ifd, int len) {
    int i, j ;
    char buf[10000] ;
    int nbytes ;

    fread(buf, 1, len, ifd) ;
    fseek(ifd, -len, SEEK_CUR) ;

    fprintf(stderr, "WITNESSDUMP:\n") ;
    i = 0 ;
    while (i < len) {
	nbytes = buf[i] ;
        COLOR(GRN) ;
	fprintf(stderr, "%d bytes\n", nbytes & 0xff) ;
        COLOR(reset) ;
	i++ ;
	if (i < len) {
            for (j = 0 ; j < nbytes ; j++) {
	        if (j > 0 && (j % 10) == 0)
	            fprintf(stderr,"\n") ;
                COLOR(RED) ;
	        fprintf(stderr, "%02x ", buf[i] & 0xff) ;
                COLOR(reset) ;
	        i++ ;
	    }
	}
	fprintf(stderr, "\n") ;
    }
    fprintf(stderr,"\n") ;
}

void mpush(void *p) {
    mstack.fptr++ ;
    mstack.ptrs[mstack.fptr] = p ;
    if (mstack.fptr > MAXMALLOC) {
	fprintf(stderr, "error: mallocstack overflow: %d\n", mstack.fptr) ;
	exit(1) ;
    }
}

bool mpop() {
    if (mstack.fptr < 0)
        return false ; 
    free(mstack.ptrs[mstack.fptr]) ;
    mstack.fptr-- ;
    return true ;
}

int mfree() {
    int i = 0 ;
    if (mstack.fptr >= 0) {
        printf("freeing %d mallocs\n", mstack.fptr) ; 
        while(mpop()) 
            i++ ;
    }
    mstack.savefptr = -1 ;
    return i ;
}

// free mem stack items with index >= ptr
int mrollback() {
    int i = 0 ;
    while(mstack.fptr > mstack.savefptr) { 
	mpop() ;
        i++ ;
    }
    debug_print("MROLLBACK: freed %d items\n", i) ;
    return i ;
}

void minit() {
    mstack.savefptr = -1 ;
    mstack.fptr = -1 ;
}

void mmark() {
    mstack.savefptr = mstack.fptr ;
}

void sinit() {
    sstack.fptr = -1 ;
    altsstack.fptr = -1 ;
}

void spush(struct scriptstackitem *item) {
    debug_print("SPUSH: fptr = %d\n", sstack.fptr) ;
    sstack.fptr++ ;
    sstack.item[sstack.fptr] = *item ;
    if (sstack.fptr > MAXSCRIPTITEMS) {
	fprintf(stderr, "error: scriptstack overflow\n") ;
	exit(1) ;
    }
    if (DEBUG)
      print_scriptstack() ;
}

int sfree() {
    int i = 0 ;
    sstack.fptr = -1 ;
    //struct scriptstackitem item ;
    //while(spop(&item)) 
        //i++ ;
    return i ;
}

void altspush(struct scriptstackitem *item) {
    debug_print("ALTSPUSH: fptr = %d\n", altsstack.fptr) ;
    altsstack.fptr++ ;
    altsstack.item[altsstack.fptr] = *item ;
    if (altsstack.fptr > MAXSCRIPTITEMS) {
	fprintf(stderr, "error: scriptstack overflow\n") ;
	exit(1) ;
    }
    if (DEBUG)
        print_altscriptstack() ;
}

bool spop(struct scriptstackitem *item) {
    debug_print("SPOP: fptr = %d\n", sstack.fptr) ;
    if (sstack.fptr < 0)
        return false ; 
    memcpy(item, &sstack.item[sstack.fptr], sizeof(struct scriptstackitem)) ;
    sstack.fptr-- ;
    if (DEBUG)
        print_scriptstack() ;
    return true ;
}

bool altspop(struct scriptstackitem *item) {
    debug_print("SPOP: fptr = %d\n", altsstack.fptr) ;
    if (altsstack.fptr < 0)
        return false ; 
    memcpy(item, &altsstack.item[altsstack.fptr], sizeof(struct scriptstackitem)) ;
    altsstack.fptr-- ;
    print_altscriptstack() ;
    return true ;
}

bool slook(struct scriptstackitem *item, int n) {
    debug_print("SGRAB: %d, fptr = %d\n", n, sstack.fptr) ;
    if (sstack.fptr < n)
        return false ; 

    memcpy(item, &sstack.item[sstack.fptr-n], sizeof(struct scriptstackitem)) ;
    print_scriptstack() ;
    return true ;
}

char *varintstr(uint64_t vi) {
    static char vistr[20] ;
    if (vi < 0xfd)
	sprintf(vistr, "%02x", (uint8_t)vi) ;
    else if (vi < 0xffff) 
	sprintf(vistr, "fd%04x", (uint16_t)vi) ;
    else if (vi < 0xffffffff) 
	sprintf(vistr, "fe%08x", (uint32_t)vi) ;
    else
	sprintf(vistr, "fe%016lx", (uint64_t)vi) ;

    return vistr ;
}

// return false if no more transactions
bool nexttransaction(struct transaction *tx, FILE *blockfd) {
    int tx0 ;
    int segwit0 ;
    int segwit1 ;
    int nbytes ;
    int i, j, k ;
    uint16_t i16 ;
    int ninbytes ;
    uint64_t nwitnesselements ;
    int witnesslen ;
    long currbyte ;
    int ntxbytes ;
    char buf[3000000] ;
    char txid[65] ;

    tx0 = ftell(blockfd) ;

    segwit0 = 0 ;
    segwit1 = 0 ;

    // get transaction version
    fread(&tx->version, 4, 1, blockfd) ; 
    debug_print("transaction version=%d\n", tx->version) ;

    // check for presence of witness flag (next two bytes 0x0 and 0x1)
    fread(&i16, 2, 1, blockfd) ;
    if (i16 == 0x0100) {
        tx->witnessed = true ;
        debug_print("transaction witnessed flag 0x%x\n", i16) ;
    }
    else {
        fseek(blockfd, -2, SEEK_CUR);  // no witness flag, so put the bytes back
        tx->witnessed = false ;  
        debug_print("transaction witnessed flag 0x%x\n", i16) ;
    }

    // get number of inputs for this transaction & allocate space for them
    tx->incounter = varint(blockfd, &nbytes) ;
    if (tx->incounter > MAXINPUTS) {
        fprintf(stderr, "error: number of inputs %lu exceeds maximum %d\n", tx->incounter, MAXINPUTS) ;
        exit(1) ;
    }
    tx->xinputs = malloc(tx->incounter * sizeof(struct transinputs)) ;
    if (tx->xinputs == NULL) {
        fprintf(stderr, "error: could not malloc inputs for transaction \n") ;
        exit(1) ;
    }
    mpush(tx->xinputs) ;
    debug_print("transaction has %lu inputs\n", tx->incounter) ;

    // process each input for this transaction
    debug_print("reading %lu transaction inputs\n", tx->incounter) ;
    for (j = 0 ; j < tx->incounter ; j++) {
        fread(&tx->xinputs[j].prevxhash, 32, 1, blockfd) ;
        debug_print("tx input %d prevxhash=%s\n",j,hashstr(tx->xinputs[j].prevxhash)) ;
        fread(&tx->xinputs[j].prevxoutindex, 4, 1, blockfd) ;
        debug_print("tx input %d prev index=%d\n",j,tx->xinputs[j].prevxoutindex) ;
        tx->xinputs[j].xscriptlen = varint(blockfd, &nbytes) ;
        debug_print("tx input %d script length=%lu\n",j,tx->xinputs[j].xscriptlen) ;
        tx->xinputs[j].script.code = malloc(tx->xinputs[j].xscriptlen) ;
        tx->xinputs[j].script.len = tx->xinputs[j].xscriptlen ;

        if (tx->xinputs[j].script.code == NULL) {
            debug_print("error: could not malloc input %d script for transaction\n", j) ;
            exit(1) ;
        }
        mpush(tx->xinputs[j].script.code) ;
        fread(tx->xinputs[j].script.code, tx->xinputs[j].xscriptlen, 1, blockfd) ;

        fread(&tx->xinputs[j].seqnum, 4, 1, blockfd) ;
        debug_print("tx input %d sequence number=0x%x\n",j,tx->xinputs[j].seqnum) ;
    }

    // get number of outputs for this transaction & allocate space for them
    tx->outcounter = varint(blockfd, &nbytes) ;
    tx->xoutputs = malloc(tx->outcounter * sizeof(struct transoutputs)) ;
    if (tx->xoutputs == NULL) {
        fprintf(stderr, "error: could not malloc outputs for transaction\n") ;
        exit(1) ;
    }
    mpush(tx->xoutputs) ;
    debug_print("tx has %lu outputs\n", tx->outcounter) ;

    debug_print("reading %lu tx outputs\n", tx->outcounter) ;
    for (j = 0 ; j < tx->outcounter ; j++) {
        fread(&tx->xoutputs[j].satoshis, 8, 1, blockfd) ;
        debug_print("output %d satoshis=%lu (%.6f BTC)\n",j, tx->xoutputs[j].satoshis, (double)tx->xoutputs[j].satoshis/SATOSHIS2BTC) ;
        tx->xoutputs[j].xscriptlen = varint(blockfd, &nbytes) ;
        debug_print("output %d script length=%lu\n",j,tx->xoutputs[j].xscriptlen) ;
        tx->xoutputs[j].script.code = malloc(tx->xoutputs[j].xscriptlen) ;
        tx->xoutputs[j].script.len = tx->xoutputs[j].xscriptlen ;
        if (tx->xoutputs[j].script.code == NULL) {
            debug_print("error: could not malloc output %d script\n", j) ;
            exit(1) ;
        }
        mpush(tx->xoutputs[j].script.code) ;
        fread(tx->xoutputs[j].script.code, tx->xoutputs[j].xscriptlen, 1, blockfd) ;
    }

    if (tx->witnessed) {
        segwit0 = ftell(blockfd) ;

        debug_print("reading %lu witnesses\n", tx->incounter) ;
        for (j = 0 ; j < tx->incounter ; j++) {
            ninbytes = 0 ;
            nwitnesselements = varint(blockfd, &nbytes) ;
            ninbytes += nbytes ;
            debug_print("witness %d has %lu elements\n", j, nwitnesselements) ;
            for (k = 0 ; k < nwitnesselements ; k++) {
                witnesslen = varint(blockfd, &nbytes) ;
		ninbytes += nbytes ;
                fseek(blockfd, witnesslen, SEEK_CUR) ;
                ninbytes += witnesslen ;
            }
            debug_print("witness %d: read %d bytes of witness data\n", j,ninbytes) ;
            fseek(blockfd, -ninbytes, SEEK_CUR) ;
            tx->witnesses[j].script.code = malloc(ninbytes+nbytes) ;
            if (tx->witnesses[j].script.code == NULL) {
                fprintf(stderr, "error: could not malloc code for witness %d\n",j) ;
                exit(1) ;
            }
            mpush(tx->witnesses[j].script.code) ;
            fread(tx->witnesses[j].script.code, ninbytes, 1, blockfd) ;
            tx->witnesses[j].script.len = ninbytes ;
        }
        segwit1 = ftell(blockfd) ;
    }

    fread(&tx->lock_time, 4, 1, blockfd) ;
    debug_print("lock time=%x\n",tx->lock_time) ;

    //blk.transactions[i].lock_time = bswap_32(blk.transactions[i].lock_time) ;

    currbyte = ftell(blockfd) ;
    if (segwit0 == 0) {
        ntxbytes = ftell(blockfd) - tx0 ;
        fseek(blockfd, -ntxbytes, SEEK_CUR) ;
        fread(buf, ntxbytes, 1, blockfd) ;
    }
    else {
        ntxbytes = ftell(blockfd) - tx0 ;
        fseek(blockfd, -ntxbytes, SEEK_CUR) ;
        fread(buf, ntxbytes, 1, blockfd) ;
        //fprintf(stderr, "WBUF1=%s\n\n", bufstr(buf, ntxbytes, false)) ;

        fseek(blockfd, tx0, SEEK_SET) ;
        ntxbytes = segwit0 - tx0 ;
        fread(buf, 4, 1, blockfd) ;
        fseek(blockfd, 2, SEEK_CUR) ;
        fread(buf+4, ntxbytes-6, 1, blockfd) ;
        //fprintf(stderr, "WBUF2=%s\n\n", bufstr(buf, ntxbytes-2, false)) ;

        fseek(blockfd, segwit1, SEEK_SET) ;
        fread(buf+ntxbytes-2, currbyte - segwit1, 1, blockfd) ;
        ntxbytes += currbyte-segwit1 ;
        //fprintf(stderr, "WBUF3=%s\n\n", bufstr(buf, ntxbytes-2, false)) ;
        ntxbytes -= 2 ;
    }

    memcpy(tx->hash, double_sha256(buf, ntxbytes, txid), HASHLEN) ;
    tx->offset = tx0 ;
    debug_print("TXHASH %s\n",  bufstr(tx->hash, HASHLEN, true)) ;
    debug_print("==========================%s=================================\n", "END TX") ;
    return true ;
}

struct transaction *txlookup(char *hashstring) {
    char hash[HASHLEN] ;
    char hex1[3], hex2[3] ;
    char *tag ;
    char txhexfile[MAXFILEPATH] ;
    char blockfile[MAXFILEPATH] ;
    FILE *hexfd, *datafd ;
    bool found = false ;
    int nread ;
    struct txindexrecord txrec ;
    static struct transaction tx ;
    MYSQL *con ;
    char query[MAXQUERY] ;

    con = mysql_init(NULL) ;
    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    debug_print("TXLOOKUP %s\n", hashstring) ;

    if (mysql_real_connect(con, SQLHOST, SQLUSER, SQLPASS, SQLDB, 0, NULL, 0) == NULL) {
      finish_with_error(con);
    }

    str2buf(hashstring, hash) ;

    tag = bufstr(hash+31, 1, false);
    strcpy(hex1,tag) ;
    tag = bufstr(hash+30, 1, false);
    strcpy(hex2,tag) ;
    sprintf(txhexfile, "%s/%s/%s/%s%s.dat", TXHEXDIR, hex1, hex2, hex1, hex2) ;

    hexfd = fopen(txhexfile, "r") ;
    if (hexfd == NULL) {
	fprintf(stderr, "error: can't open %s for reading\n", txhexfile) ;
	exit(1) ;
    }
    //fprintf(stderr, "opened txhex index file %s\n", txhexfile) ;

    nread = fread(&txrec, sizeof(struct txindexrecord), 1, hexfd) ;
    while (nread > 0 && !found) {
	if (memcmp(hash, txrec.hash, HASHLEN) == 0) {
	    found = true ;
	}
	else
            nread = fread(&txrec, sizeof(struct txindexrecord), 1, hexfd) ;
    }

    if (!found) {
	//fprintf(stderr, "error: couldn't find tx %s\n", hashstring) ;
	fclose(hexfd) ;
	mysql_close(con) ;
	return((struct transaction *)NULL) ;
    }

    sprintf(blockfile, "%s/blk%05d.dat", BLOCKDIR, txrec.blockfilenum) ;
    debug_print("TXLOOKUP: blockfile %s, offset %d\n", blockfile, txrec.offset) ;

    datafd = fopen(blockfile, "r") ;
    if (datafd == NULL) {
	fprintf(stderr, "error: can't open %s for reading\n", blockfile) ;
	exit(1) ;
    }
    debug_print("txlookup(): opened %s for reading\n", blockfile) ;

    fseek(datafd, txrec.offset, SEEK_SET) ;

    nexttransaction(&tx, datafd) ;
    tx.blockfilenum = txrec.blockfilenum ;
    tx.blocknum = txrec.blocknum ;
    tx.txnum = txrec.txnum ;

    sprintf(query,"SELECT timestamp FROM %s.blocktimestamps where blockfilenum=%d and blocknum=%d", SQLDB, txrec.blockfilenum, txrec.blocknum) ;
    debug_print("%s\n", query) ;
    if (mysql_query(con, query)) {
      finish_with_error(con);
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        finish_with_error(con);
    }

    int num_fields = mysql_num_fields(result);
    int num_rows = mysql_num_rows(result);

    MYSQL_ROW row;

    if (num_fields != 1 || num_rows != 1) {
	fprintf(stderr, "error: tx timestamp query returned multiple values\n") ;
	exit(1) ;
    }
    row = mysql_fetch_row(result) ;
    sprintf(tx.timestamp, "%s", row[0]);

    fclose(hexfd) ;
    fclose(datafd) ;
    mysql_close(con) ;
    return(&tx) ;
}

bool tx_in_hexdb(struct txindexrecord *tx) {
    char hex1[3], hex2[3] ;
    char *tag ;
    char txhexfile[MAXFILEPATH] ;
    FILE *hexfd ;
    int nread ;
    struct txindexrecord txrec ;
    int hexsize ;
    int nrecs ;
    int i ;
    long offset ;

    //return false ;

    tag = bufstr(tx->hash+31, 1, false);
    strcpy(hex1,tag) ;
    tag = bufstr(tx->hash+30, 1, false);
    strcpy(hex2,tag) ;
    sprintf(txhexfile, "%s/%s/%s/%s%s.dat", TXHEXDIR, hex1, hex2, hex1, hex2) ;

    hexfd = fopen(txhexfile, "r") ;
    if (hexfd == NULL) {
	//fprintf(stderr, "error: can't open %s for reading\n", txhexfile) ;
	return false ;
    }

    fseek(hexfd, 0, SEEK_END) ;
    hexsize = ftell(hexfd) ;
    if (hexsize % sizeof(struct txindexrecord) != 0) {
	fprintf(stderr, "TXPEEK: error: hex db file size %d not an even number of txindexrecords\n", hexsize) ;
	exit(1) ;
    }
    nrecs = hexsize/sizeof(struct txindexrecord) ;
    //debug_print("hexdb file %s has %d recs\n", txhexfile, nrecs) ;

    offset = hextell(tx, hexfd) ;
    fseek(hexfd, offset, SEEK_SET) ;
    nread = fread(&txrec, sizeof(struct txindexrecord), 1, hexfd) ;
    debug_print("at OFFSET %lu GOT blockfilenum=%d, blocknum=%d, hash=%s\n", offset, txrec.blockfilenum, txrec.blocknum, hashstr(txrec.hash)) ;
    while (nread > 0 && txrec.blockfilenum <= tx->blockfilenum) {
	debug_print("wanted %u %u, have %u %u\n", txrec.blockfilenum, txrec.blocknum, tx->blockfilenum, tx->blocknum) ;
	if (memcmp(tx->hash, txrec.hash, HASHLEN) == 0) {
            fclose(hexfd) ;
	    return true ;
	}
	else
            nread = fread(&txrec, sizeof(struct txindexrecord), 1, hexfd) ;
    }

    fclose(hexfd) ;
    return false ;
}

void printtx(struct transaction *tx) {
    int i ;
    //struct txoutput *txo ;
    uint64_t totalsatoshisout ;
    uint64_t totalsatoshisin ;
    uint64_t fee ;
    bool scriptresult ;
    struct transaction *txo ;
    char script_pattern[100000] ;

    sinit() ;
    printf("TXID %s\n", bufstr(tx->hash, HASHLEN, true)) ;
    printf("tx version: %d\n", tx->version) ;
    printf("ninputs %lu, noutputs %lu\n", tx->incounter, tx->outcounter) ;

    totalsatoshisin = 0 ;
    printf("\n") ;
    printf("inputs:\n") ;

    for (i = 0 ; i < tx->incounter ; i++) {
	printf("%d: prevxhash %s\n", i, hashstr(tx->xinputs[i].prevxhash)) ;
	printf("%d: prevoutindex %d\n", i, tx->xinputs[i].prevxoutindex) ;

	if (tx->xinputs[i].prevxoutindex >= 0) {
	    int outindex = tx->xinputs[i].prevxoutindex ;
	    //txo = txo_query(hashstr(tx->xinputs[i].prevxhash), tx->xinputs[i].prevxoutindex) ;
	    txo = txlookup(hashstr(tx->xinputs[i].prevxhash)) ;
	    if (txo != NULL) {
	        printf("previous UTXO: %s, %lu satoshis\n", hashstr(tx->xinputs[i].prevxhash), txo->xoutputs[outindex].satoshis) ;
	        printf("previous UTXO script: len=%d, code=%s\n", txo->xoutputs[outindex].script.len, bufstr(txo->xoutputs[outindex].script.code, txo->xoutputs[outindex].script.len, false)) ;
	    }
	    else {
		debug_print("previous UTXO for %s index %d not foumd!\n", hashstr(tx->xinputs[i].prevxhash), i) ;
		exit(1) ;
	    }

	    printf("%d: input script len=%d, code=%s\n", i, tx->xinputs[i].script.len, bufstr(tx->xinputs[i].script.code, tx->xinputs[i].script.len, false)) ;

	    // process unlock script and lock script together
	    scriptresult = process_script(catscripts(&tx->xinputs[i].script, &txo->xoutputs[outindex].script), tx, i, script_pattern) ;
	    debug_print("INPUTSCRIPTPATTERN %s\n", script_pattern) ;
	    if (scriptresult) {
	        printf("input script %d returns TRUE\n", i) ;
	    }
	    else {
	        printf("input script %d returns FALSE\n", i) ;
		exit(1) ;
	    }

	    printf("%d: seq num=%x\n", i, tx->xinputs[i].seqnum) ;
            totalsatoshisin += txo->xoutputs[outindex].satoshis ;
	    //printf("UTXODATE %s %d %s\n", bufstr(tx->hash, HASHLEN, true), outindex, txo->timestamp);
	}
    }
    printf("SATOSHIS IN: %lu\n", totalsatoshisin) ;

    totalsatoshisout = 0 ;
    printf("\n") ;
    printf("outputs:\n") ;
    for (i = 0 ; i < tx->outcounter ; i++) {
	printf("%d: satoshis %lu (%.6f BTC)\n", i, tx->xoutputs[i].satoshis, tx->xoutputs[i].satoshis/SATOSHIS2BTC) ;
	printf("%d: output script len=%lu, code=%s\n", i, tx->xoutputs[i].xscriptlen, bufstr(tx->xoutputs[i].script.code, tx->xoutputs[i].xscriptlen, false)) ;
	scriptresult = process_script(&tx->xoutputs[i].script, NULL, i, script_pattern) ;
	fprintf(stderr, "\nOUTSCRIPTPATTERN %s %s %lu\n", script_pattern, bufstr(tx->hash, HASHLEN, true), tx->xoutputs[i].satoshis) ;

        totalsatoshisout += tx->xoutputs[i].satoshis ;
    }

    fee = totalsatoshisin - totalsatoshisout ;
    printf("FEE: %lu\n", fee) ;

    if (tx->witnessed) {
        printf("\n") ;
	printf("witness list:\n") ;
        for (i = 0 ; i < tx->incounter ; i++) {
	    printf("%d: script len %d, code %s\n", i, tx->witnesses[i].script.len, bufstr(tx->witnesses[i].script.code, tx->witnesses[i].script.len, false)) ;
	    //scriptresult = process_script(&tx->witnesses[i].script, NULL, i, script_pattern) ;
	    //debug_print("\nWITNESSSCRIPTPATTERN %s %s %lu\n", script_pattern, bufstr(tx->hash, HASHLEN, true), tx->xoutputs[i].satoshis) ;
        }
    }
    else
	printf("\nwitnessed is false\n") ;

    printf("\nlock_time 0x%x (%u)\n", tx->lock_time, tx->lock_time) ;
    printf("\nblock file %s/blk%05d.dat\n", BLOCKDIR, tx->blockfilenum) ;
    printf("txnum %d\n", tx->txnum) ;
    printf("offset %d\n", tx->offset) ;
    printf("timestamp %s\n", tx->timestamp) ;
}

char *dtime(time_t t) {
    static char buf[80] ;
    struct tm ts ;

    setenv("TZ", TIMESTAMPTZ, 1);
    tzset();

    // Format time, "yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);

    return buf ;
}

char *datestr(time_t t) {
    static char buf[80] ;
    struct tm ts ;

    setenv("TZ", TIMESTAMPTZ, 1);
    tzset();

    // Format time, "yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &ts);

    return buf ;
}

void finish_with_error(MYSQL *con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

double btcquote(time_t utime) {
    MYSQL *con = mysql_init(NULL);
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    char query[MAXQUERY] ;
    char udate[80] ;
    struct tm ts ;

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, SQLHOST, SQLUSER, SQLPASS, SQLDB, 0, NULL, 0) == NULL) {
      finish_with_error(con);
    }

    // get 00:00:00Z at the current unix epoch
    setenv("TZ", TIMESTAMPTZ, 1);
    tzset();

    ts = *localtime(&utime);

    sprintf(udate, "%04d-%02d-%02d", ts.tm_year+1900, ts.tm_mon+1, ts.tm_mday) ;

    sprintf(query, "select close from btchist where udate=\"%s\"", udate) ;
    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    res = mysql_store_result(con);
    if (mysql_num_rows(res) != 1) {
	fprintf(stderr, "error: btchist db for %s returned %lu results\n", udate, mysql_num_rows(res)) ;
	exit(1) ;
    }

    row = mysql_fetch_row(res) ;

    mysql_close(con) ;

    return atof(row[0]) ;
}

void print_scriptstack() {
    int i;

    if (sstack.fptr < 0)
	return ;

    fprintf(stderr,"******stack start*********\n") ;
    for (i = 0 ; i <= sstack.fptr ; i++)
	fprintf(stderr, "sstack %d: len %d data: %s\n", i, sstack.item[i].len, bufstr(sstack.item[i].data, sstack.item[i].len, false)) ;
    fprintf(stderr,"******stack end***********\n") ;
}

void print_altscriptstack() {
    int i;

    if (altsstack.fptr < 0)
	return ;

    fprintf(stderr,"******stack start*********\n") ;
    for (i = 0 ; i <= altsstack.fptr ; i++)
	fprintf(stderr, "altstack %d: len %d data: %s\n", i, altsstack.item[i].len, bufstr(altsstack.item[i].data, altsstack.item[i].len, false)) ;
    fprintf(stderr,"******stack end***********\n") ;
}

struct txoutput *txo_query(char *hashstr, uint16_t outindex) {
    static struct txoutput txo ;
    struct transaction *tx ;

    debug_print("TXOUTPUT tx %s, output %d\n", hashstr, outindex) ;
    tx = txlookup(hashstr) ;

    if (tx != NULL) {
        debug_print("TXOUTPUT pkh=%s, satoshis=%lu (%.6f BTC)\n", bufstr(tx->hash, HASHLEN, true), tx->xoutputs[outindex].satoshis, tx->xoutputs[outindex].satoshis/SATOSHIS2BTC) ; 
        strcpy(txo.txid, hashstr) ;
        txo.satoshis = tx->xoutputs[outindex].satoshis ;
        txo.outindex = outindex ;
        txo.outscript = tx->xoutputs[outindex].script ;
    }
    else
	return NULL ;

    return &txo ;
}

uint8_t *hash160(uint8_t *buf, int len) {
    static uint8_t h160[RIPEMD160_DIGEST_LENGTH] ;
    uint8_t *h ;

    h = sha256(buf, len) ;
    debug_print("HASH160: %s\n", bufstr(h, HASHLEN, true)) ;
    ripemd160(h, HASHLEN, h160) ;

    return h160 ;
}

char *wallet_address(uint8_t *publickeystr) {
    static uint8_t waddr[WALLETADDRESSSIZE];
    uint8_t foo[WALLETADDRESSSIZE];
    uint8_t h160[RIPEMD160_DIGEST_LENGTH] ;
    int nbuf ;

    nbuf = str2buf(publickeystr, waddr) ;
    strcpy(foo, bufstr(waddr, nbuf, false)) ;
    debug_print("XYZ %s, nbuf=%lu\n", foo, strlen(foo)) ;
    str2buf(foo, waddr) ;

    //memcpy(h160, hash160(waddr, nbuf), RIPEMD160_DIGEST_LENGTH) ;
    //strcpy(waddr, bufstr(h160, RIPEMD160_DIGEST_LENGTH, false)) ;


    memcpy(h160, hash160(foo, strlen(foo)), RIPEMD160_DIGEST_LENGTH) ;
    strcpy(waddr, bufstr(h160, RIPEMD160_DIGEST_LENGTH, false)) ;
    return waddr ;
}

// encode int to script stack item
struct scriptstackitem *scriptnumenc(int32_t num) {
    static struct scriptstackitem snum ;
    uint32_t absnum ;
    unsigned char *bptr ;
    bool isneg = false ;
    int i ;

    absnum = abs(num) ;
    if (num < 0)
	isneg = true ;

    bptr = (uint8_t *)&absnum ;
    snum.len = 1 ;
    for (i =0 ; i < 4 ; i++) {
	snum.data[i] = 0 ;
	if (i != 0 && *(bptr+i) != 0) {
	    snum.len++ ;
	}
    }

    if (isneg)
	*(bptr+(snum.len-1)) |= 0x80 ;

    for (i = 0 ; i < snum.len ; i++)  {
	snum.data[i] = *(bptr+i) ;
	//printf("%d: 0x%x\n", i, snum.data[i]) ;
    }

    //printf("snum.len=%d\n", snum.len) ;

    return &snum ;
}

// decode script stack item to int
int32_t scriptnumdec(struct scriptstackitem *item) {
    static int32_t num ;
    struct scriptstackitem snum ;
    unsigned char *bptr ;
    bool isneg = false ;
    int i ;

    snum.len = 1 ;
    for (i =0 ; i < 4 ; i++) {
	if (i != 0 && item->data[i] != 0) {
	    snum.len++ ;
	}
    }

    if (item->data[snum.len-1] & 0x80) {
	isneg = true ;
	item->data[snum.len-1] &= 0x7f ;
    }

    memcpy(&num, item->data, snum.len) ;

    if (isneg)
	num *= -1 ;
    //printf("snum.len=%d\n", snum.len) ;

    return num ;
}

struct xscript *catscripts(struct xscript *s1, struct xscript *s2) {
    static struct xscript script ;
    // combine scripts
    script.len = s1->len + s2->len ;
    script.code = malloc(script.len) ;
    if (script.code == NULL) {
        debug_print("error: could not malloc %s script for transaction\n", "temp") ;
        exit(1) ;
    }
    memcpy(script.code, s1->code, s1->len) ;
    memcpy(script.code+s1->len, s2->code, s2->len) ;
    mpush(script.code) ;
    return &script ;
}

// return file offset to the given blockfilenum (in txrec) in the hex fd file\n
long hextell(struct txindexrecord *txrec, FILE *hexfd) {
    long ftell0 ;
    long ftellend ;
    int reclow, rechigh, reccurr ;
    uint32_t nrecs ;
    int offset ;
    bool found = false ;
    struct txindexrecord tx ;
    int i = 0;
    int backoffset ;

    ftell0 = ftell(hexfd) ;
    fseek(hexfd, 0, SEEK_END) ;
    ftellend = ftell(hexfd) ;

    if (ftellend % sizeof(struct txindexrecord) != 0) {
	fprintf(stderr,"HEXTELL: error: hex db file length %lu not even multiple of sizeof(struct txindexrecord)\n", ftellend) ;
	exit(1) ;
    }

    nrecs = ftellend/sizeof(struct txindexrecord) ;
    debug_print( "file size=%lu, nrecs=%d\n", ftell0, nrecs) ;

    reclow = 0 ;
    rechigh = nrecs ;

    debug_print("===============%s===================\n", "HEXTELL") ;
    while (rechigh - reclow > 1) {
        reccurr = reclow + (rechigh-reclow)/2 ;  // binary search
        offset = reccurr*sizeof(struct txindexrecord) ;
	debug_print("---------------%d-------------------\n", i) ;
	debug_print("looking for %d %d %s\n", txrec->blockfilenum, txrec->blocknum, hashstr(txrec->hash)) ;
	debug_print("initial %d: %d->%d->%d, offset=%d\n", i, reclow, reccurr, rechigh, offset) ;
	fseek(hexfd, offset, SEEK_SET) ;
	fread(&tx, sizeof(struct txindexrecord), 1, hexfd) ;
	debug_print("at %d got blockfilenum=%d, blocknum=%d (want %d, %d)\n", offset, tx.blockfilenum, tx.blocknum, txrec->blockfilenum, txrec->blocknum) ;

	if (tx.blockfilenum < txrec->blockfilenum)
            reclow = reccurr ;
	else if (tx.blockfilenum > txrec->blockfilenum)
	    rechigh = reccurr ;
	else {
	    debug_print("EQUAL %d\n",i) ;
	    if (tx.blocknum < txrec->blocknum) 
	        reclow = reccurr ;
	    else if (tx.blocknum > txrec->blocknum)
	        rechigh = reccurr ;
	    else {
#if 1
		backoffset = 2*sizeof(struct txindexrecord) ;
		while (tx.blockfilenum == txrec->blockfilenum && tx.blocknum == txrec->blocknum && ftell(hexfd) >= backoffset) {
		    fseek(hexfd, -backoffset, SEEK_CUR) ;
	            fread(&tx, sizeof(struct txindexrecord), 1, hexfd) ;
		    debug_print("backing up to %d %d %s\n", tx.blockfilenum, tx.blocknum, hashstr(tx.hash)) ;
		    reccurr-- ;
		}
		if (reccurr <= 0)
		    reccurr = 0 ;
		else
		    reccurr++ ;
#endif
	        rechigh = reccurr ;
	        reclow = reccurr ;
            }
	}
	debug_print("%d: low=%d, high=%d, curr=%d, offset=%d\n", i, reclow, rechigh, reccurr, offset) ;
	i++ ;
    }
    debug_print("***FINAL: blockfilenum=%d, blocknum=%d (want %d, %d)\n", tx.blockfilenum, tx.blocknum, txrec->blockfilenum, txrec->blocknum) ;

    offset = reclow*sizeof(struct txindexrecord) ;
    fseek(hexfd, ftell0, SEEK_SET) ;
    debug_print("offset=%d\n", offset) ;
//exit(1) ;
    return offset ;
}
