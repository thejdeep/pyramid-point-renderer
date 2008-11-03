
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

SUBDIRS = ellipse_rasterization \
	ewa_surface_splatting \
	jfa_splatting \
	point_ids \
	pyramid_point_renderer \
	pyramid_point_renderer_color \
	pyramid_point_renderer_texture \
	pyramid_templates \
	pyramid_lod \
	pyramid_trees \
	pyramid_triangle_renderer \
	triangle_renderer

OBJS =	application.o \
	camera.o \
	main.o \
	object.o \
	ply.o \
	point_based_renderer.o \
	primitives.o \
	surfels.o \
	timer.o \
	ellipse_rasterization.o \
	ewa_surface_splatting.o \
	jfa_splatting.o \
	point_ids.o \
	pyramid_point_renderer.o \
	pyramid_point_renderer_color.o \
	pyramid_point_renderer_texture.o \
	pyramid_point_renderer_er.o \
	pyramid_point_renderer_lod.o \
	pyramid_point_renderer_trees.o \
	pyramid_triangle_renderer.o \
	triangle_renderer.o 

include Makefile.in

OBJ = $(patsubst %,$(OBJDIR)/%,$(OBJECTS))

OBJ2 = $(patsubst %,$(OBJDIR)/%,$(OBJS))

LIBDIRS = -L$(HOME)/lcgtk/glslKernel/

MATLIB     = -lm
#GLLIBS	  = -lopengl32 -lglu32
GLLIBS     = -lGLU -lGL
GLUTLIB    = -lGLee -lglslKernel -lglut 
XLIBS      = -lXext -lX11 -lXi -lpthread

LIBLIST =  $(GLUTLIB) $(GLLIBS) $(MATLIB)

###################################

$(OBJDIR)/%.o: %.cc
	@echo
	@echo "Compiling C++ code : $@"
	$(CXX) -c -o $@ $< $(INCLUDEDIRS) $(CXXFLAGS)

$(OBJDIR)/%.o: %.c
	@echo
	@echo "Compiling C code : $@"
	$(CC) -c -o $@ $< $(INCLUDEDIRS) $(CCFLAGS)

pyramid-point-renderer: $(OBJ)
	for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} ) ; done
	@echo
	@echo "Linking :  $@"
	$(CXX) $(OBJ2) -o $@ $(CXXFLAGS) $(LIBDIRS) $(LIBLIST)

clean:
	for dir in ${SUBDIRS} ; do ( cd $$dir ; ${MAKE} clean ) ; done
	rm -f *.o $(OBJDIR)/*.o *~ core $(INCDIR)/*~ 

# IDIR =../include
# CC=gcc
# CFLAGS=-I$(IDIR)

# ODIR=obj
# LDIR =../lib

# LIBS=-lm

# _DEPS = hellomake.h
# DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# _OBJ = hellomake.o hellofunc.o 
# OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# $(ODIR)/%.o: %.c $(DEPS)
# 	gcc -c -o $@ $< $(CFLAGS)

# hellomake: $(OBJ)
# 	gcc -o $@ $^ $(CFLAGS) $(LIBS)

# .PHONY: clean

# depend:
# 	$(CC) $(CXXFLAGS) -M *.cc > .depend

# clean:	
# 	rm ./objs/*.o point_render *~

# #all: clean depend point_render

# ifeq (.depend,$(wildcard .depend))
# include .depend
# endif


# g++ -Wl,--no-undefined -o pyramid-point-renderer application.o camera.o ellipse_rasterization.o ewa_surface_splatting.o jfa_splatting.o main.o object.o ply.o point_based_renderer.o primitives.o point_ids.o pyramid_point_renderer.o pyramid_point_renderer_color.o pyramid_point_renderer_texture.o pyramid_point_renderer_er.o pyramid_point_renderer_lod.o pyramid_point_renderer_trees.o pyramid_triangle_renderer.o surfels.o timer.o triangle_renderer.o -L/usr/lib -L/usr/X11R6/lib -L/home/ricardo/lcgtk/glslKernel/ -glslKernel -lglut -lQtOpenGL -lQtGui -lQtCore -lGLU -lGL -lpthread  

# g++ -Wl,--no-undefined -o pyramid-point-renderer application.o camera.o ellipse_rasterization.o ewa_surface_splatting.o jfa_splatting.o main.o object.o ply.o point_based_renderer.o primitives.o point_ids.o pyramid_point_renderer.o pyramid_point_renderer_color.o pyramid_point_renderer_texture.o pyramid_point_renderer_er.o pyramid_point_renderer_lod.o pyramid_point_renderer_trees.o pyramid_triangle_renderer.o surfels.o timer.o triangle_renderer.o -L/usr/lib -L/usr/X11R6/lib -L/home/ricardo/lcgtk/glslKernel/ -lglslKernel -lglut -lQtOpenGL -lQtGui -lQtCore -lGLU -lGL -lpthread

# g++ -Wl,--no-undefined -o pyramid-point-renderer application.o camera.o ellipse_rasterization.o ewa_surface_splatting.o jfa_splatting.o main.o object.o ply.o point_based_renderer.o primitives.o point_ids.o pyramid_point_renderer.o pyramid_point_renderer_color.o pyramid_point_renderer_texture.o pyramid_point_renderer_er.o pyramid_point_renderer_lod.o pyramid_point_renderer_trees.o pyramid_triangle_renderer.o surfels.o timer.o triangle_renderer.o -L/usr/lib -L/usr/X11R6/lib -L/home/ricardo/lcgtk/glslKernel/ -lglslKernel -lglut -lQtOpenGL -lQtGui -lQtCore -lGLU -lGL -lpthread

# g++ -Wl,--no-undefined -o pyramid-point-renderer application.o camera.o ellipse_rasterization.o ewa_surface_splatting.o jfa_splatting.o main.o object.o ply.o point_based_renderer.o primitives.o point_ids.o pyramid_point_renderer.o pyramid_point_renderer_color.o pyramid_point_renderer_texture.o pyramid_point_renderer_er.o pyramid_point_renderer_lod.o pyramid_point_renderer_trees.o pyramid_triangle_renderer.o surfels.o timer.o triangle_renderer.o -L/usr/lib -L/usr/X11R6/lib -L/home/ricardo/lcgtk/glslKernel/ -glslKernel -lglut -lQtOpenGL -lQtGui -lQtCore -lGLU -lGL -lpthread  
