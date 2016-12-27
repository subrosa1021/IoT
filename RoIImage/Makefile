CC		= g++ -g
OPENCV	= `pkg-config --libs opencv`
CURL	= -lcurl
OPT		= -L/path/lib -I/path/include/
LIBS	= -I/usr/local/inclue/
CFLAGS	= -lraspicam -lraspicam_cv

all: Smarttable_ver2

Smarttable_ver2: Smarttable_ver2.o
	$(CC) -o $@ $< $(CURL) $(OPENCV) $(LIBS) $(CFLAGS)

Smarttable_ver2.o: Smarttable_ver2.c
	$(CC) $(OPT) -c $<

clean:
	rm -rf Smarttable_ver2 *.o
