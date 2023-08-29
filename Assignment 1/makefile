pairsofwords: main.o program.o hash_table.o linked_list.o crc64.o getWord.o
	cc -o $@ $^

main.o: main.c program.h
	cc $< -c

program.o: program.c program.h
	cc $< -c

hash_table.o: hash_table.c hash_table.h
	cc $< -c

linked_list.o: linked_list.c linked_list.h
	cc $< -c

crc64.o: crc64.c crc64.h
	cc $< -c

getWord.o: getWord.c getWord.h
	cc $< -c

.PHONY: clean
clean:
	rm pairsofwords main.o program.o hash_table.o linked_list.o crc64.o getWord.o