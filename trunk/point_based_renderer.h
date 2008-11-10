/*
** point_based_renderer.h Point Based Render header.
**
**
**   history:	created  02-Jul-07
*/


#ifndef __POINT_BASED_RENDERER_H__
#define __POINT_BASED_RENDERER_H__

#include "glslKernel/glslKernel.h"
#include "surfels.h"
#include "pyramid_types.h"
#include "materials.h"
#include "object.h"

/**
 * Base class for rendering algorithms.
 **/
class PointBasedRenderer
{
 private:

 public:
  /**
   * Default constructor, creates an 1024x1024 screen size.
   **/
  PointBasedRenderer() : window_width(1024), window_height(1024),
    canvas_width(1024), canvas_height(1024),
    material_id(0), depth_test(1), elliptical_weight(1),
    reconstruction_filter_size(1.0), prefilter_size(1.0)
    {}

  /**
   * Constructor for given screen size.
   * @param w Screen width.
   * @param h Screen height.
   **/
  PointBasedRenderer(int w, int h) : window_width(w), window_height(h),
    canvas_width(h), canvas_height(h),
    material_id(0), depth_test(1), elliptical_weight(1),
    reconstruction_filter_size(1.0), prefilter_size(1.0)
    {}

   virtual ~PointBasedRenderer() {};

   /**
    * Render point based model using deferred shading (per pixel shading).
    **/
   virtual void draw( void ) {}

   /**
    * Intepolate samples in screen space using pyramid method.
    **/
   virtual void interpolate( void ) {}

   /**
    * Projects samples to screen space.
    * @param p Point to primitives instance containing samples.
    **/
   virtual void projectSamples(Primitives* p) {}

   /**
    * Clears all buffers, including those of the framebuffer object.
    **/
   virtual void clearBuffers( void ) {}

   /**
    * Sets the size of the prefilter (default = 1.0).
    * This filter works as an increment of the radius size in screen space.
    * @param s Prefilter size.
    **/
   virtual void setPrefilterSize(double s) { prefilter_size = s; }

   /**
    * Sets the size of the reconstruction filter (default = 1.0).
    * This filter works as a multiplier of the radius size in screen space.
    * @param s Reconstruction filter size.
    **/
   virtual void setReconstructionFilterSize(double s) { reconstruction_filter_size = s; }

   /** 
    * Sets the kernel size, for templates rendering only.
    * @param Kernel size.
    **/
   virtual void setGpuMaskSize ( int k ) {}

   /** 
    * Sets eye vector used mainly for backface culling.
    * @param e Given eye vector.
    **/
   void setEye (Point e) {
     eye = e;
   }

   /**
    * Sets light vector for per vertex shading.
    * @param l Given light vector.
    **/
   void setLight (double l[3]) {
     light_dir[0] = l[0];
     light_dir[1] = l[1];
     light_dir[2] = l[2];
   }

   /**
    * Sets the material id number for rendering.
    * @param m Material id.
    **/
   void setMaterial (const int m) {
     if (m < NUM_MATERIALS)
       material_id = m;
   }

   /**
    * Gets the material id number for rendering.
    * @return Current material id.
    **/
   const int getMaterial ( void ) {
     return material_id;
   }

   void upMaterial ( void ) {
     ++material_id;
     if (material_id == NUM_MATERIALS)
       material_id = 0;

   }
   void downMaterial ( void ) {
     --material_id;
     if (material_id < 0)
       material_id = NUM_MATERIALS - 1;
   }

   /**
    * Sets the depth test flag on/off.
    * @param d Given depth test state.
    **/
   void setDepthTest( const bool d ) {
     depth_test = d;
   }

   /**
    * Sets the backface culling flag on/off.
    * @param b Given backface culling state.
    **/
   void setBackFaceCulling( const bool b ) {
     back_face_culling = b;
   }

   void setEllipticalWeight( const bool w ) {
     elliptical_weight = w;
   }

 protected:
   /// Window width.
   GLuint window_width;
   /// Window height.
   GLuint window_height;

   /// Canvas width.
   int canvas_width;
   /// Canvas height.
   int canvas_height;

   /// Light direction vector.
   double light_dir[3];
   /// Eye position.
   Point eye;

   /// Identification of the material from materials.h table.
   int material_id;

   /// Flag to turn on/off depth test
   bool depth_test;

   /// Flag to turn on/off back_face_culling
   bool back_face_culling;

   /// Flag to turn on/off elliptical weight
   bool elliptical_weight;

   /// Size of reconstruction filter.
   double reconstruction_filter_size;
   /// Size of antialising filter.
   double prefilter_size;
};

inline void check_for_ogl_error() {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "%s(%d) glError: %s\n", __FILE__, __LINE__, gluErrorString(err));
  }
}

#endif
