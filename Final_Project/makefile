.PHONY : all
all : myftp myftpserve

myftp: myftp.o
	cc -g -o $@ $^

myftpserve: myftpserve.o
	cc -g -o $@ $^

myftp.o: myftp.c myftp.h
	cc -g $< -c

myftpserve.o: myftpserve.c myftp.h
	cc -g $< -c

.PHONY: clean
clean:
	rm myftp myftpserve myftp.o myftpserve.o