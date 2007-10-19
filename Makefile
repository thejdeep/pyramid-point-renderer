.SUFFIXES: .cc .c

# flags for C++ compiler:
MODE = final


INCLUDEDIRS = -I.

ifeq ($(MODE), timing)
CXXFLAGS = -g -O3 -Wall -Wno-deprecated \
		  -DTIMING 
else
CXXFLAGS = -g -O3 -Wall -Wno-deprecated
endif

CCFLAGS = -g -O3 -Wall

# compiler names
CXX = g++
CC = gcc

LIBDIRS = -L/usr/local/lib \
	-L/usr/X11R6/lib

MATLIB     = -lm
GLLIBS     = -lGL -lGLU
GLUTLIB    = -lglut
XLIBS      = -lXext -lX11 -lXi 

LIBSLIST = $(GLLIBS) $(GLUTLIB) $(XLIBS) $(MATLIB)

OBJFILES = timer.o point_render.o camera.o glslkernel.o object.o \
	point_based_render.o pyramid_point_render.o ewa_surface_splatting.o \
	pyramid_triangle_renderer.o triangle_renderer.o ply.o GLee.o

CODES =  point_render.cc camera.cc timer.c glslkernel.cc object.cc\
	point_based_render.cc pyramid_point_render.cc ewa_surface_splatting.cc \
	pyramid_triangle_renderer.cc triangle_renderer.cc ply.c GLee.cpp

###################################

point_render: $(OBJFILES)
	@echo "Linking ...."
	$(CXX) $(CXXFLAGS) $(OBJFILES) $(LIBDIRS) $(LIBSLIST) -o point_render 

.cc.o: $*.h
	@echo "Compiling C++ code ...."
	$(CXX) -c $(INCLUDEDIRS) $(CXXFLAGS) $*.cc

.c.o: $*.h
	@echo "Compiling C code ...."
	$(CC) -c $(INCLUDEDIRS) $(CCFLAGS) $*.c

depend:
	$(CC) $(CXXFLAGS) -M *.cc > .depend

clean:	
	rm *.o point_render *~

all: clean depend point_render

ifeq (.depend,$(wildcard .depend))
include .depend
endif

