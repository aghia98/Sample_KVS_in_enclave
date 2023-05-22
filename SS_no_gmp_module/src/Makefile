ssss-split:
	g++ -c d_string.cpp split.cpp common.cpp
	ar rcs libshamir_split.a *.o
	mv libshamir_split.a ..
	rm *.o

ssss-combine:
	g++ -c combine.cpp strtok.cpp common.cpp
	ar rcs libshamir_combine.a *.o
	mv libshamir_combine.a ..
	rm *.o