all: ssss-split ssss-combine

client-split: client-split.cpp
	g++ -c client-split.cpp ssss-split.cpp
	g++ client-split.o ssss-split.o -W -Wall -O2 -lgmp -o client-split
	rm ssss-split.o client-split.o

ssss-combine: ssss-combine.c

	gcc -c ssss-combine.c
	gcc ssss-combine.o -W -Wall -O2 -lgmp -o ssss-combine
	rm ssss-combine.o

clean:
	rm -rf client-split ssss-combine