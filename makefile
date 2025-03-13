all:
	mkdir -p build
	g++ -std=c++20 -g -I"." ./src/*.cpp -o ./build/main

clean:
	rm -rf ./build
