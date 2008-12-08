
OS = linux

OBJECTS = application.o \
	camera.o \
	main.o \
	matrix.o \
	object.o \
	ply.o \
	point_based_renderer.o \
	primitives.o \
	surfels.o \
	timer.o

SUBDIRS = pyramid_point_renderer_base \
	pyramid_point_renderer \
	pyramid_point_renderer_color \
	pyramid_templates

OBJS =	application.o \
	camera.o \
	main.o \
	object.o \
	ply.o \
	point_based_renderer.o \
	primitives.o \
	surfels.o \
	timer.o \
	pyramid_point_renderer_base.o \
	pyramid_point_renderer.o \
	pyramid_point_renderer_color.o \
	pyramid_point_renderer_er.o

CODES =	application.cc \
	camera.cc \
	main.cc \
	object.cc \
	ply.c \
	point_based_renderer.cc \
	primitives.cc \
	surfels.cc \
	timer.c \
	pyramid_point_renderer/pyramid_point_renderer_base.cc \
	pyramid_point_renderer/pyramid_point_renderer.cc \
	pyramid_point_renderer_color/pyramid_point_renderer_color.o \
	pyramid_templates/pyramid_point_renderer_er.o

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

###################################

$(OBJDIR)/%.o: %.cc
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

clean:
	for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} clean ) ; done
	rm -f *.o $(OBJDIR)/*.o *~ core $(INCDIR)/*~ ppr

depend: $(CODES)
	makedepend $(INCLUDEDIRS) $(CODES)

# DO NOT DELETE
