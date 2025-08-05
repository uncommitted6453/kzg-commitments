.PHONY=run

run: kzg-bn158
	./kzg-bn158

KZG_SRC=src/*.cpp
KZG_INCLUDE=-Intl/include -Imiracl-core/cpp
KZG_LIB=miracl-core/cpp/core.a ntl/src/ntl.a -lgmp

kzg-bn158: src/*.cpp miracl-core/cpp/config_curve_BN158.h ntl/src/ntl.a
	g++ $(KZG_SRC) $(KZG_LIB) $(KZG_INCLUDE) -Iconfig/curve_BN158 -o $@

kzg-bn254: src/*.cpp miracl-core/cpp/config_curve_BN254.h ntl/src/ntl.a
	g++ $(KZG_SRC) $(KZG_LIB) $(KZG_INCLUDE) -Iconfig/curve_BN254 -o $@

miracl-core/cpp/config_curve_BN158.h:
	cd miracl-core/cpp && python3 config64.py -o 41

miracl-core/cpp/config_curve_BN254.h:
	cd miracl-core/cpp && python3 config64.py -o 28

ntl/src/ntl.a:
	cd ntl/src && ./configure && make
