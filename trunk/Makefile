OS = linux

#include Makefile.in

.SUFFIXES: .cc .c

OS = linux

# compiler names
CC = gcc
CXX = g++

VCGDIR = $(HOME)/devel/vcglib/

ifeq ($(OS), windows)
INCLUDEDIRS = -I$(VCGDIR) -I. -I.. -I/usr/include/GL
OBJDIR = ./objs
else
INCLUDEDIRS = -I$(VCGDIR) -I/usr/include/GL -I./ -I../
OBJDIR = objs
endif

CXXFLAGS = -g -O3 -Wall -Wno-deprecated

CCFLAGS = -g -O3 -Wall

OBJECTS = application.o \
	main.o \
	point_based_renderer.o \
	plylib.o \
	object.o \
	trackball.o \
	trackmode.o \
	pyramid_point_renderer_base.o \
	pyramid_point_renderer.o \
	pyramid_point_renderer_color.o
#	pyramid_point_renderer_elipse.o \
#	pyramid_point_renderer_er.o

# SUBDIRS = pyramid_point_renderer_base \
# 	pyramid_point_renderer \
# 	pyramid_point_renderer_color \
# 	pyramid_point_renderer_elipse \
# 	pyramid_templates

OBJS =	plylib.o \
	application.o \
	main.o \
	point_based_renderer.o \
	object.o \
	trackball.o \
	trackmode.o


CODES =	application.cc \
	main.cc \
	point_based_renderer.cc \
	object.cc \
	$(VCGDIR)/wrap/gui/trackball.cpp \
	$(VCGDIR)/wrap/gui/trackmode.cpp \
	$(VCGDIR)/wrap/ply/plylib.cpp \
	pyramid_point_renderer_base.cc \
	pyramid_point_renderer.cc \
	pyramid_point_renderer_color.cc
#	pyramid_point_renderer_er.cc
#	pyramid_point_renderer_elipse/pyramid_point_renderer_elipse.cc \



OBJ = $(patsubst %,$(OBJDIR)/%,$(OBJECTS))

OBJ2 = $(patsubst %,$(OBJDIR)/%,$(OBJS))

ifeq ($(OS), windows)
LIBDIRS = 
else
LIBDIRS = -L/usr/lib64/	
endif

MATLIB     = -lm
ifeq ($(OS), windows)
GLLIBS	  = -lopengl32 -lglu32
GLUTLIB    = -lGLEW -lglut32win
else
GLLIBS     = -lGLU -lGL
GLUTLIB    = -lGLEW -lglut
endif
XLIBS      = -lXext -lX11 -lXi -lpthread

LIBLIST = $(GLUTLIB) $(GLLIBS) $(MATLIB)

HEADERS = application.h \
	main.h \
	point_based_renderer.h \
	object.h \
	pyramid_point_renderer_base.h \
	pyramid_point_renderer.h \
	pyramid_point_renderer_color.h \
	surfel.hpp\
	IOSUrfels.hpp
#	pyramid_point_renderer_er.h\
#	pyramid_point_renderer_elipse/pyramid_point_renderer_elipse.h \

###################################

all: ppr trackball plylib

$(OBJDIR)/%.o: %.cc
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/trackmode.o: $(VCGDIR)/wrap/gui/trackmode.cpp
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/trackball.o: $(VCGDIR)/wrap/gui/trackball.cpp
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/plylib.o: $(VCGDIR)/wrap/ply/plylib.cpp
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)


$(OBJDIR)/%.o: %.c
	@echo
	@echo "Compiling C code : $@"
	$(CC) -c -o $@ $< $(INCLUDEDIRS) $(CCFLAGS)

ppr: $(OBJ) 
	$(CXX) $(OBJ) -o $@ $(CXXFLAGS) $(LIBDIRS) $(LIBLIST)
	# for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} ) ; done
	# @echo
	# @echo "Linking :  $@"
	# $(CXX) $(OBJ2) -o $@ $(CXXFLAGS) $(LIBDIRS) $(LIBLIST)

trackball: $(OBJDIR)/trackmode.o $(OBJDIR)/trackball.o
plylib: $(OBJDIR)/plylib.o

clean:
	for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} clean ) ; done
	rm -f *.o $(OBJDIR)/*.o *~ core $(INCDIR)/*~ ppr

depend: $(CODES)
	makedepend $(INCLUDEDIRS) $(CODES)

