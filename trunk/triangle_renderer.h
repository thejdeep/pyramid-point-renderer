/*
** triangle_renderer.h Triangle renderer header.
**
**
**   history:	created  16-Oct-07
*/


#ifndef __TRIANGLE_RENDERER_H__
#define __TRIANGLE_RENDERER_H__

#include "point_based_render.h"

class TriangleRenderer : public PointBasedRender
{
 private:

 public:
  TriangleRenderer();
  ~TriangleRenderer();

  void draw();
  void draw( double[3] ); 

  void setTriangles( vector<Triangle> *t );
  void setVertices( vector<Surfel> *s );

  void setReconstructionFilterSize( void ) {}
  void setPreilterSize( void ) {}

 private:

   /// Triangle Display List
   GLuint triangleDisplayList;
   
   /// Pointer to list of surfels (vertices).
   vector<Surfel> *surfels;
   
   /// Pointer to list of triangles.
   vector<Triangle> *triangles;

};

#define CHECK_FOR_OGL_ERROR()	  									 \
	do {															 \
		GLenum err;													 \
		err = glGetError();											 \
		if (err != GL_NO_ERROR)										 \
		{															 \
			fprintf(stderr, "%s(%d) glError: %s\n",					 \
					__FILE__, __LINE__, gluErrorString(err));		 \
		}															 \
	} while(0)

#if !defined NULL
#	define NULL 0
#endif

#if !defined TRUE
#	define TRUE 1
#endif

#if !defined FALSE
#	define FALSE 0
#endif


 /* macros */

#if !defined MAX
#	define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif
