ssss-split:
	g++ -c d_string.cpp split.cpp common.cpp ssss-split.cpp
	g++ d_string.o split.o ssss-split.o common.o -o shamir-split
	rm d_string.o split.o ssss-split.o common.o

ssss-combine:
	g++ -c d_string.cpp combine.cpp strtok.cpp common.cpp ssss-combine.cpp
	g++ *.o -o shamir-combine
	rm *.o 

clean:
	rm shamir-split shamir-combine