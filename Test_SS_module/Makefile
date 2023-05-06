all: ssss-split ssss-combine

ssss-split: ssss-split.c
	gcc -c ssss-split.c
	gcc ssss-split.o -W -Wall -O2 -lgmp -o ssss-split
	rm ssss-split.o

ssss-combine: ssss-combine.c

	gcc -c ssss-combine.c
	gcc ssss-combine.o -W -Wall -O2 -lgmp -o ssss-combine
	rm ssss-combine.o

clean:
	rm -rf ssss-split ssss-combine