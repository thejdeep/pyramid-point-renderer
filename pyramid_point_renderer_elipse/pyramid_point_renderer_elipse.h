/*
** pyramid_point_renderer.h Pyramid Point Based Rendering header.
**
**
**   history:	created  20-Nov-09
*/


#ifndef __PYRAMID_POINT_RENDERER_ELIPSE_H__
#define __PYRAMID_POINT_RENDERER_ELIPSE_H__

#include <cmath>
#include <cassert>

#include "../pyramid_point_renderer_base/pyramid_point_renderer_base.h"

class PyramidPointRendererElipse : public PyramidPointRendererBase
{
	private:
		void createShaders ( void );

	public:
		PyramidPointRendererElipse(int w, int h);
 
};

#endif
