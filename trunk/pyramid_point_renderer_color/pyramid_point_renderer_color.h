/*
** pyramid_point_renderer.h Pyramid Point Based Rendering header.
**
**
**   history:	created  02-Jul-07
*/


#ifndef __PYRAMID_POINT_RENDERER_COLOR_H__
#define __PYRAMID_POINT_RENDERER_COLOR_H__

#include <cmath>
#include <cassert>

#include "../pyramid_point_renderer/pyramid_point_renderer.h"

class PyramidPointRendererColor : public PyramidPointRenderer
{
 private:
  
  void createShaders ( void );

 public:

  PyramidPointRendererColor(int w, int h);
 
};

#endif
