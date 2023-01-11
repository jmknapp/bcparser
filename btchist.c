#define HISTDIR "coinhist"
#define LOOKBACKDAYS 1
#define SECSPERDAY 86400
#define BTCSYMBOL "BITSTAMP_SPOT_BTC_USD"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "bitcoin.h"
#include <mysql.h>
#include <json-c/json.h>

int main(int argc, char **argv) {
    time_t utime, gmt0, gmt1 ;
    struct tm ts ;
    int iday ;
    MYSQL *con = mysql_init(NULL);
    MYSQL_RES *res, *res2 ;
    MYSQL_ROW row ;
    char query[MAXQUERY] ;
    FILE *ifd ;
    char histfile[100] ;
    int i ;
    char *jsonbuf ;
    int jsonsize ;
    double open, high, low, close ;
    char date[80] ;
    int ihist ;
    int nhist ;
    bool opened ;
    int n ;

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

    time(&utime) ;  // cuurent unix epoch
    ts = *localtime(&utime);

    // set clock time to 00:00:00
    ts.tm_hour = 0 ;
    ts.tm_min = 0 ;
    ts.tm_sec = 0 ;
    gmt0 = mktime(&ts);  // epoch at 00:00:00 of current UTC day

    // for all lookback days up to today
    for (iday = -LOOKBACKDAYS ; iday <= 0 ; iday++) {
	time_t ut0, ut1 ;

	ut0 = gmt0 + iday*SECSPERDAY ;  // start epoch of day
	ut1 = ut0 + SECSPERDAY ;  // end epoch of day
        ts = *localtime(&ut0);

        sprintf(date, "%04d-%02d-%02d", ts.tm_year+1900,  ts.tm_mon+1, ts.tm_mday) ;  // UTC date yyyy-mm-dd

	// get all coinAPI files collected on this day
	sprintf(query,"select coinapifile from coinapihist where timestamp >= %lu and timestamp < %lu order by coinapifile", ut0, ut1) ;
        if (mysql_query(con, query)) {
            finish_with_error(con);
        }
        res = mysql_store_result(con);
        nhist = mysql_num_rows(res) ;  // number of coinAPI files

	opened = false ;
	high = 0 ;
	low = 1e8 ;
	close = 0 ;
	open = 0 ;

	// for each coinAPI file get BTC price
	for (ihist = 0 ; ihist < nhist ; ihist++) {
	    struct json_object *parsed_json ;
	    struct json_object *symbol_rec ;
	    struct json_object *symbol_id ;
	    struct json_object *ask_price ;
	    int nsyms ;
	    char symbolstr[80] ;
	    double ask ;

            while ((row = mysql_fetch_row(res))) {
		sprintf(histfile,"%s/%s/%s", HOME, HISTDIR,row[0]) ;
		ifd = fopen(histfile, "r") ;
                if (ifd == NULL) {
                    fprintf(stderr, "error: can't open %s for reading\n", histfile) ;
                    mysql_close(con) ;
                    exit(1) ;
                }
                //printf("%s\n", histfile);
		fseek(ifd, 0, SEEK_END) ;
		jsonsize = ftell(ifd);
		rewind(ifd) ;
		jsonbuf = malloc(jsonsize) ;

                fread(jsonbuf, jsonsize, 1, ifd) ;

		parsed_json = json_tokener_parse(jsonbuf) ;

		nsyms = json_object_array_length(parsed_json) ;
		//printf("nsyms = %d\n", nsyms) ;

		i = 0 ;
		symbol_rec = json_object_array_get_idx(parsed_json, i) ;
		json_object_object_get_ex(symbol_rec, "symbol_id", &symbol_id) ;
		strcpy(symbolstr, json_object_get_string(symbol_id)) ;
		while (i < nsyms && strcmp(symbolstr, BTCSYMBOL) != 0) {
		    i++ ;
		    symbol_rec = json_object_array_get_idx(parsed_json, i) ;
		    json_object_object_get_ex(symbol_rec, "symbol_id", &symbol_id) ;
		    strcpy(symbolstr, json_object_get_string(symbol_id)) ;
		}
		if (i != nsyms) {
		    json_object_object_get_ex(symbol_rec, "ask_price", &ask_price) ;
		    ask = json_object_get_double(ask_price) ;
		    //printf("%s %f\n", symbolstr, ask) ;
		    //json_object_object_get_ex(parsed_json, "symbol_id", &symbol_id) ;
		    if (ask < low)
			low = ask ;
		    if (ask > high)
			high = ask ;
		    close = ask ;
		    if (opened == false) {
			open = ask ;
			opened = true ;
		    }
		}
		free(jsonbuf) ;
                fclose(ifd) ;
	    }
	}
	//printf("%s: high=%f, low=%f, close=%f\n", date, high, low, close) ;
	if (opened)  {
	    sprintf(query,"select udate from btchist where udate=\"%s\"", date) ;
            if (mysql_query(con, query)) {
                finish_with_error(con);
            }
            res2 = mysql_store_result(con);
            n = mysql_num_rows(res2) ;
	    if (n == 0) {
                // not there yet, insert
                sprintf(query, "insert into btchist (udate,open,high,low,close) VALUES (\"%s\",%f,%f,%f,%f)", date, open, high, low, close) ;
                printf("%s\n", query) ;
                if (mysql_query(con, query)) {
                    finish_with_error(con);
                }
            }
	    else {
                // is there already, update
                sprintf(query, "update btchist set open=%f, high=%f, low=%f, close=%f where udate=\"%s\"", open, high, low, close, date) ;
                printf("%s\n", query) ;
                if (mysql_query(con, query)) {
                    finish_with_error(con);
                }
	    }
	}
    }

    mysql_close(con) ;
}
