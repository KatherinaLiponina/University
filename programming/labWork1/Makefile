all: lab1leaN3248 libleaN3248_1.so libleaN3248_2.so plugin12.so plugin5.so libexe.so


lab1leaN3248: main.o
	gcc -g -o lab1leaN3248 main.o -L. -ldl
main.o: main.c
	gcc -c main.c


libleaN3248_1.so: float_plugin.o
	gcc -shared -o libleaN3248_1.so float_plugin.o
float_plugin.o: float_plugin.c
	gcc -fPIC -c float_plugin.c
libleaN3248_2.so: float_in_bin_plug.o
	gcc -shared -o libleaN3248_2.so float_in_bin_plug.o
float_in_bin_plugin.o: float_in_bin_plug.c
	gcc -fPIC -c float_in_bin_plug.c
plugin12.so: plugin12.o
	gcc -shared -o plugin12.so plugin12.o
plugin12.o: plugin12.c
	gcc -fPIC -c plugin12.c
plugin5.so: plugin5.o
	gcc -shared -o plugin5.so plugin5.o
plugin5.o: plugin5.c
	gcc -fPIC -c plugin5.c
libexe.so: libexe.o
	gcc -shared -o libexe.so libexe.o
libexe.o: libexe.c
	gcc -fPIC -c libexe.c

clean: 
	rm *.o
