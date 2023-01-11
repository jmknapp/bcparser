#define HOME "/hyper/.bitcoin"
#define BLOCKDIR "/hyper/.bitcoin/blocks"
#define TXDATADIR "/hyper/.bitcoin/txdata"
//#define TXDATADIR "/home/jmknapp/bcx4/txdata"
#define TXINDEXDIR "/hyper/.bitcoin/txindex"
#define TXHEXDIR "/hyper/.bitcoin/txhex.arch"
//#define TXHEXDIR "/home/jmknapp/bcx4/txhex"


#define HASHLEN 32
#define SHA1LEN 20
#define HASHSTRLEN 65
#define MAGIC 0xD9B4BEF9
#define BLOCKHEADERSIZE 80
#define SATOSHIS2BTC 1e8
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#define COLOR(c) (fprintf(stderr, "%s", c))
#define DEBUG 0

#define MAXMALLOC 5000000
#define MAXTRANSLEN 2000000
#define MAXTRANSEL 10000 
#define MAXINPUTS 20000
#define MAXBUFSTR 5000000
#define MAXSTRING 10000
#define MAXFILEPATH 200
#define MAXQUERY 300
#define MAXSCRIPTITEMLEN 75
#define MAXSCRIPTITEMS 5000
#define WALLETADDRESSSIZE 300
#define MAX58LEN 200
#define MAXSCRIPT 1024
#define MAXTIMELOCK 500000000

#define SQLDB "crypto"
#define SQLUSER "bitcoin"
#define SQLPASS "sw0rdfish"
#define SQLHOST "localhost"

#define TIMESTAMPTZ "GMT0BST"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <byteswap.h>
#include <mysql.h>
#include "openssl/sha.h"
#include "ansi.h"
#include "bcsqllib.h"
#include "ripemd160.h"

void printhash(char *h) ;
void printhashrev(char *h) ;
uint64_t varint(FILE *ifd, int *nbytes) ;
char *double_sha256_string(char *string, int len, char outputBuffer[65], bool reverse) ;
char *double_sha256(char *buf, int len, char outputBuffer[65]) ;
void sha256_string(char *string, int len, char outputBuffer[65]) ;
void sha256_stringrev(char *string, int len, char outputBuffer[65]) ;
char *sha256(char *buf, int len) ;
struct blockheader *getblockheader(FILE *blockfd) ;

struct blockheader {
    int32_t version ;
    char hashPrevBlock[32] ;
    char hashMerkleRoot[32] ;
    uint32_t timestamp ;
    uint32_t bits ;
    uint32_t nonce ;
} ;

struct xscript {
    uint16_t len ;
    uint8_t *code ;
} ;

struct transinputs {
   char prevxhash[32] ;
   int32_t prevxoutindex ;
   uint64_t xscriptlen ;
   struct xscript script ;
   uint32_t seqnum ;
} ;

struct transoutputs {
    uint64_t satoshis ;
    uint64_t xscriptlen ;
    struct xscript script; 
} ;

struct witnesslist {
    struct xscript script ;
    //int len ;
    //char *code ;
} ;

struct txindexrecord {
   char hash[HASHLEN] ;
   uint16_t blockfilenum ;
   uint32_t blocknum ;
   uint32_t txnum ;
   uint32_t offset ;
} ;

struct txindexrecord_new {
   char hash[HASHLEN] ;
   uint16_t blockfilenum ;
   uint32_t blocknum ;
   uint32_t txnum ;
   uint32_t offset ;
} ;

struct transaction {
    uint32_t version ;
    bool witnessed ;
    uint64_t incounter ;
    struct transinputs *xinputs ;
    uint64_t outcounter ;
    struct transoutputs *xoutputs ;
    struct witnesslist witnesses[MAXINPUTS]; 
    uint32_t lock_time ;
    uint16_t blockfilenum ;
    uint32_t blocknum ;
    uint32_t txnum ;
    uint32_t offset ;
    char hash[HASHLEN] ;
    char timestamp[80] ;
} ;

struct block {
    uint32_t magic ;
    uint32_t blocksize ;
    struct blockheader *blkhdr ;
    uint64_t transactionCounter ;
    struct transaction *transactions ;
} ;

struct mallocstack {
    int fptr ;
    int savefptr ;
    void *ptrs[MAXMALLOC] ;
} mstack ;

struct scriptstackitem {
    uint8_t type ;
    uint16_t len ;
    uint8_t data[MAXSCRIPTITEMLEN] ;
} ;

struct scriptstack {
    int16_t fptr;
    struct scriptstackitem item[MAXSCRIPTITEMS] ;
} sstack;

struct altscriptstack {
    int16_t fptr;
    struct scriptstackitem item[MAXSCRIPTITEMS] ;
} altsstack;

struct txoutput {
    char txid[HASHSTRLEN] ;
    char pkhash[RIPEMD160_DIGEST_LENGTH] ;
    uint16_t outindex ;
    uint64_t satoshis ;
    struct xscript outscript ;
    uint32_t seqnum ;
} ;

char *serializedtransaction(struct transaction *tx) ;
struct block *nextblock(FILE *blockfd, int blockfilenum, int blocknum) ;
void printblockinfo(struct block *blk) ;
void hexdump(FILE *ifd, int len) ;
char *hashstr(char *hash) ;
void witnessdump(FILE *ifd, int len) ;
void mpush(void *p) ;
bool mpop() ;
bool spop(struct scriptstackitem *item) ;
int mrollback() ;
void mmark() ;
void minit() ;
void sinit() ;
int mfree() ;
int sfree() ;
char *varintstr(uint64_t vi) ;
char *bufstr(char *buf, int len, bool reverse) ;
int string2bytes(char *string, char *buf) ;
int str2buf(char *str, char *buf) ;
int str2bufrev(char *str, char *buf) ;
uint8_t hexchar2int(char hexchar) ;
bool nexttransaction(struct transaction *tx, FILE *blockfd) ;
struct transaction *txlookup(char *hashstring) ;
void printtx(struct transaction *tx) ;
char *dtime(time_t t) ;
char *datestr(time_t t) ;
int setenv() ;
void tzset() ;
void finish_with_error(MYSQL *con) ;
bool tx_in_hexdb(struct txindexrecord *tx) ;
double btcquote(time_t utime) ;
bool process_script(struct xscript *script, struct transaction *tx, int inputnum, char *script_pattern) ;
void print_scriptstack() ;
struct txoutput *txo_query(char *hashstr, uint16_t outindex) ;
uint8_t *hash160(uint8_t *buf, int len) ;
void sha1(char *sha1buf, const char *buf, int len) ;
char *wallet_address(uint8_t *publickey) ;
bool b58enc(char *b58, size_t *b58sz, const void *data, size_t binsz) ;
struct scriptstackitem *scriptnumenc(int32_t num) ;
int32_t scriptnumdec(struct scriptstackitem *item) ;
void spush(struct scriptstackitem *item) ;
void altspush(struct scriptstackitem *item) ;
bool altspop(struct scriptstackitem *item) ;
struct xscript *catscripts(struct xscript *s1, struct xscript *s2) ;
void print_altscriptstack() ;
