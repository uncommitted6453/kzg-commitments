.PHONY=all clean config_bn158

KZG_SRC=$(wildcard src/*.cpp)
KZG_OBJ=$(patsubst src/%.cpp, obj/%.o, $(KZG_SRC))
KZG_LIB=miracl-core/cpp/core.a ntl/src/ntl.a

all: example/example

example/example: example/example.cpp lib/kzg-bn158.a lib/core.a lib/ntl.a
	g++ example/example.cpp -Iinclude lib/kzg-bn158.a lib/core.a lib/ntl.a -lgmp -o $@
	./example/example

lib/kzg-bn158.a: config_bn158 lib/ntl.a $(KZG_OBJ) | lib
	cp ntl/src/ntl.a lib
	cp -r ntl/include/NTL include
	ar rvs $@ $(KZG_OBJ)

lib/kzg-bn254.so: src/*.cpp miracl-core/cpp/config_curve_BN254.h ntl/src/ntl.a | lib
	g++ -fPIC $(KZG_SRC) $(KZG_INCLUDE) -Iconfig/curve_BN158 -shared -o $@

obj/%.o: src/%.cpp include/kzg.h | obj
	g++ -Iinclude -c -o $@ $<

config_bn158: miracl-core/cpp/config_curve_BN158.h | lib include
	cp miracl-core/cpp/core.a lib
	cp miracl-core/cpp/*_BN158.h include
	cp miracl-core/cpp/*_B160_56.h include
	cp miracl-core/cpp/arch.h include
	cp miracl-core/cpp/core.h include
	cp miracl-core/cpp/randapi.h include
	cp config/curve_BN158/kzg_config.h include/kzg_config.h

miracl-core/cpp/config_curve_BN158.h:
	cd miracl-core/cpp && python3 config64.py -o 41

miracl-core/cpp/config_curve_BN254.h:
	cd miracl-core/cpp && python3 config64.py -o 28

include/kzg.h:
	cp src/kzg.h include

lib/ntl.a:
	cd ntl/src && ./configure && make
	cp ntl/src/ntl.a lib

obj:
	mkdir -p obj

lib:
	mkdir -p lib

include:
	mkdir -p include

clean:
	rm -rf include obj lib
