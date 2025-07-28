.PHONY=run

run: kzg
	./kzg

kzg: src/*.cpp miracl-core/cpp/core.a ntl/src/ntl.a
	g++ src/*.cpp miracl-core/cpp/core.a ntl/src/ntl.a -Intl/include -Imiracl-core/cpp -lgmp -o kzg

miracl-core/cpp/core.a:
	cd miracl-core/cpp && python3 config64.py -o 41

ntl/src/ntl.a:
	cd ntl/src && ./configure && make
