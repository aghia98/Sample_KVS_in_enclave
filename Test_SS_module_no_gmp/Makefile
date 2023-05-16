ssss-split:
	g++ -c d_string.cpp split.cpp common.cpp ssss-split.cpp
	g++ d_string.o split.o ssss-split.o common.o -o shamir-split
	rm d_string.o split.o ssss-split.o common.o

ssss-combine:
	g++ -c d_string.cpp combine.cpp strtok.cpp ssss-combine.cpp
	g++ d_string.o combine.o strtok.o ssss-combine.o -o shamir-combine
	rm d_string.o strtok.o combine.o ssss-combine.o 

clean:
	rm shamir-split shamir-combine