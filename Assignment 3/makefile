assignment3: main.o assignment3.o
	cc -g -o $@ $^

main.o: main.c
	cc -g $< -c

assignment3.o: assignment3.c
	cc -g $< -c

.PHONY: clean
clean:
	rm assignment3 main.o assignment3.o