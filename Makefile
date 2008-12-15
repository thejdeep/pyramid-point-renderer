
OS = linux

OBJECTS = application.o \
	main.o \
	matrix.o \
	object.o \
	point_based_renderer.o \
	primitives.o \
	surfels.o \
	timer.o \
	trackball.o \
	trackmode.o \
	plylib.o

SUBDIRS = pyramid_point_renderer_base \
	pyramid_point_renderer \
	pyramid_point_renderer_color \
	pyramid_templates

OBJS =	application.o \
	main.o \
	object.o \
	point_based_renderer.o \
	primitives.o \
	surfels.o \
	timer.o \
	trackball.o \
	trackmode.o \
	plylib.o \
	pyramid_point_renderer_base.o \
	pyramid_point_renderer.o \
	pyramid_point_renderer_color.o \
	pyramid_point_renderer_er.o

CODES =	application.cc \
	main.cc \
	object.cc \
	point_based_renderer.cc \
	primitives.cc \
	surfels.cc \
	timer.c \
	$(HOME)/vcglib/wrap/gui/trackball.cpp \
	$(HOME)/vcglib/wrap/gui/trackmode.cpp \
	pyramid_point_renderer/pyramid_point_renderer_base.cc \
	pyramid_point_renderer/pyramid_point_renderer.cc \
	pyramid_point_renderer_color/pyramid_point_renderer_color.cc \
	pyramid_templates/pyramid_point_renderer_er.cc

include Makefile.in

OBJ = $(patsubst %,$(OBJDIR)/%,$(OBJECTS))

OBJ2 = $(patsubst %,$(OBJDIR)/%,$(OBJS))

ifeq ($(OS), windows)
LIBDIRS = -L"$(HOME)/lcgtk/glslKernel/"
else
LIBDIRS = -L$(HOME)/lcgtk/glslKernel/
endif

MATLIB     = -lm
ifeq ($(OS), windows)
GLLIBS	  = -lopengl32 -lglu32
GLUTLIB    = -lGLee -lglslKernel -lglut32win
else
GLLIBS     = -lGLU -lGL
GLUTLIB    = -lGLee -lglslKernel -lglut
endif
XLIBS      = -lXext -lX11 -lXi -lpthread

LIBLIST = $(GLUTLIB) $(GLLIBS) $(MATLIB)

VCGDIR = $(HOME)/vcglib/wrap/gui

HEADERS = application.h \
	main.h \
	object.h \
	point_based_renderer.h \
	primitives.h \
	surfels.h \
	timer.h \
	pyramid_point_renderer/pyramid_point_renderer_base.h \
	pyramid_point_renderer/pyramid_point_renderer.h \
	pyramid_point_renderer_color/pyramid_point_renderer_color.h \
	pyramid_templates/pyramid_point_renderer_er.h

###################################

all: ppr trackball plylib

$(OBJDIR)/%.o: %.cc
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/trackmode.o: $(HOME)/vcglib/wrap/gui/trackmode.cpp
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/trackball.o: $(HOME)/vcglib/wrap/gui/trackball.cpp
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/plylib.o: $(HOME)/vcglib/wrap/ply/plylib.cpp
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)


$(OBJDIR)/%.o: %.c
	@echo
	@echo "Compiling C code : $@"
	$(CC) -c -o $@ $< $(INCLUDEDIRS) $(CCFLAGS)

ppr: $(OBJ) 
	for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} ) ; done
	@echo
	@echo "Linking :  $@"
	$(CXX) $(OBJ2) -o $@ $(CXXFLAGS) $(LIBDIRS) $(LIBLIST)

trackball: $(OBJDIR)/trackmode.o $(OBJDIR)/trackball.o
plylib: $(OBJDIR)/plylib.o

clean:
	for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} clean ) ; done
	rm -f *.o $(OBJDIR)/*.o *~ core $(INCDIR)/*~ ppr

depend: $(CODES)
	makedepend $(INCLUDEDIRS) $(CODES)

# DO NOT DELETE
