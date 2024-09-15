LDFLAGS_SS += -L../SS_no_gmp_module -lshamir_combine	


recovery:
	g++ -c recovery.cpp test_recovery.cpp
	g++ *.o -o recovery
	rm *.o 


clean:
	rm recovery