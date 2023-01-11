HEADERS = bitcoin.h ansi.h ripemd160.h memzero.h script.h sha1.h
LIBS = bitcoinlib.o bccryptolib.o process_script.o sha1.o 

default: blkdmp foo nblocks parsetxdata dumphex txlookup qd whalescan btchist coinapidb txdump scripttest txlist reversehexdb txhexfiles

blkdmp.o: blkdmp.c $(HEADERS)
	c99 -c blkdmp.c -o blkdmp.o -lcrypto -g `mysql_config --cflags --libs`

foo.o: foo.c $(HEADERS)
	c99 -c foo.c -o foo.o -lcrypto -g `mysql_config --cflags --libs`

coinapidb.o: coinapidb.c $(HEADERS)
	c99 -c coinapidb.c -o coinapidb.o -lcrypto -g `mysql_config --cflags --libs`

scripttest.o: scripttest.c $(HEADERS)
	c99 -c scripttest.c -o scripttest.o -lcrypto -g `mysql_config --cflags --libs`

qd.o: qd.c $(HEADERS)
	c99 -c qd.c -o qd.o -lcrypto -g `mysql_config --cflags --libs`

txdump.o: txdump.c $(HEADERS)
	c99 -c txdump.c -o txdump.o -lcrypto -g `mysql_config --cflags --libs`

btchist.o: btchist.c $(HEADERS)
	c99 -c btchist.c -o btchist.o -lcrypto -g `mysql_config --cflags --libs`

whalescan.o: whalescan.c $(HEADERS)
	c99 -c whalescan.c -o whalescan.o -lcrypto -g `mysql_config --cflags --libs`

nblocks.o: nblocks.c $(HEADERS)
	c99 -c nblocks.c -o nblocks.o -g `mysql_config --cflags --libs`

parsetxdata.o: parsetxdata.c $(HEADERS)
	c99 -c parsetxdata.c -o parsetxdata.o -g `mysql_config --cflags --libs`

reversehexdb.o: reversehexdb.c $(HEADERS)
	c99 -c reversehexdb.c -o reversehexdb.o -g `mysql_config --cflags --libs`

txhexfiles.o: txhexfiles.c $(HEADERS)
	c99 -c txhexfiles.c -o txhexfiles.o -g `mysql_config --cflags --libs`

txlist.o: txlist.c $(HEADERS)
	c99 -c txlist.c -o txlist.o -g `mysql_config --cflags --libs`

dumphex.o: dumphex.c $(HEADERS)
	c99 -c dumphex.c -o dumphex.o -g `mysql_config --cflags --libs`

memzero.o: memzero.c $(HEADERS)
	c99 -c memzero.c -o memzero.o -g

ripemd160.o: ripemd160.c $(HEADERS)
	c99 -c ripemd160.c -o ripemd160.o -g

bitcoinlib.o: bitcoinlib.c $(HEADERS) 
	c99 -c bitcoinlib.c -o bitcoinlib.o -g `mysql_config --cflags --libs`

process_script.o: process_script.c $(HEADERS) 
	c99 -c process_script.c -o process_script.o -g `mysql_config --cflags --libs`

bcsqllib.o: bcsqllib.c $(HEADERS)
	c99 -c bcsqllib.c -o bcsqllib.o -g `mysql_config --cflags --libs`

bccryptolib.o: bccryptolib.c $(HEADERS)
	c99 -c bccryptolib.c -o bccryptolib.o -g `mysql_config --cflags --libs`

sha1.o: sha1.c $(HEADERS)
	c99 -c sha1.c -o sha1.o -g `mysql_config --cflags --libs`

txlookup.o: txlookup.c $(HEADERS)
	gcc -c txlookup.c -o txlookup.o -g `mysql_config --cflags --libs`

blkdmp: blkdmp.o $(LIBS)
	c99 blkdmp.o $(LIBS) -o blkdmp -lcrypto -g `mysql_config --cflags --libs`

foo: foo.o $(LIBS)
	c99 foo.o $(LIBS) -o foo -lcrypto -g `mysql_config --cflags --libs`

coinapidb: coinapidb.o bitcoinlib.o bccryptolib.o
	c99 coinapidb.o $(LIBS) -o coinapidb -lcrypto -g `mysql_config --cflags --libs`

qd: qd.o $(LIBS) 
	c99 qd.o $(LIBS) -o qd -lcrypto -g `mysql_config --cflags --libs`

scripttest: scripttest.o bitcoinlib.o bccryptolib.o process_script.o sha1.o
	c99 scripttest.o $(LIBS) -o scripttest -lcrypto -g `mysql_config --cflags --libs`

txdump: txdump.o bitcoinlib.o bccryptolib.o
	c99 txdump.o $(LIBS) -o txdump -lcrypto -g `mysql_config --cflags --libs`

btchist: btchist.o bitcoinlib.o bccryptolib.o
	c99 btchist.o $(LIBS) -o btchist -lcrypto -ljson-c -g `mysql_config --cflags --libs`

whalescan: whalescan.o bitcoinlib.o bccryptolib.o
	c99 whalescan.o $(LIBS) -o whalescan -lcrypto -g `mysql_config --cflags --libs`

nblocks: nblocks.o bitcoinlib.o bccryptolib.o
	c99 nblocks.o $(LIBS) -o nblocks -lcrypto -g `mysql_config --cflags --libs`

parsetxdata: parsetxdata.o  $(LIBS)
	c99 parsetxdata.o $(LIBS) -o parsetxdata -lcrypto -g `mysql_config --cflags --libs`

reversehexdb: reversehexdb.o  $(LIBS)
	c99 reversehexdb.o $(LIBS) -o reversehexdb -lcrypto -g `mysql_config --cflags --libs`

txhexfiles: txhexfiles.o  $(LIBS)
	c99 txhexfiles.o $(LIBS) -o txhexfiles -lcrypto -g `mysql_config --cflags --libs`

txlist: txlist.o $(LIBS)
	c99 txlist.o $(LIBS) -o txlist -lcrypto -g `mysql_config --cflags --libs`

dumphex: dumphex.o dumphex.o bitcoinlib.o bccryptolib.o
	c99 dumphex.o $(LIBS) -o dumphex -lcrypto -g `mysql_config --cflags --libs`

txlookup: txlookup.o txlookup.o bccryptolib.o process_script.o $(LIBS)
	gcc txlookup.o $(LIBS) -o txlookup -lcrypto -g `mysql_config --cflags --libs`

clean:
	-rm -f parsetxdata.o
	-rm -f parsetxdata
	-rm -f blkdmp.o
	-rm -f blkdmp
	-rm -f dumphex.o
	-rm -f dumphex
	-rm -f txlookup.o
	-rm -f txlookup
	-rm -f qd.o
	-rm -f qd
	-rm -f whalescan.o
	-rm -f whalescan
	-rm -f txdump.o
	-rm -f txdump
	-rm -f hexfix.o
	-rm -f hexfix
	-rm -f bitcoinlib.o
	-rm -f ripremd160.o
	-rm -f memzero.o
	-rm -f process_script.o
	-rm -f bccryptolib.o
	-rm -f bccryptolib
	-rm -f btchist.o
	-rm -f btchist
	-rm -f coinapidb.o
	-rm -f coinapidb
	-rm -f foo.o
	-rm -f foo
	-rm -f nblocks.o
	-rm -f nblocks
	-rm -f scripttest.o
	-rm -f scripttest
	-rm -f sha1.o
	-rm -f sha1
	-rm -f txlist.o
	-rm -f txlist
