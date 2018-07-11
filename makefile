all:	
	g++ l_cas.c -O3 -std=c++11 -I /home/frb2/cpp-driver/include/ -I /home/frb2/sparse-map/ -lm -L /home/frb2/cpp-driver/build/ -lcassandra -shared -fPIC -o l_cas -g
run:
	./l_cas