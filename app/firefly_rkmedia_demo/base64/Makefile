all:
	g++ -c base64.cpp
	ar rs libbase_64.a base64.o
	g++ main.cpp -L. -lbase_64 -Ibase64 -o main
