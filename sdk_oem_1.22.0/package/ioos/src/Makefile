MODEFILE=cgi.o
STRIP=$(CROSS)strip
TARGET=ioos
CFLAGS= -fPIC -Wall -Iinclude
ALL: clean $(TARGET) setsysflag resetd
FILE=common.o log.o server.o arraylist.o debug.o \
 json_object.o json_tokener.o json_util.o linkhash.o \
 printbuf.o connection.o protocol.o $(MODEFILE) crc32.o gzip.o csp.o

ioos: $(FILE) main.c
	$(CC) -O2 -o $(TARGET)  main.c $(FILE) $(LDFLAGS) -lz -luci -lubox -lshare $(CFLAGS) 
resetd: resetd.c
	$(CC) -O2 -o resetd resetd.c $(LDFLAGS) -lshare $(CFLAGS)  
setsysflag: setsysflag.c
	$(CC) -O2 -o setsysflag setsysflag.c $(LDFLAGS) -lshare $(CFLAGS)  
%.o:%.c
	$(CC) -c -o $@ $^ $(CFLAGS) 
clean: 
	rm -f *.o ioos tspeed resetd
