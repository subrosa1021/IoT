CC		= gcc -g
CURL	= -lcurl
OPT		= -L/path/lib -I/path/include/

all: f_curl

f_curl: f_curl.o
	$(CC) -o $@ $< $(CURL)

f_curl.o: f_curl.c
	$(CC) $(OPT) -c $<

clean:
	rm -rf f_curl *.o
