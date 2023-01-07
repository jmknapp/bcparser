#include "bitcoin.h"
#include "script.h"

bool process_script(struct xscript *script, struct transaction *tx, int inputnum, char *scriptpattern) {
    uint32_t scriptptr ;
    uint8_t opcode ;
    struct scriptstackitem item, item2, item3, item4, item5, item6, h160, ripemd160op, sha256op, sha1op ;
    char *h ;
    bool scriptresult = true ;
    int32_t i, j, n ;
    uint8_t ripemd160buf[RIPEMD160_DIGEST_LENGTH] ;
    uint8_t sha256buf[HASHLEN] ;
    uint8_t sha1buf[SHA1LEN] ;
    int opifdepth = 0 ;
    struct txoutput *txo ;
    uint16_t npush2 ;
    uint32_t npush4 ;
    bool doscriptpattern = true ;

    debug_print("=========================\nprocessing script: %s, len=%d\n", bufstr(script->code, script->len, false), script->len) ;

    sinit() ;

    if (scriptpattern == NULL)
	doscriptpattern = false ;

    // get tx output
    if (tx != NULL) {
        //txo = txo_query(hashstr(tx->xinputs[inputnum].prevxhash), inputnum) ;
        txo = txo_query(hashstr(tx->xinputs[inputnum].prevxhash), tx->xinputs[inputnum].prevxoutindex) ;
        if (txo == (struct txoutput *)NULL) {
            fprintf(stderr,"PROCESS_SCRIPT: error: can't find tx %s\n", hashstr(tx->xinputs[inputnum].prevxhash)) ;
            exit(1) ;
        }    
    }

    if (script->len == 0)
        return true ;

    scriptptr = 0 ;
    strcpy(scriptpattern, "") ;

    while (scriptptr < script->len) {
        opcode = script->code[scriptptr] ;
        debug_print("opcode: %d (%x): %s\n", opcode, opcode, opcodestring(opcode)) ;

	if (doscriptpattern) {
	    strcat(scriptpattern, ":") ;
	    strcat(scriptpattern, opcodestring(opcode)) ;
	}

        if (opcode > 0 && opcode <= 75) {
            item.len = opcode ;
            item.type = OP_DATA ;
            memcpy(item.data, script->code+scriptptr+1, opcode) ;
            debug_print("pushing %d  bytes onto the script stack: %s\n", opcode, bufstr(script->code+scriptptr+1, opcode, false)) ;
            spush(&item) ;
            scriptptr += opcode ;
        }
        else {
            switch (opcode) {
                case OP_0:
                    item.len = 0 ;
                    spush(&item) ;
                    break;
                case OP_PUSHDATA1:
                    scriptptr++ ;
                    item.len = script->code[scriptptr] ;
                    scriptptr++ ;
                    memcpy(&item.data, &script->code[scriptptr], item.len) ;
                    scriptptr += item.len ;
                    spush(&item) ;
                    break;
                case OP_PUSHDATA2:
                    scriptptr++ ;
                    memcpy(&npush2, script->code+scriptptr, 2) ;
                    debug_print("OP_PUSHDATA2: n=%d\n", npush2) ;
                    scriptptr += 2 ;
                    item.len = npush2 ;

                    memcpy(item.data, script->code+scriptptr, npush2) ;
                    scriptptr += npush2 ;
                    spush(&item) ;
                    break;
                case OP_PUSHDATA4:
                    scriptptr++ ;
                    memcpy(&npush4, script->code+scriptptr, 4) ;
                    scriptptr += 4 ;
                    item.len = npush4 ;
                    memcpy(item.data, script->code+scriptptr, npush4) ;
                    scriptptr += npush4 ;
                    spush(&item) ;
                    break;
                case OP_1NEGATE:
                    spush(scriptnumenc(-1)) ;
                    break;
                case OP_TRUE:  // also OP_1
                    spush(scriptnumenc(1)) ;
                    break;
                case OP_2:
                    spush(scriptnumenc(2)) ;
                    break;
                case OP_3:
                    spush(scriptnumenc(3)) ;
                    break;
                case OP_4:
                    spush(scriptnumenc(4)) ;
                    break;
                case OP_5:
                    spush(scriptnumenc(5)) ;
                    break;
                case OP_6:
                    spush(scriptnumenc(6)) ;
                    break;
                case OP_7:
                    spush(scriptnumenc(7)) ;
                    break;
                case OP_8:
                    spush(scriptnumenc(8)) ;
                    break;
                case OP_9:
                    spush(scriptnumenc(9)) ;
                    break;
                case OP_10:
                    spush(scriptnumenc(10)) ;
                    break;
                case OP_11:
                    spush(scriptnumenc(11)) ;
                    break;
                case OP_12:
                    spush(scriptnumenc(12)) ;
                    break;
                case OP_13:
                    spush(scriptnumenc(13)) ;
                    break;
                case OP_14:
                    spush(scriptnumenc(144)) ;
                    break;
                case OP_15:
                    spush(scriptnumenc(15)) ;
                    break;
                case OP_16:
                    spush(scriptnumenc(16)) ;
                    break;
                case OP_NOP:
                    break;
                case OP_IF:
                    spop(&item) ;
                    if (scriptnumdec(&item) != 0) {
                        opifdepth++ ;
                        scriptptr++ ;
                    }
                    else {  // condition is false, look for next ELSE or ENDIF
                        while (script->code[scriptptr] != OP_ELSE && script->code[scriptptr] != OP_ENDIF && scriptptr < script->len)
                            scriptptr++ ;
                        if (script->code[scriptptr] == OP_ELSE) {
                            opifdepth++ ;
                            scriptptr++ ;
                        }
                        else if (script->code[scriptptr] == OP_ENDIF)
                            scriptptr++ ;
                        else {
                            debug_print("OP_IF: error: matching ELSE or ENDIF %s\n", "not found") ;
                            scriptresult = false ;
                        }
                    }
                    break;
                case OP_NOTIF:
                    spop(&item) ;
                    if (scriptnumdec(&item) == 0) {
                        opifdepth++ ;
                        scriptptr++ ;
                    }
                    else {  // condition is true, look for next ELSE or ENDIF
                        while (script->code[scriptptr] != OP_ELSE && script->code[scriptptr] != OP_ENDIF && scriptptr < script->len)
                            scriptptr++ ;
                        if (script->code[scriptptr] == OP_ELSE) {
                            opifdepth++ ;
                            scriptptr++ ;
                        }
                        else if (script->code[scriptptr] == OP_ENDIF)
                            scriptptr++ ;
                        else {
                            debug_print("OP_IF: error: matching ELSE or ENDIF %s\n", "not found") ;
                            scriptresult = false ;
                        }
                    }
                    break;
                case OP_ELSE:
                    if (opifdepth <= 0) {
                        debug_print("OP_ELSE: error: ELSE without prior OP_IF at spriptptr=%d\n", scriptptr) ;
                        scriptresult = false ;
                    }
                    else {
                        scriptptr++ ;
                    }
                    break;
                case OP_ENDIF:
                    opifdepth-- ;
                    if (opifdepth < 0) {
                        debug_print("OP_ENDIF: error: OP_ENDIF without prior OP_IF at scriptptr=%d\n", scriptptr) ;
                        scriptresult = false ;
                    }
                    scriptptr++ ;
                    break;
                case OP_VERIFY:
                    spop(&item) ;
                    if (item.len != 1 || item.data[0] != 1)
                        scriptresult = false ;
                    break;
                case OP_RETURN:
                    scriptresult = false ;
                    break;
                case OP_TOALTSTACK:
                    spop(&item) ;
                    altspush(&item) ;
                    break;
                case OP_FROMALTSTACK:
                    altspop(&item) ;
                    spush(&item) ;
                    break;
                case OP_IFDUP:
                    spop(&item) ;
                    if (item.len != 1 || item.data[0] != 0) {
                        spush(&item) ;
                        spush(&item) ;
                    }
                    else
                        spush(&item) ;
                    break;
                case OP_DEPTH:
                    spush(scriptnumenc(sstack.fptr+1)) ;
                    break;
                case OP_DROP:
                    spop(&item) ;
                    break;
                case OP_DUP:
                    spop(&item) ;
                    spush(&item) ;
                    spush(&item) ;
                    break;
                case OP_NIP:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(&item) ;
                    break;
                case OP_OVER:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(&item2) ;
                    spush(&item) ;
                    spush(&item2) ;
                    break;
                case OP_PICK:
                    spop(&item) ;
                    if (item.len != 1) {
                        debug_print("OP_PICK error: top stack item (n) has %d bytes\n", item.len) ;
                        scriptresult = false ;
                    }
                    else {
                        slook(&item2, item.data[0]) ;
                        spush(&item2) ;
                    }
                    break;
                case OP_ROLL:
                    // get n from top of stack
                    spop(&item) ;
                    n = scriptnumdec(&item) ;
                    if (n > sstack.fptr) {
                        fprintf(stderr, "OP_ROLL: error: stack size is %d, can't roll item %d back\n", sstack.fptr, n) ;
                        exit(1) ;
                    }
                    item2 = sstack.item[sstack.fptr - n] ;
                    for (j = sstack.fptr - n; j < sstack.fptr ; j++)
                        sstack.item[j] = sstack.item[j+1] ;
                    sstack.fptr-- ;
                    spush(&item2) ;
                    break;
                case OP_ROT:
                    spop(&item) ;
                    spop(&item2) ;
                    spop(&item3) ;
                    spush(&item2) ;
                    spush(&item) ;
                    spush(&item3) ;
                    break;
                case OP_SWAP:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(&item) ;
                    spush(&item2) ;
                    break;
                case OP_TUCK:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(&item) ;
                    spush(&item2) ;
                    spush(&item) ;
                    break;
                case OP_2DROP:
                    spop(&item) ;
                    spop(&item2) ;
                    break;
                case OP_2DUP:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(&item2) ;
                    spush(&item) ;
                    spush(&item2) ;
                    spush(&item) ;
                    break;
                case OP_3DUP:
                    spop(&item) ;
                    spop(&item2) ;
                    spop(&item3) ;
                    spush(&item3) ;
                    spush(&item2) ;
                    spush(&item) ;
                    spush(&item3) ;
                    spush(&item2) ;
                    spush(&item) ;
                    break;
                case OP_2OVER:
                    slook(&item, 3) ;
                    slook(&item2, 2) ;
                    spush(&item) ;
                    spush(&item2) ;
                    break;
                case OP_2ROT:
                    spop(&item) ;
                    spop(&item2) ;
                    spop(&item3) ;
                    spop(&item4) ;
                    spop(&item5) ;
                    spop(&item6) ;
                    spush(&item4) ;
                    spush(&item3) ;
                    spush(&item2) ;
                    spush(&item) ;
                    spush(&item6) ;
                    spush(&item5) ;
                    break;
                case OP_2SWAP:
                    spop(&item) ;
                    spop(&item2) ;
                    spop(&item3) ;
                    spop(&item4) ;
                    spush(&item3) ;
                    spush(&item4) ;
                    spush(&item) ;
                    spush(&item2) ;
                    break;
                case OP_CAT:
                    scriptresult = false ;
                    break;
                case OP_SUBSTR:
                    scriptresult = false ;
                    break;
                case OP_LEFT:
                    scriptresult = false ;
                    break;
                case OP_RIGHT:
                    scriptresult = false ;
                    break;
                case OP_SIZE:
                    spop(&item) ;
                    spush(&item) ;
                    spush(scriptnumenc(item.len)) ;
                    break;
                case OP_INVERT:
                    scriptresult = false ;
                    break;
                case OP_AND:
                    scriptresult = false ;
                    break;
                case OP_OR:
                    scriptresult = false ;
                    break;
                case OP_XOR:
                    scriptresult = false ;
                    break;
                case OP_EQUAL:
                    spop(&item) ;
                    spop(&item2) ;

                    if ((item.len != item2.len) || memcmp(item.data, item2.data, item.len) != 0)
                        spush(scriptnumenc(0)) ;
                    else
                        spush(scriptnumenc(1)) ;
                    break;
                case OP_EQUALVERIFY:
                    spop(&item) ;
                    spop(&item2) ;

                    if ((item.len != item2.len) || memcmp(item.data, item2.data, item.len) != 0)
                        scriptresult = false ;
                    break;
                case OP_1ADD:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    i++ ;
                    spush(scriptnumenc(i)) ;
                    break;
                case OP_1SUB:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    i-- ;
                    spush(scriptnumenc(i)) ;
                    break;
                case OP_2MUL:
                    scriptresult = false ;
                    break;
                case OP_2DIV:
                    scriptresult = false ;
                    break;
                case OP_NEGATE:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    i *= -1 ;
                    spush(scriptnumenc(i)) ;
                    break;
                case OP_ABS:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    if (i < 0)
                        i *= -1 ;
                    spush(scriptnumenc(i)) ;
                    break;
                case OP_NOT:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    if (i == 0)
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_0NOTEQUAL:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    if (i == 0)
                        spush(scriptnumenc(0)) ;
                    else
                        spush(scriptnumenc(1)) ;
                    break;
                case OP_ADD:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(scriptnumenc(scriptnumdec(&item) + scriptnumdec(&item2))) ;
                    break;
                case OP_SUB:
                    spop(&item) ;
                    spop(&item2) ;
                    spush(scriptnumenc(scriptnumdec(&item2) - scriptnumdec(&item))) ;
                    break;
                case OP_MUL:
                    scriptresult = false ;
                    break;
                case OP_DIV:
                    scriptresult = false ;
                    break;
                case OP_MOD:
                    scriptresult = false ;
                    break;
                case OP_LSHIFT:
                    scriptresult = false ;
                    break;
                case OP_RSHIFT:
                    scriptresult = false ;
                    break;
                case OP_BOOLAND:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item) != 0 && scriptnumdec(&item2) != 0)
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_BOOLOR:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item) != 0 || scriptnumdec(&item2) != 0)
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_NUMEQUAL:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item) == scriptnumdec(&item2))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_NUMEQUALVERIFY:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item) != scriptnumdec(&item2))
                        scriptresult = false ;
                    break;
                case OP_NUMNOTEQUAL:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item) != scriptnumdec(&item2))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_LESSTHAN:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item2) < scriptnumdec(&item))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_GREATERTHAN:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item2) > scriptnumdec(&item))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_LESSTHANOREQUAL:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item2) <= scriptnumdec(&item))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_GREATERTHANOREQUAL:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item2) >= scriptnumdec(&item))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_MIN:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item2) >= scriptnumdec(&item))
                        spush(scriptnumenc(scriptnumdec(&item))) ;
                    else
                        spush(scriptnumenc(scriptnumdec(&item2))) ;
                    break;
                case OP_MAX:
                    spop(&item) ;
                    spop(&item2) ;
                    if (scriptnumdec(&item2) >= scriptnumdec(&item))
                        spush(scriptnumenc(scriptnumdec(&item2))) ;
                    else
                        spush(scriptnumenc(scriptnumdec(&item))) ;
                    break;
                case OP_WITHIN:
                    spop(&item) ; // max
                    spop(&item2) ; // min
                    spop(&item3) ; // num
                    if (scriptnumdec(&item3) >= scriptnumdec(&item2) && scriptnumdec(&item3) < scriptnumdec(&item))
                        spush(scriptnumenc(1)) ;
                    else
                        spush(scriptnumenc(0)) ;
                    break;
                case OP_RIPEMD160:
                    spop(&item) ;
                    ripemd160(item.data, item.len, ripemd160buf) ;
                    memcpy(&ripemd160op.data, ripemd160buf, RIPEMD160_DIGEST_LENGTH) ;
                    ripemd160op.len = RIPEMD160_DIGEST_LENGTH ;
                    ripemd160op.type = OP_DATA ;
                    spush(&ripemd160op) ;
                    debug_print("OP_RIPEMD160: %s\n", bufstr(ripemd160op.data, ripemd160op.len, false)) ;
                    break;
                case OP_SHA1:
                    spop(&item) ;
                    sha1(sha1buf, item.data, item.len) ;
                    memcpy(&sha1op.data, sha1buf, SHA1LEN) ;
                    sha1op.len = SHA1LEN ;
                    sha1op.type = OP_DATA ;
                    spush(&sha1op) ;
                    break;
                case OP_SHA256:
                    spop(&item) ;
                    memcpy(&sha256op.data, sha256(item.data, item.len), HASHLEN) ;
                    sha256op.len = HASHLEN ;
                    sha256op.type = OP_DATA ;
                    spush(&sha256op) ;
                    break;
                case OP_HASH160:
                    spop(&item) ;
                    memcpy(&h160.data, hash160(item.data, item.len), RIPEMD160_DIGEST_LENGTH) ;
                    h160.len = RIPEMD160_DIGEST_LENGTH ;
                    h160.type = OP_DATA ;
                    debug_print("HASH160: %s\n", bufstr(h160.data, h160.len, false)) ;
                    spush(&h160) ;
                    break;
                case OP_HASH256:
                    spop(&item) ;
                    memcpy(&sha256op.data, double_sha256(item.data, item.len, sha256buf), HASHLEN) ;
                    sha256op.len = HASHLEN ;
                    sha256op.type = OP_DATA ;
                    spush(&sha256op) ;
                    break;
                case OP_CODESEPARATOR:
                    break;
                case OP_CHECKSIG:
                    spop(&item) ;   // public key
                    spop(&item2) ;  // sig
                    spush(scriptnumenc(1)) ;  // just put TRUE on stack for now
                    // TODO
                    break;
                case OP_CHECKSIGVERIFY:
                    spop(&item) ;   // public key
                    spop(&item2) ;  // sig
                    // TODO
                    break;
                case OP_CHECKMULTISIG:
                    spop(&item) ;   // public key
                    spop(&item2) ;  // sig
                    spop(&item3) ;  // sig
                    spush(scriptnumenc(1)) ;  // just put TRUE on stack for now
                    // TODO
                    break;
                case OP_CHECKMULTISIGVERIFY:
                    spop(&item) ;   // public key
                    spop(&item2) ;  // sig
                    spop(&item3) ;  // sig
                    // TODO
                    break;
                case OP_CHECKLOCKTIMEVERIFY:
                    spop(&item) ;
                    if (item.len != 1) {
                        debug_print("OP_CHECKLOCKTIMEVERIFY: error: stack item length is %d\n", item.len) ;
                        scriptresult = false ;
                    }
                    else {
                        i = scriptnumdec(&item) ;
                        debug_print("OP_CHECKLOCKTIMEVERIFY: lock time is 0x%x, stack has 0x%x\n", tx->lock_time, i) ; 

                        if (i < 0 || (i >= MAXTIMELOCK && tx->lock_time < MAXTIMELOCK) || (tx->lock_time >= MAXTIMELOCK && i < MAXTIMELOCK) && tx->xinputs[inputnum].seqnum != 0xffff) {
                            debug_print("OP_CHECKLOCKTIMEVERIFY: error: lock_time 0x%x out of bounds\n", tx->lock_time) ; 
                            scriptresult = false ;
                        }
                        else
                            spush(&item) ;
                    }
                    break;
                case OP_CHECKSEQUENCEVERIFY:
                    spop(&item) ;
                    i = scriptnumdec(&item) ;
                    if (tx->xinputs[inputnum].seqnum >= i)
                        scriptresult = false ;
                    else
                        spush(&item) ;
                    break;
                case OP_PUBKEYHASH:
                    scriptresult = false ;
                    break;
                case OP_PUBKEY:
                    scriptresult = false ;
                    break;
                case OP_INVALIDOPCODE:
                    scriptresult = false ;
                    break;
                case OP_RESERVED:
                    if (opifdepth == 0)
                        scriptresult = false ;
                    break;
                case OP_VER:
                    if (opifdepth == 0)
                        scriptresult = false ;
                    break;
                case OP_VERIF:
                    break;
                case OP_VERNOTIF:
                    scriptresult = false ;
                    break;
                case OP_RESERVED1:
                    if (opifdepth == 0)
                        scriptresult = false ;
                    break;
                case OP_RESERVED2:
                    if (opifdepth == 0)
                        scriptresult = false ;
                    break;
                case OP_NOP1:
                    break;
                case OP_NOP4:
                    break;
                case OP_NOP5:
                    break;
                case OP_NOP6:
                    break;
                case OP_NOP7:
                    break;
                case OP_NOP8:
                    break;
                case OP_NOP9:
                    break;
                case OP_NOP10:
                    break;
                default:
		    debug_print("PROCESS_SCRIPT: error: unknown opcode 0x%x\n", opcode) ;
                    exit(1) ;
            }
        }

        if (scriptresult == false) {
            //free(script->code) ;
            sfree() ;
            return false ;
        }

        scriptptr++ ;  // poin to next opcode
    }
    //free(script->code) ;
    sfree() ;
    return scriptresult ;
}

char *opcodestring(uint8_t opcode) {
    static char opstr[10000] ;
    if (opcode > 0 && opcode <= 75) {
        strcpy(opstr, "OP_DATA") ;
	return opstr;
    }

    switch (opcode) {
        case OP_DATA:
            strcpy(opstr, "OP_DATA") ;
            break;
        case OP_OPCODE:
            strcpy(opstr, "OP_OPCODE") ;
            break;
        case OP_0:
            strcpy(opstr, "OP_0") ;
            break;
        case OP_PUSHDATA1:
            strcpy(opstr, "OP_PUSHDATA1") ;
            break;
        case OP_PUSHDATA2:
            strcpy(opstr, "OP_PUSHDATA2") ;
            break;
        case OP_PUSHDATA4:
            strcpy(opstr, "OP_PUSHDATA4") ;
            break;
        case OP_1NEGATE:
            strcpy(opstr, "OP_1NEGATE") ;
            break;
        case OP_TRUE:
            strcpy(opstr, "OP_TRUE") ;
            break;
        case OP_2:
            strcpy(opstr, "OP_2") ;
            break;
        case OP_3:
            strcpy(opstr, "OP_3") ;
            break;
        case OP_4:
            strcpy(opstr, "OP_4") ;
            break;
        case OP_5:
            strcpy(opstr, "OP_5") ;
            break;
        case OP_6:
            strcpy(opstr, "OP_6") ;
            break;
        case OP_7:
            strcpy(opstr, "OP_7") ;
            break;
        case OP_8:
            strcpy(opstr, "OP_8") ;
            break;
        case OP_9:
            strcpy(opstr, "OP_9") ;
            break;
        case OP_10:
            strcpy(opstr, "OP_10") ;
            break;
        case OP_11:
            strcpy(opstr, "OP_11") ;
            break;
        case OP_12:
            strcpy(opstr, "OP_12") ;
            break;
        case OP_13:
            strcpy(opstr, "OP_13") ;
            break;
        case OP_14:
            strcpy(opstr, "OP_14") ;
            break;
        case OP_15:
            strcpy(opstr, "OP_15") ;
            break;
        case OP_16:
            strcpy(opstr, "OP_16") ;
            break;
        case OP_NOP:
            strcpy(opstr, "OP_NOP") ;
            break;
        case OP_IF:
            strcpy(opstr, "OP_IF") ;
            break;
        case OP_NOTIF:
            strcpy(opstr, "OP_NOTIF") ;
            break;
        case OP_ELSE:
            strcpy(opstr, "OP_ELSE") ;
            break;
        case OP_ENDIF:
            strcpy(opstr, "OP_ENDIF") ;
            break;
        case OP_VERIFY:
            strcpy(opstr, "OP_VERIFY") ;
            break;
        case OP_RETURN:
            strcpy(opstr, "OP_RETURN") ;
            break;
        case OP_TOALTSTACK:
            strcpy(opstr, "OP_TOALTSTACK") ;
            break;
        case OP_FROMALTSTACK:
            strcpy(opstr, "OP_FROMALTSTACK") ;
            break;
        case OP_IFDUP:
            strcpy(opstr, "OP_IFDUP") ;
            break;
        case OP_DEPTH:
            strcpy(opstr, "OP_DEPTH") ;
            break;
        case OP_DROP:
            strcpy(opstr, "OP_DROP") ;
            break;
        case OP_DUP:
            strcpy(opstr, "OP_DUP") ;
            break;
        case OP_NIP:
            strcpy(opstr, "OP_NIP") ;
            break;
        case OP_OVER:
            strcpy(opstr, "OP_OVER") ;
            break;
        case OP_PICK:
            strcpy(opstr, "OP_PICK") ;
            break;
        case OP_ROLL:
            strcpy(opstr, "OP_ROLL") ;
            break;
        case OP_ROT:
            strcpy(opstr, "OP_ROT") ;
            break;
        case OP_SWAP:
            strcpy(opstr, "OP_SWAP") ;
            break;
        case OP_TUCK:
            strcpy(opstr, "OP_TUCK") ;
            break;
        case OP_2DROP:
            strcpy(opstr, "OP_2DROP") ;
            break;
        case OP_2DUP:
            strcpy(opstr, "OP_2DUP") ;
            break;
        case OP_3DUP:
            strcpy(opstr, "OP_3DUP") ;
            break;
        case OP_2OVER:
            strcpy(opstr, "OP_2OVER") ;
            break;
        case OP_2ROT:
            strcpy(opstr, "OP_2ROT") ;
            break;
        case OP_2SWAP:
            strcpy(opstr, "OP_2SWAP") ;
            break;
        case OP_CAT:
            strcpy(opstr, "OP_CAT") ;
            break;
        case OP_SUBSTR:
            strcpy(opstr, "OP_SUBSTR") ;
            break;
        case OP_LEFT:
            strcpy(opstr, "OP_LEFT") ;
            break;
        case OP_RIGHT:
            strcpy(opstr, "OP_RIGHT") ;
            break;
        case OP_SIZE:
            strcpy(opstr, "OP_SIZE") ;
            break;
        case OP_INVERT:
            strcpy(opstr, "OP_INVERT") ;
            break;
        case OP_AND:
            strcpy(opstr, "OP_AND") ;
            break;
        case OP_OR:
            strcpy(opstr, "OP_OR") ;
            break;
        case OP_XOR:
            strcpy(opstr, "OP_XOR") ;
            break;
        case OP_EQUAL:
            strcpy(opstr, "OP_EQUAL") ;
            break;
        case OP_EQUALVERIFY:
            strcpy(opstr, "OP_EQUALVERIFY") ;
            break;
        case OP_1ADD:
            strcpy(opstr, "OP_1ADD") ;
            break;
        case OP_1SUB:
            strcpy(opstr, "OP_1SUB") ;
            break;
        case OP_2MUL:
            strcpy(opstr, "OP_2MUL") ;
            break;
        case OP_2DIV:
            strcpy(opstr, "OP_2DIV") ;
            break;
        case OP_NEGATE:
            strcpy(opstr, "OP_NEGATE") ;
            break;
        case OP_ABS:
            strcpy(opstr, "OP_ABS") ;
            break;
        case OP_NOT:
            strcpy(opstr, "OP_NOT") ;
            break;
        case OP_0NOTEQUAL:
            strcpy(opstr, "OP_0NOTEQUAL") ;
            break;
        case OP_ADD:
            strcpy(opstr, "OP_ADD") ;
            break;
        case OP_SUB:
            strcpy(opstr, "OP_SUB") ;
            break;
        case OP_MUL:
            strcpy(opstr, "OP_MUL") ;
            break;
        case OP_DIV:
            strcpy(opstr, "OP_DIV") ;
            break;
        case OP_MOD:
            strcpy(opstr, "OP_MOD") ;
            break;
        case OP_LSHIFT:
            strcpy(opstr, "OP_LSHIFT") ;
            break;
        case OP_RSHIFT:
            strcpy(opstr, "OP_RSHIFT") ;
            break;
        case OP_BOOLAND:
            strcpy(opstr, "OP_BOOLAND") ;
            break;
        case OP_BOOLOR:
            strcpy(opstr, "OP_BOOLOR") ;
            break;
        case OP_NUMEQUAL:
            strcpy(opstr, "OP_NUMEQUAL") ;
            break;
        case OP_NUMEQUALVERIFY:
            strcpy(opstr, "OP_NUMEQUALVERIFY") ;
            break;
        case OP_NUMNOTEQUAL:
            strcpy(opstr, "OP_NUMNOTEQUAL") ;
            break;
        case OP_LESSTHAN:
            strcpy(opstr, "OP_LESSTHAN") ;
            break;
        case OP_GREATERTHAN:
            strcpy(opstr, "OP_GREATERTHAN") ;
            break;
        case OP_LESSTHANOREQUAL:
            strcpy(opstr, "OP_LESSTHANOREQUAL") ;
            break;
        case OP_GREATERTHANOREQUAL:
            strcpy(opstr, "OP_GREATERTHANOREQUAL") ;
            break;
        case OP_MIN:
            strcpy(opstr, "OP_MIN") ;
            break;
        case OP_MAX:
            strcpy(opstr, "OP_MAX") ;
            break;
        case OP_WITHIN:
            strcpy(opstr, "OP_WITHIN") ;
            break;
        case OP_RIPEMD160:
            strcpy(opstr, "OP_RIPEMD160") ;
            break;
        case OP_SHA1:
            strcpy(opstr, "OP_SHA1") ;
            break;
        case OP_SHA256:
            strcpy(opstr, "OP_SHA256") ;
            break;
        case OP_HASH160:
            strcpy(opstr, "OP_HASH160") ;
            break;
        case OP_HASH256:
            strcpy(opstr, "OP_HASH256") ;
            break;
        case OP_CODESEPARATOR:
            strcpy(opstr, "OP_CODESEPARATOR") ;
            break;
        case OP_CHECKSIG:
            strcpy(opstr, "OP_CHECKSIG") ;
            break;
        case OP_CHECKSIGVERIFY:
            strcpy(opstr, "OP_CHECKSIGVERIFY") ;
            break;
        case OP_CHECKMULTISIG:
            strcpy(opstr, "OP_CHECKMULTISIG") ;
            break;
        case OP_CHECKMULTISIGVERIFY:
            strcpy(opstr, "OP_CHECKMULTISIGVERIFY") ;
            break;
        case OP_CHECKLOCKTIMEVERIFY:
            strcpy(opstr, "OP_CHECKLOCKTIMEVERIFY") ;
            break;
        case OP_CHECKSEQUENCEVERIFY:
            strcpy(opstr, "OP_CHECKSEQUENCEVERIFY") ;
            break;
        case OP_PUBKEYHASH:
            strcpy(opstr, "OP_PUBKEYHASH") ;
            break;
        case OP_PUBKEY:
            strcpy(opstr, "OP_PUBKEY") ;
            break;
        case OP_INVALIDOPCODE:
            strcpy(opstr, "OP_INVALIDOPCODE") ;
            break;
        case OP_RESERVED:
            strcpy(opstr, "OP_RESERVED") ;
            break;
        case OP_VER:
            strcpy(opstr, "OP_VER") ;
            break;
        case OP_VERIF:
            strcpy(opstr, "OP_VERIF") ;
            break;
        case OP_VERNOTIF:
            strcpy(opstr, "OP_VERNOTIF") ;
            break;
        case OP_RESERVED1:
            strcpy(opstr, "OP_RESERVED1") ;
            break;
        case OP_RESERVED2:
            strcpy(opstr, "OP_RESERVED2") ;
            break;
        case OP_NOP1:
            strcpy(opstr, "OP_NOP1") ;
            break;
        case OP_NOP4:
            strcpy(opstr, "OP_NOP4") ;
            break;
        case OP_NOP5:
            strcpy(opstr, "OP_NOP5") ;
            break;
        case OP_NOP6:
            strcpy(opstr, "OP_NOP6") ;
            break;
        case OP_NOP7:
            strcpy(opstr, "OP_NOP7") ;
            break;
        case OP_NOP8:
            strcpy(opstr, "OP_NOP8") ;
            break;
        case OP_NOP9:
            strcpy(opstr, "OP_NOP9") ;
            break;
        case OP_NOP10:
            strcpy(opstr, "OP_NOP10") ;
            break;
	default:
            debug_print("OPCODE_STR: error: unknown opcode 0x%x\n", opcode) ;
            strcpy(opstr, "OP_UNKNOWN") ;
    }
    return opstr ;
}
