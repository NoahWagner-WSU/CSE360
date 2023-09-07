linenum: main.o lineNum.o
	cc -o $@ $^

main.o: main.c
	cc $< -c

lineNum.o: lineNum.c
	cc $< -c

.PHONY: clean
clean:
	rm linenum main.o lineNum.o