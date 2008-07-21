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
    mMinorAxis = std::make_pair(0.0,Vector3());
    mMajorAxis = std::make_pair(0.0,Vector3());
		 
  }
	 
  Surfel (const Surfel<Real>& pSurfel)
  {
    mCenter             = pSurfel.Center();
    mNormal             = pSurfel.Normal();
    mMinorAxis          = pSurfel.MinorAxis();
    mMajorAxis          = pSurfel.MajorAxis();
    mColor              = pSurfel.color(); 
    mSplatRadius        = pSurfel.Radius();
    mPerpendicularError = pSurfel.perpendicularError();
    mID                 = pSurfel.ID();
  }
	 
  Surfel (const Point3& 	center, 
	  const Vector3& normal,
	  const std::pair<Real,Vector3>& pMinorAxis,
	  const std::pair<Real,Vector3>& pMajorAxis,
	  unsigned int 	id ) : mCenter(center),
			       mNormal(normal),
			       mMinorAxis(pMinorAxis),
			       mMajorAxis(pMajorAxis),
			       mID(id)
  {
    mColor = Color(1.0,0.0,0.0); 

  };	
	 
  Surfel (const Point3& 	position, 
	  const Vector3& normal, 
	  const Real& 	radius, 
	  unsigned int 	id, 
	  const Real& 	pError ) : mCenter(position),
				   mNormal(normal),
				   mSplatRadius(radius),
				   mPerpendicularError(pError),
				   mID(id)
  {
    mColor = Color(0.0,0.0,0.0); 
    mNormal.normalize();
    Vector3 lV = Perpendicular(mNormal);
    Vector3 lU = mNormal ^ lV;
    lU.normalize();
    mMinorAxis = std::make_pair(mSplatRadius,lV);
    mMajorAxis = std::make_pair(mSplatRadius,lU);
  };	
	
	
  Surfel (const Point3& position, 
	  const Vector3& normal,
	  const Color& color,
	  Real radius,
	  unsigned int 	id ) : 	mCenter(position),
				mNormal(normal),
				mColor(color),
				mSplatRadius(radius),
				mPerpendicularError(0),
				mID(id)
  {
    mNormal.normalize();
    Vector3 lV = Perpendicular(mNormal);
    Vector3 lU = mNormal ^ lV;
    lU.normalize();
    mMinorAxis = std::make_pair(mSplatRadius,lV);
    mMajorAxis = std::make_pair(mSplatRadius,lU);
  };
	  
  Surfel (const Point3& position, 
	  const Vector3& normal, 
	  const Real& 	radius, 
	  unsigned int 	id ) : 	mCenter(position),
				mNormal(normal),
				mSplatRadius(radius),
				mPerpendicularError(0),
				mID(id)
  {
    mColor = Color(0.0,0.0,0.0);
    mNormal.normalize();
    Vector3 lV = Perpendicular(mNormal);
    Vector3 lU = mNormal ^ lV;
    lU.normalize();
    mMinorAxis = std::make_pair(mSplatRadius,lV);
    mMajorAxis = std::make_pair(mSplatRadius,lU);
  };
		  
  inline const Surfel<Real>& operator= ( const Surfel<Real>& pSurfel)
  {
    this->mCenter    = pSurfel.Center();
    this->mNormal    = pSurfel.Normal();
    this->mMinorAxis = pSurfel.MinorAxis();
    this->mMajorAxis = pSurfel.MajorAxis();
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

  Real perpendicularError () const 
  { 
    return ( this->mPerpendicularError ); 
  };

  void SetMinorAxis( const std::pair<Real,Vector3>& pMinorAxis)
  {
    this->mMinorAxis = pMinorAxis; 
  }

  void SetMajorAxis( const std::pair<Real,Vector3>& pMajorAxis)
  {
    this->mMajorAxis = pMajorAxis; 
  }
	 
  std::pair<Real,Vector3> MinorAxis() const
  {
    return (this->mMinorAxis); 
  }

  std::pair<Real,Vector3> MajorAxis() const 
  {
    return (this->mMajorAxis); 
  }
	 
  /// I/O operator - output
  inline friend std::ostream& operator << (std::ostream& out, const Surfel &s) 
  {
    out << s.perpendicularError() << " " << s.Center.x() << " " 
	<< s.Center.y() 	<< " " << s.Center.z() << " " 
	<< s.radius() 		<< " " << s.Normal.x() << " " 
	<< s.Normal.y() 	<< " " << s.Normal.z();
	    
    return out;
  };
	 
  Real Area() const
  {
    return (  (M_PI * mMinorAxis.first) * (M_PI * mMajorAxis.first) );
  }
	 
	 
	 
  Vector3 Perpendicular( const Vector3& pVector)
  {
    //select the shortest of projections of axes on v
    //(the closest to perpendicular to v),
    //and project it to the plane defined by v
    if ( fabs( pVector.x()) < fabs( pVector.y()) ) // x < y 
      {

	if ( fabs( pVector.x()) < fabs( pVector.z()) )
	  {  // x < y && x < z
	    Vector3 lPerpendicularX (1.0 - (pVector.x() * pVector.x()),
				     -pVector.x() * pVector.y(),
				     -pVector.x() * pVector.z() );
				 
	    return lPerpendicularX.norm();
	  }
      }  
    else
      { //y <= x

	if (fabs(pVector.y()) < fabs(pVector.z()) )
	  {  // y <= x && y < z
	    Vector3 lPerpendicularY( -pVector.y() * pVector.x(), 
				     1.0 - (pVector.y() * pVector.y()), 
				     -pVector.y() * pVector.z() );
				 
	    return lPerpendicularY.norm();

	  }
      }
    // z <= x && z <= y
    Vector3 lPerpendicularZ(-pVector.z() * pVector.x(), 
			    -pVector.z() * pVector.y(), 
			    1.0 - (pVector.z() * pVector.z()));
		 
    return lPerpendicularZ.norm();

  }	   
	   	 
  std::list<Point3* > BoundariesSamples(unsigned int pSteps) const
  { 

    if (pSteps == 0)
      {
	pSteps = 4;
      }
		 	 
    std::list<Point3* > lPoints;

    Real lAlpha 			= 0.0;     
    Real lSinAlpha 		= 0.0;
    Real lCosAlpha 		= 0.0;

    Real lX 				= 0.0;
    Real lY 				= 0.0;
    Real lFactor 			= 0.0;

    Real lCos				= 0.0;
    Real lSin				= 0.0;
    Vector3 lDirection     = Vector3();
		 
    Real i = 0;

    while (i < 360) 
      {

	lAlpha = ( i / 180 ) * M_PI;
	lSinAlpha =  sin( lAlpha );
	lCosAlpha =  cos( lAlpha );


	lX = mMinorAxis.first * lCosAlpha;

	lY = mMajorAxis.first * lSinAlpha;

	lFactor = sqrt( lX*lX + lY*lY );

	lCos = lX / lFactor;
	lSin = lY / lFactor;

	lDirection = (mMinorAxis.second * lCos) + (mMajorAxis.second * lSin);
		 
	lPoints.push_back( new Point3( (mCenter + (lDirection * lFactor)) ) );

	i = i + (360.0 / pSteps);
			 
      } 

    return lPoints;

  }
	 
  void draw()
  {
		 
    ListPtrPoint3 lBoundaries = this->BoundariesSamples(50);
 		 	
    for(ListPtrPoint3Iterator it = lBoundaries.begin();it != lBoundaries.end();++it)
      {		
	Point3 point = (*(*it));
	glPointSize(1.5);
	glColor3f(0.0,1.0,0.25);
				
	glVertex3f(point[0],point[1],point[2]);
      }
			

  }

private:
	 
  /// Point coordinates
  Point3 mCenter;

  /// Estimated surface normal at point sample
  Vector3 mNormal;
	  
  Color mColor;
	  
  /// Splat radius
  Real mSplatRadius;

  /// Minor Axis
  std::pair<Real,Vector3> mMinorAxis;
	  
  /// Major Axis
  std::pair<Real,Vector3> mMajorAxis;
	  
  /// Perpendicular error
  Real mPerpendicularError;
	  
  /// An identification number for the surfel
  unsigned int mID;
	   

};

#endif
