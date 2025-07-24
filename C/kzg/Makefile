.PHONY=run

run: kzg
	./kzg

kzg: *.c
	gcc -std=c99 *.c core.a -I../core/c -o kzg
