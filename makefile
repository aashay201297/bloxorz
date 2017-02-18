all: sample2D

sample2D: aashay.cpp
	g++ -g -o sample2D aashay.cpp -lglfw -lGLEW -lGL -ldl

clean:
	rm sample2D
