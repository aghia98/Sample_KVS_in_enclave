all: ssss-split ssss-combine

ssss-split:
	gcc -c ssss-split.c
	ar rcs libshamir_split.a *.o
	mv libshamir_split.a ..
	rm *.o

ssss-combine:
	gcc -c ssss-combine.c
	ar rcs libshamir_combine.a *.o
	mv libshamir_combine.a ..
	rm *.o
	