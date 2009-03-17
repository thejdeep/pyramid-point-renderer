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

#include "../pyramid_point_renderer_base/pyramid_point_renderer_base.h"

class PyramidPointRendererER : public PyramidPointRendererBase
{
 private:

  void createShaders ( void );

  const int synthesisCallbackFunc( void ) ;
  const int analysisCallbackFunc( void ) ;
  const int projectionCallbackFunc( void ) ;
  const int phongShadingCallbackFunc( void ) ;
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
