/*
** point_based_render.h Point Based Render header.
**
**
**   history:	created  02-Jul-07
*/


#ifndef __POINT_BASED_RENDER_H__
#define __POINT_BASED_RENDER_H__

#include "glslkernel.h"
#include "surfels.h"
#include "pyramid_types.h"
#include "materials.h"
#include "object.h"

class PointBasedRender
{
 private:

 public:
  PointBasedRender() : window_width(1024), window_height(1024),
    canvas_width(1024), canvas_height(1024),
    zoom_factor(1.0), material_id(0), depth_test(1), elliptical_weight(1),
    reconstruction_filter_size(1.0), prefilter_size(1.0)
    {}

  PointBasedRender(int w, int h) : window_width(w), window_height(h),
    canvas_width(h), canvas_height(h),
    zoom_factor(1.0), material_id(0), depth_test(1), elliptical_weight(1),
    reconstruction_filter_size(1.0), prefilter_size(1.0)
    {}

   virtual ~PointBasedRender() {};
   
   virtual void draw( ) {}
   virtual void draw(int) {}
   virtual void interpolate( ) {}
   virtual void projectSamples(Primitives*) {}
   virtual void clearBuffers( ) {}

   virtual void setVertices( std::vector<Surfel> *surfels ) {}
   virtual void setTriangles( std::vector<Triangle> *triangles ) {}

   virtual void setPrefilterSize(double s) {}
   virtual void setReconstructionFilterSize(double s) {}
   virtual void setZoomFactor (double z) {}
   virtual void setEye (double e[3]) {}
   virtual void setLight (double l[3]) {}

   void setMaterial (const int m) {
     if (m < NUM_MATERIALS)
       material_id = m;
   }

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

   void setDepthTest( const bool d ) {
     depth_test = d;
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
   double eye[3];
   /// Zoom factor.
   double zoom_factor;

   /// Identification of the material from materials.h table.
   int material_id;

   /// Flag to turn on/off depth test
   bool depth_test;

   /// Flag to turn on/off elliptical weight
   bool elliptical_weight;


   /// Size of reconstruction filter.
   double reconstruction_filter_size;
   /// Size of antialising filter.
   double prefilter_size;
};

#endif
