#ifndef SURFEL_HPP_
#define SURFEL_HPP_

/**
 * 
 * Author: Ricardo Marroquim, Felipe Moura
 *
 * Data created: 08-01-2008
 *
 **/

#include <vector>
#include <cmath>

#include <iostream>
#include <wrap/gl/shaders.h>


// #include <vcg/simplex/vertex/base.h>
// #include <vcg/simplex/face/base.h>
// #include <vcg/simplex/edge/base.h>
// #include <vcg/complex/trimesh/base.h>

// #include <vcg/math/quadric.h>
// #include <vcg/complex/trimesh/clean.h>

#include <wrap/io_trimesh/io_ply.h>
#include <wrap/ply/plylib.h>

#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export_ply.h>


#include "GL/gl.h"
#include "GL/glu.h"

#include "vcg/space/point3.h"
#include "vcg/space/color4.h"

extern "C" 
{
#include <stdio.h>
#include <strings.h>
}

#include <list>

//#define PI 3.1415926535897932384626433832795

/**
 * Surfel class.
 * A surface element (surfel) is a sample point of the surface.
 * Also known as splats in some implementations.
 * The splat has an elliptical or circular form, and an estimated
 * surface normal at it's center.
 **/

using namespace vcg;

template <class Real> class Surfel
{
public:
		
  typedef std::list<Point3f* >       			ListPtrPoint3;
  typedef typename ListPtrPoint3::iterator  	ListPtrPoint3Iterator;

  typedef ::vcg::ply::PropDescriptor PropDescriptor ;

/* IO PLY */

	enum IO_SURFEL
	{
		IOM_SURFEL = 0x10002
	};

	struct LoadPlySurfel
	{
		float cx;
		float cy;
		float cz;
		float nx;
		float ny;
		float nz;
		float major_axisx;
		float major_axisy;
		float major_axisz;
		float major_axis_size;
		float minor_axisx;
		float minor_axisy;
		float minor_axisz;
		float minor_axis_size;
		float r;
		float g;
		float b;
		float max_error;
		float min_error;
	};


	static const  PropDescriptor &SurfelDesc(int i)
	{
		static const PropDescriptor surfel[19] =
		{
				{"surfel","cx"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,cx)				,0,0,0,0,0  ,0},
				{"surfel","cy"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,cy)				,0,0,0,0,0  ,0},
				{"surfel","cz"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,cz)				,0,0,0,0,0  ,0},
				{"surfel","nx"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,nx)				,0,0,0,0,0  ,0},
				{"surfel","ny"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,ny)				,0,0,0,0,0  ,0},
				{"surfel","nz"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,nz)				,0,0,0,0,0  ,0},
				{"surfel","major_axisx"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,major_axisx)		,0,0,0,0,0  ,0},
				{"surfel","major_axisy"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,major_axisy)		,0,0,0,0,0  ,0},
				{"surfel","major_axisz"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,major_axisz)		,0,0,0,0,0  ,0},
				{"surfel","major_axis_size"	,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,major_axis_size)	,0,0,0,0,0  ,0},
				{"surfel","minor_axisx"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,minor_axisx)		,0,0,0,0,0  ,0},
				{"surfel","minor_axisy"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,minor_axisy)		,0,0,0,0,0  ,0},
				{"surfel","minor_axisz"  	,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,minor_axisz)		,0,0,0,0,0  ,0},
				{"surfel","minor_axis_size" ,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,minor_axis_size)	,0,0,0,0,0  ,0},
				{"surfel","r"	 			,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,r)					,0,0,0,0,0  ,0},
				{"surfel","g"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,g)					,0,0,0,0,0  ,0},
				{"surfel","b"				,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,b)					,0,0,0,0,0  ,0},
				{"surfel","max_error"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,max_error)			,0,0,0,0,0  ,0},
				{"surfel","min_error"		,vcg::ply::T_FLOAT,vcg::ply::T_FLOAT,offsetof(LoadPlySurfel,min_error)			,0,0,0,0,0  ,0}
		};
		return surfel[i];
	}
/* END */

  Surfel ()
  {
    mCenter = Point3f();
    mNormal = Point3f();
    mQuality = 0.0;
    mSplatRadius = 0.0;

  }
  Surfel (const Point3f& position, 
	  const Point3f& normal,
	  const Color4b& color,
	  Real quality,
	  Real radius,
	  unsigned int 	id ) : 	mCenter(position),
				mNormal(normal),
				mColor(color),
				mQuality(quality),
				mSplatRadius(radius),
				mID(id)
  {

  };


  Surfel (const LoadPlySurfel& pSurfel) :
											 mCenter(pSurfel.cx,pSurfel.cy,pSurfel.cz),
    										 mNormal(pSurfel.nx,pSurfel.ny,pSurfel.nz),
    									 	 mColor(pSurfel.r,pSurfel.g,pSurfel.b,1.0),
    									 	 mSplatRadius(pSurfel.major_axis_size),
    		  								 mMinorAxis(std::make_pair<Real,Point3f>(pSurfel.minor_axis_size,
																 Point3f(pSurfel.minor_axisx,
    		  										               		  pSurfel.minor_axisy,
    		  										               		  pSurfel.minor_axisz))),
    										 mMajorAxis(std::make_pair<Real,Point3f>(pSurfel.major_axis_size,
																  Point3f(pSurfel.major_axisx,
    												            		  pSurfel.major_axisy,
    												            		  pSurfel.major_axisz))),
    										 mMaxError(pSurfel.max_error),
    										 mMinError(pSurfel.min_error),
    										 mID(0)

  {}

  inline const Surfel<Real>& operator= ( const Surfel<Real>& pSurfel)
  {
	  this->mCenter    			= pSurfel.Center();
	  this->mNormal    			= pSurfel.Normal();
	  this->mMinorAxis 			= pSurfel.MinorAxis();
	  this->mMajorAxis 			= pSurfel.MajorAxis();
	  this->mMaxError 			= pSurfel.MaxError();
	  this->mMinError 			= pSurfel.MinError();
	  this->mColor     			= pSurfel.Color();
	  this->mQuality   			= pSurfel.Quality();
	  this->mSplatRadius  		= pSurfel.Radius();
	  this->mID					= pSurfel.ID();

	  return ( *this );
  }
	 
  Surfel (const Point3f& position)
  {
    this->mCenter 		= position;
	         
  };	    
	   
  ~Surfel() {};
	
  const Point3f Center () const 
  { 
    return  ( this->mCenter ) ; 
  };
	 
  Real Center(const int axis)  const
  { 
    return ( this->mCenter[axis] ); 
  };
	 
  void SetCenter(const Point3f& pCenter) 
  { 
    this->mCenter = Point3f(pCenter); 
  };
	
  const Point3f Normal(void) const 
  { 
    return (this->mNormal); 
  };
	 
  Real Normal(int axis) const 
  { 
    return ( this->mNormal[axis] ); 
  };
	 
  void SetNormal (const Point3f& normal )
  { 
    this->mNormal = Point3f(normal); 
  };

  unsigned int ID () const 
  { 
    return ( this->mID ); 
  };

  void SetID (unsigned int id) 
  {	
    this->mID = id; 
  };

  const Real Radius (void) const
  { 
    return this->mSplatRadius;
  }

  void SetRadius ( const Real& pRadius ) 
  { 
    this->mSplatRadius = pRadius; 
  };

  const Real Quality (void) const
  { 
    return this->mQuality;
  }

  void SetQuality ( const Real& pQuality )
  { 
    this->mQuality = pQuality;
  };

  Color4b Color (void) const
  { 
    return this->mColor; 
  };

  void SetColor ( const Color4b& pColor ) 
  { 
    this->mColor = pColor; 
  };

  void SetMinorAxis( const std::pair<Real,Point3f>& pMinorAxis)
  {
  	 this->mMinorAxis = pMinorAxis;
  }


  void SetMajorAxis( const std::pair<Real,Point3f>& pMajorAxis)
  {
  	 this->mMajorAxis = pMajorAxis;
  }

  std::pair<Real,Point3f > MinorAxis() const
  {
  	 return (this->mMinorAxis);
  }


  std::pair<Real,Point3f > MajorAxis() const
  {
  	 return (this->mMajorAxis);
  }

  Real MaxError () const
  {
  	 return ( this->mMaxError );
  };

  void SetMaxError (const Real& pMaxError)
  {
  	 this->mMaxError = pMaxError;
  };


  Real MinError () const
  {
  	 return ( this->mMinError );
  };


  void SetMinError (const Real& pMinError)
  {
  	 this->mMinError = pMinError;
  };

	 
  /// I/O operator - output
  inline friend std::ostream& operator << (std::ostream& out, const Surfel &s) 
  {
    out <<  "Position "   << s.mCenter[0]  << " "
			      << s.mCenter[1]  << " "
			      << s.mCenter[2]  << " Radius "
				  << s.Radius()   << " Normal "
				  << s.mNormal[0]  << " "
				  << s.mNormal[1]  << " "
				  << s.mNormal[2];
	    
    return out;
  };
	 

private:
	 
  /// Point coordinates
  Point3f mCenter;

  /// Estimated surface normal at point sample
  Point3f mNormal;

  /// RGB color
  Color4b mColor;

  /// Quality
  Real mQuality;
	  
  /// Splat radius
  Real mSplatRadius;

  /// Surfel ID
  int mID;

  /// Minor Axis
  std::pair<Real, Point3f> mMinorAxis;

  /// Major Axis
  std::pair<Real, Point3f> mMajorAxis;

  Real mMaxError;
  Real mMinError;

};


#endif
