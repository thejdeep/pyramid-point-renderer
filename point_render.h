/**
 * Header file for point_render.cc
 *
 * Point-based model rendering algorithm.
 * 
 * Author: Ricardo Marroquim
 *
 * Data created: 05-01-07
 *
 **/

#ifndef __POINT_RENDER__
#define __POINT_RENDER__

//OpenGL and GLUT includes
#include <assert.h>

//IO includes
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <math.h>

#include <list>
#include <vector>

//#include "ewa_surface_splatting.h"
//#include "pyramid_triangle_renderer.h"
//#include "triangle_renderer.h"

#include "pyramid_point_render.h"
#include "pyramid_point_render_color.h"

#include "surfels.h"
#include "quat.h"

#include "object.h"

#define PI 3.14159265
#define HALF_PI 1.570796325
#define QUARTER_PI 0.785398163
#define E 2.71828183

unsigned int number_surfels;

PointBasedRender *point_based_render;

#endif
