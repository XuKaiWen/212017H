CC	= mpicc
CFLAG	= -g

SRC	= sequc_hello mpisum ring pingpong

all : $(SRC)

%   : %.c
	$(CC) $(CFLAG) -o $@ $<

clean:
	rm -f *~ *.o a.out 

cleanall:
	rm -f *~ *.o a.out
	rm -f $(SRC)



