assignment3: main.o assignment3.o
	cc -o $@ $^

main.o: main.c
	cc $< -c

assignment3.o: assignment3.c
	cc $< -c

.PHONY: clean
clean:
	rm linenum main.o assignment3.o