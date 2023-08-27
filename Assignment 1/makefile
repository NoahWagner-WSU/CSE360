main: main.o program.o hash_table.o linked_list.o
	cc $^ -o $@

main.o: main.c program.o hash_table.o linked_list.o
	cc $< -c $@

program.o: program.c program.h hash_table.o linked_list.o
	cc $< -c $@

hash_table.o: hash_table.c hash_table.h linked_list.o
	cc $< -c $@

linked_list.o: linked_list.c linked_list.h
	cc $< -c $@

.PHONY: clean
clean:
	rm main main.o program.o hash_table.o linked_list.o