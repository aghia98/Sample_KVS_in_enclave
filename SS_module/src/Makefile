all: ssss-split

ssss-split:
	g++ -c ssss-split.cpp
	ar rcs libshamir_split.a *.o
	mv libshamir_split.a ..
	rm *.o
	