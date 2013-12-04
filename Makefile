CFLAGS= -lm -g -pg -std=c++0x -Wall -march=native -pipe -lGLU -lglut -pthread 
FILES=main.cpp point.cpp mesh.cpp screen.cpp mesh_draw.cpp
mps:$(FILES) *.h
	g++ $(CFLAGS) $(FILES) -o mps
