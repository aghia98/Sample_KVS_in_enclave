LDFLAGS_SS += -L../SS_no_gmp_module -lshamir_combine	

ssss-split:
	g++ -c split.cpp common.cpp ssss-split.cpp
	g++ *.o -o shamir-split
	rm *.o

ssss-combine:
	g++ -c combine.cpp strtok.cpp ssss-combine.cpp
	g++ *.o -o shamir-combine
	rm *.o 

ssss-combine-with-lib:
	g++ -c ssss-combine.cpp
	g++ ssss-combine.o $(LDFLAGS_SS) -o shamir-combine
	rm *.o 

clean:
	rm shamir-split shamir-combine *o