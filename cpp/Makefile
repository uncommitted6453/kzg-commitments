.PHONY=run

run: kzg
	./kzg

kzg: *.cpp
	g++ *.cpp ../core/cpp/core.a -I../core/cpp -lntl -lgmp -o kzg
