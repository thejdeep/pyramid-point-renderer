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

#include "GL/gl.h"
#include "GL/glu.h"

#include "lal/Point3.hpp"
#include "lal/Vector3.hpp"
#include "lal/Color.hpp"

extern "C" 
{
#include <stdio.h>

#include <strings.h>
}

//#define PI 3.1415926535897932384626433832795

/**
 * Surfel class.
 * A surface element (surfel) is a sample point of the surface.
 * Also known as splats in some implementations.
 * The splat has an elliptical or circular form, and an estimated
 * surface normal at it's center.
 **/


template <class Real > class Surfel
{
public:
		
  typedef LAL::Point3<Real>  Point3; 
  typedef LAL::Vector3<Real> Vector3;
  typedef LAL::Color         Color;


  typedef std::list<Point3* >       			ListPtrPoint3;
  typedef typename ListPtrPoint3::iterator  	ListPtrPoint3Iterator;

  Surfel ()
  {
    mCenter = Point3();
    mNormal = Vector3();
    mSplatRadius = 0.0;

		 
  }


	
  Surfel (const Point3& position, 
	  const Vector3& normal,
	  const Color& color,
	  Real radius,
	  unsigned int 	id ) : 	mCenter(position),
				mNormal(normal),
				mColor(color),
				mSplatRadius(radius),
				mID(id)
  {

  };
	  
	  
  inline const Surfel<Real>& operator= ( const Surfel<Real>& pSurfel)
  {
    this->mCenter    = pSurfel.Center();
    this->mNormal    = pSurfel.Normal();
 
    this->mColor     = pSurfel.color();
		 
    return ( *this );
  }
	 
  Surfel (const Point3& 	position)
  {
    this->mCenter 		= position;
	         
  };
	
    
	   
  ~Surfel() {};
	
  const Point3 Center () const 
  { 
    return  ( this->mCenter ) ; 
  };
	 
  Real Center(const int axis)  const
  { 
    return ( this->mCenter[axis] ); 
  };
	 
  void SetCenter(const Point3& pCenter) 
  { 
    this->mCenter = Point3(pCenter); 
  };
	
  const Vector3 Normal(void) const 
  { 
    return (this->mNormal); 
  };
	 
  Real Normal(int axis) const 
  { 
    return ( this->mNormal[axis] ); 
  };
	 
  void SetNormal (const Vector3& normal )
  { 
    this->mNormal = Vector3(normal); 
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

  Color color (void) const 
  { 
    return this->mColor; 
  };

  void SetColor ( const Color& pColor ) 
  { 
    this->mColor = pColor; 
  };

	 
  /// I/O operator - output
  inline friend std::ostream& operator << (std::ostream& out, const Surfel &s) 
  {
    out << s.perpendicularError() << " " << s.Center[0] << " " 
	<< s.Center[1] 	<< " " << s.Center[2] << " " 
	<< s.radius() 		<< " " << s.Normal[0] << " " 
	<< s.Normal[1] 	<< " " << s.Normal[2];
	    
    return out;
  };
	 

private:
	 
  /// Point coordinates
  Point3 mCenter;

  /// Estimated surface normal at point sample
  Vector3 mNormal;
	  
  Color mColor;
	  
  /// Splat radius
  Real mSplatRadius;
   
  int mID;

};

#endif
