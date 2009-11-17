all: libpopt.a optimize_test

clean:
	$(RM) *.o libpopt.a optimize_test

%.o: %.cc
	$(CXX) -c -O3 -g -Wall -Wno-sign-compare $<

libpopt.a: optimize.o
	$(AR) rcs libpopt.a optimize.o

optimize_test: optimize_test.o
	$(CXX) optimize_test.o libpopt.a -o optimize_test
