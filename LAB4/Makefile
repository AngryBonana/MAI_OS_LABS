all:
	gcc -shared -fPIC src/lib1.c -lm -o libvar1.so
	gcc -shared -fPIC src/lib2.c -lm -o libvar2.so
	gcc  static.c -L. -lvar1 -lm -o static1.out
	gcc  static.c -L. -lvar2 -lm -o static2.out
	gcc dynamic.c -ldl -lm -o dynamic.out


clean:
	rm -f *.out *.o *.so