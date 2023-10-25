assignment7: main.o assignment7.o
	cc -g -o $@ $^

main.o: main.c
	cc -g $< -c

assignment7.o: assignment7.c
	cc -g $< -c

.PHONY: clean
clean:
	rm assignment7 main.o assignment7.o