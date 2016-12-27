CC		= gcc -g
LIBS	= -lpthread -lwiringPi
CURL 	= -lcurl
OPT		= -L/path/lib -I/path/include/

all:webserver

webserver:webserver.o
	$(CC) -o $@ $< $(LIBS) $(CURL)

webserver.o:webserver.c
	$(CC) $(OPT) -c $<

clean:
	rm -rf webserver *.o

