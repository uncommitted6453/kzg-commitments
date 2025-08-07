.PHONY=all clean config_bn158 config_bn254 config_bls12381

KZG_SRC=$(wildcard src/*.cpp)
KZG_H=$(wildcard src/*.h)
KZG_OBJ=$(patsubst src/%.cpp, obj/%.o, $(KZG_SRC))
KZG_LIB=miracl-core/cpp/core.a ntl/src/ntl.a

all: testing/testing

testing/testing: testing/testing.cpp lib/kzg-bn254.a lib/core.a lib/ntl.a
	g++ testing/testing.cpp -Iinclude lib/kzg-bn254.a lib/core.a lib/ntl.a -lgmp -o $@
	./testing/testing

example/example: example/example.cpp lib/kzg-bn254.a lib/core.a lib/ntl.a
	g++ example/example.cpp -Iinclude lib/kzg-bn254.a lib/core.a lib/ntl.a -lgmp -o $@
	./$@

demo/shared/kzg-cli: demo/shared/kzg-cli.cpp lib/kzg-bn254.a lib/core.a lib/ntl.a
	g++ demo/shared/kzg-cli.cpp -Iinclude lib/kzg-bn254.a lib/core.a lib/ntl.a -lgmp -o $@

benchmark/benchmark: benchmark/benchmark.cpp lib/kzg-bls12381.a lib/core.a lib/ntl.a
	g++ benchmark/benchmark.cpp -Iinclude lib/kzg-bls12381.a lib/core.a lib/ntl.a -lgmp -o $@
	./benchmark/benchmark

benchmark/benchmark-bn158: benchmark/benchmark.cpp lib/kzg-bn158.a lib/core.a lib/ntl.a
	g++ benchmark/benchmark.cpp -Iinclude lib/kzg-bn158.a lib/core.a lib/ntl.a -lgmp -o $@

benchmark/benchmark-bn254: benchmark/benchmark.cpp lib/kzg-bn254.a lib/core.a lib/ntl.a
	g++ benchmark/benchmark.cpp -Iinclude lib/kzg-bn254.a lib/core.a lib/ntl.a -lgmp -o $@

benchmark/benchmark-bls12381: benchmark/benchmark.cpp lib/kzg-bls12381.a lib/core.a lib/ntl.a
	g++ benchmark/benchmark.cpp -Iinclude lib/kzg-bls12381.a lib/core.a lib/ntl.a -lgmp -o $@

lib/kzg-bn158.a: config_bn158 include/NTL/config.h lib/ntl.a $(KZG_OBJ) | lib
	ar rvs $@ $(KZG_OBJ)

lib/kzg-bn254.a: config_bn254 include/NTL/config.h lib/ntl.a $(KZG_OBJ) | lib
	ar rvs $@ $(KZG_OBJ)

lib/kzg-bls12381.a: config_bls12381 include/NTL/config.h lib/ntl.a $(KZG_OBJ) | lib
	ar rvs $@ $(KZG_OBJ)

obj/%.o: src/%.cpp include/kzg.h $(KZG_H) | obj
	g++ -Iinclude -c -o $@ $<

config_bn158: miracl-core/cpp/config_curve_BN158.h | lib include
	cp miracl-core/cpp/core.a lib
	cp miracl-core/cpp/*_BN158.h include
	cp miracl-core/cpp/*_B160_56.h include
	cp miracl-core/cpp/arch.h include
	cp miracl-core/cpp/core.h include
	cp miracl-core/cpp/randapi.h include
	cp config/curve_BN158/kzg_config.h include/kzg_config.h

config_bn254: miracl-core/cpp/config_curve_BN254.h | lib include
	cp miracl-core/cpp/core.a lib
	cp miracl-core/cpp/*_BN254.h include
	cp miracl-core/cpp/*_B256_56.h include
	cp miracl-core/cpp/arch.h include
	cp miracl-core/cpp/core.h include
	cp miracl-core/cpp/randapi.h include
	cp config/curve_BN254/kzg_config.h include/kzg_config.h

config_bls12381: miracl-core/cpp/config_curve_BLS12381.h | lib include
	cp miracl-core/cpp/core.a lib
	cp miracl-core/cpp/*_BLS12381.h include
	cp miracl-core/cpp/*_B384_58.h include
	cp miracl-core/cpp/arch.h include
	cp miracl-core/cpp/core.h include
	cp miracl-core/cpp/randapi.h include
	cp config/curve_BLS12381/kzg_config.h include/kzg_config.h

miracl-core/cpp/config_curve_BN158.h:
	cd miracl-core/cpp && python3 config64.py -o 41

miracl-core/cpp/config_curve_BN254.h:
	cd miracl-core/cpp && python3 config64.py -o 28

miracl-core/cpp/config_curve_BLS12381.h:
	cd miracl-core/cpp && python3 config64.py -o 31

include/kzg.h:
	cp src/kzg.h include

include/NTL/config.h:
	cp -r ntl/include/NTL include

lib/ntl.a: ntl/src/ntl.a
	cp ntl/src/ntl.a lib

ntl/src/ntl.a:
	cd ntl/src && ./configure && make

obj:
	mkdir -p obj

lib:
	mkdir -p lib

include:
	mkdir -p include

clean:
	rm -rf include obj lib
