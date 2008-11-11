/*
** pyramid_point_renderer.h Pyramid Point Based Rendering header.
**
**
**   history:	created  24-Apr-08
*/

#ifndef __PYRAMID_POINT_RENDERER_ER_H__
#define __PYRAMID_POINT_RENDERER_ER_H__

#include <cmath>
#include <cassert>

//#include "point_based_renderer.h"
#include "../pyramid_point_renderer/pyramid_point_renderer.h"

class PyramidPointRendererER : public PyramidPointRenderer
{
 private:

  void createShaders ( void );
  const int synthesisCallbackFunc( void ) const;
  const int analysisCallbackFunc( void ) const;
  const int projectionCallbackFunc( void ) const;
    const int phongShadingCallbackFunc( void ) const;
  void rasterizeSynthesisPyramid();
    void rasterizePhongShading(int bufferIndex);

 public:

  PyramidPointRendererER(int w, int h);

  void setGpuMaskSize ( int s ) { gpu_mask_size = s; }

 private:

  /// Gpu mask size (sub mask for each cpu mask pixel)
  int gpu_mask_size;
};

#endif
