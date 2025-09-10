debug:
	mkdir -p build
	g++ -std=c++20 -fexceptions -g -DUNIT_TEST -I"." ./src/*.cpp -o ./build/main

release:
	mkdir -p build
	g++ -std=c++20 -I"." ./src/*.cpp -o ./build/main

clean:
	rm -rf ./build
