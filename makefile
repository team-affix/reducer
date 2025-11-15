release:
	mkdir -p build
	g++ -std=c++20 -I"." ./src/*.cpp -o ./build/main

debug:
	mkdir -p build
	cd ./lc && make
	g++ -std=c++20 -fexceptions -g -DUNIT_TEST -I"." -I./lc/include/ ./lc/build/liblc.a ./src/*.cpp -o ./build/main

clean:
	rm -rf ./build
