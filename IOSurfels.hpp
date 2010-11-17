/*
 * ImporterPLY.hpp
 *
 *  Created on: Nov 2, 2009
 *      Author: fmc
 */

#ifndef IMPORTERPLY_HPP_
#define IMPORTERPLY_HPP_

//  System

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
//  VCGLib

#include <vcg/simplex/vertex/base.h>
#include <vcg/simplex/vertex/component_ocf.h>
#include <vcg/simplex/edge/base.h>
#include <vcg/simplex/face/base.h>
#include <vcg/simplex/face/component_ocf.h>
#include <vcg/simplex/face/topology.h>

#include <vcg/complex/trimesh/base.h>
#include <vcg/complex/trimesh/allocate.h>
#include <vcg/complex/trimesh/update/topology.h>
#include <vcg/complex/trimesh/update/normal.h>
#include <vcg/complex/trimesh/update/flag.h>

#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/export_ply.h>

// Local
#include "surfel.hpp"

class MyVertex;

//Vert Mem Occupancy  --- 40b ---

class MyVertex;
class MyFace;

class MyUsedTypesIO: public vcg::UsedTypes< vcg::Use<MyVertex>::AsVertexType,
					    vcg::Use<MyFace>::AsFaceType>{};


class MyVertex  : public vcg::Vertex< MyUsedTypesIO,
				      vcg::vertex::InfoOcf,           /*  4b */
				      vcg::vertex::Coord3f,           /* 12b */
				      vcg::vertex::BitFlags,          /*  4b */
				      vcg::vertex::Normal3f,          /* 12b */
				      vcg::vertex::Qualityf,          /*  4b */
				      vcg::vertex::Color4b,           /*  4b */
				      vcg::vertex::VFAdjOcf,          /*  0b */
				      vcg::vertex::MarkOcf,           /*  0b */
				      vcg::vertex::TexCoordfOcf,      /*  0b */
				      vcg::vertex::CurvaturefOcf,     /*  0b */
				      vcg::vertex::CurvatureDirfOcf,  /*  0b */
				      vcg::vertex::Radiusf         /*  0b */
				      >{};

class MyFace    : public vcg::Face< MyUsedTypesIO,
				    vcg::face::InfoOcf,              /* 4b */
				    vcg::face::VertexRef,            /*12b */
				    vcg::face::BitFlags,             /* 4b */
				    vcg::face::Normal3f,             /*12b */
				    vcg::face::QualityfOcf,          /* 0b */
				    vcg::face::MarkOcf,              /* 0b */
				    vcg::face::Color4bOcf,           /* 0b */
				    vcg::face::FFAdjOcf,             /* 0b */
				    vcg::face::VFAdjOcf,             /* 0b */
				    vcg::face::WedgeTexCoordfOcf     /* 0b */
				    > {};

class MyMesh   : public vcg::tri::TriMesh< std::vector<MyVertex> , std::vector<MyFace> >{};


template<class Real = float> class IOSurfels{

public:

  typedef ::vcg::ply::PropDescriptor PropDescriptor ;

  typedef typename MyMesh::VertexIterator VertexIterator;

  IOSurfels()
  {

  }


  static int SaveMesh (const char * filename,
		       std::vector<Surfel<Real> >& pSurfel,vcg::CallBackPos *cb = 0)
  {
    MyMesh mesh;
    VertexIterator vi = vcg::tri::Allocator<MyMesh>::AddVertices(mesh,pSurfel.size());
    int mask = 0;
    vcg::tri::io::PlyInfo info;
    bool binary = 0;
    for (size_t i = 0; i < pSurfel.size();++i)
      {

	(*vi).P()[0] = pSurfel[i].Center().x;
	(*vi).P()[1] = pSurfel[i].Center().y;
	(*vi).P()[2] = pSurfel[i].Center().z;

	(*vi).N()[0] = pSurfel[i].Normal().x;
	(*vi).N()[1] = pSurfel[i].Normal().y;
	(*vi).N()[2] = pSurfel[i].Normal().z;

	(*vi).R()	 = pSurfel[i].MajorAxis().first;


	++vi;
      }

    mask |= vcg::tri::io::Mask::IOM_VERTNORMAL;
    mask |= vcg::tri::io::Mask::IOM_VERTRADIUS;

    info.mask = mask;

    int result = vcg::tri::io::ExporterPLY<MyMesh>::Save(mesh,filename,binary,info,cb);

    return result;

  }
  static int LoadMesh (
		       const char * filename,
		       std::vector<Surfel<Real> >& pSurfel,vcg::CallBackPos *cb = 0)
  {
    MyMesh mesh;

    int mask = 0;

    mesh.Clear();

    if (cb != NULL) 	(*cb)(0, "Loading Model...");


    vcg::tri::io::PlyInfo info;
    info.cb = cb;

    vcg::tri::io::ImporterPLY<MyMesh>::LoadMask(filename, mask,info);
    vcg::tri::io::ImporterPLY<MyMesh>::Open(mesh,filename,info);


    bool normal_per_vertex = false;
    if (mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
      normal_per_vertex = true;

    bool color_per_vertex = false;
    if (mask & vcg::tri::io::Mask::IOM_VERTCOLOR)
      color_per_vertex = true;

    bool quality_per_vertex = false;
    if (mask & vcg::tri::io::Mask::IOM_VERTQUALITY)
      quality_per_vertex = true;

    bool radius_per_vertex = false;
    if (mask & vcg::tri::io::Mask::IOM_VERTRADIUS)
      radius_per_vertex = true;


    unsigned int pos = 0;
    Surfel<Real> s;

    for (MyMesh::VertexIterator vit = mesh.vert.begin(); vit != mesh.vert.end(); ++vit)
      {
	Point3f v = (*vit).P();
	Point3f n = (*vit).N();

	Color4b c (0.2, 0.2, 0.2, 1.0);
	if (color_per_vertex) {
	  c = Color4b ((GLubyte)(*vit).C()[0], (GLubyte)(*vit).C()[1], (GLubyte)(*vit).C()[2], 1.0);
	}

	Real radius = 0.25;
	if (radius_per_vertex)
	  {
	    radius = static_cast<Real> (((*vit).R()));
	  }

	Real quality = 1.0;
	if (quality_per_vertex)
	  quality = static_cast<Real> ((*vit).Q());


	s = Surfel<Real> (v, n, c, radius, pos);

	s.SetRadius(radius);

	pSurfel.push_back (s);

	++pos;

      }
    std::cout << "Surfel " << pSurfel.size() << std::endl;
    if (cb != NULL)	(*cb)(99, "Done");
    mesh.Clear();
    return 0;
  }

  static int LoadSurfels (const char * filename,std::vector<Surfel<Real> >& pSurfel,vcg::CallBackPos *cb = 0)
  {
    vcg::ply::PlyFile pf;
    vcg::tri::io::PlyInfo pi;

    pSurfel.clear();


    if( pf.Open(filename,vcg::ply::PlyFile::MODE_READ)==-1 )
      {
	pi.status = pf.GetError();
	return pi.status;

      }

    bool found = true;
    int i;
    for(i=0;i<19;++i)
      {
	if( pf.AddToRead(Surfel<Real>::SurfelDesc(i))==-1 )
	  {
	    found = false;
	    break;
	  }
      }

    if(!found)
      return pi.status;


    std::cout << "pf.elements.size() " << int(pf.elements.size()) << std::endl;

    for(i=0;i<int(pf.elements.size());i++)
      {
	int n = pf.ElemNumber(i);

	if( !strcmp( pf.ElemName(i),"surfel" ) )
	  {
	    pf.SetCurElement(i);

	    typename Surfel<Real>::LoadPlySurfel vs;
	    std::cout <<  "n  " << n << std::endl;

	    for(int j=0;j<n;++j)
	      {
		if( pf.Read( (void *)&(vs) )==-1 )
		  {
		    pi.status = vcg::tri::io::PlyInfo::E_SHORTFILE;
		    return pi.status;

		  }

		//					std::cout << "v.cx " << vs.cx << std::endl;
		//					std::cout << "v.cy " << vs.cy << std::endl;
		//					std::cout << "v.cz " << vs.cz << std::endl;
		//					std::cout << "v.nx " << vs.nx << std::endl;
		//					std::cout << "v.ny " << vs.ny << std::endl;
		//					std::cout << "v.nz " << vs.nz << std::endl;
		//
		//					std::cout << "v.major_axisx " << vs.major_axisx << std::endl;
		//					std::cout << "v.major_axisy " << vs.major_axisy << std::endl;
		//					std::cout << "v.major_axisz " << vs.major_axisz << std::endl;
		//					std::cout << "v.major_axissize " << vs.major_axis_size << std::endl;
		//
		//					std::cout << "v.minor_axisx " << vs.minor_axisx << std::endl;
		//					std::cout << "v.minor_axisy " << vs.minor_axisy << std::endl;
		//					std::cout << "v.minor_axisz " << vs.minor_axisz << std::endl;
		//					std::cout << "v.minor_axissize " << vs.minor_axis_size << std::endl;
		//
		//					std::cout << "v.r " << vs.r << std::endl;
		//					std::cout << "v.g " << vs.g << std::endl;
		//					std::cout << "v.b " << vs.b << std::endl;
		//
		//					std::cout << "v.max_error " << vs.max_error << std::endl;
		//					std::cout << "v.min_error " << vs.min_error << std::endl;


		pSurfel.push_back(Surfel<Real>(vs));

		//std::cout <<  pSurfel.back() << std::endl;
	      }

	    break;
	  }
      }

    return 0;
  }


  static int SaveSurfels(std::vector<Surfel<Real> >& pSurfel,  const char * filename,vcg::CallBackPos *cb = 0, bool binary=0)	// V1.0
  {
    FILE * fpout;

    const char * hbin = "binary_little_endian";
    const char * hasc = "ascii";
    const char * h;


    if(binary) h=hbin;
    else       h=hasc;

    fpout = fopen(filename,"wb");
    if(fpout==NULL)	{
      return ::vcg::ply::E_CANTOPEN;
    }
    fprintf(fpout,
	    "ply\n"
	    "format %s 1.0\n"
	    "comment LSplat - VCGLIB generated\n"
	    ,h
	    );

    fprintf(fpout,
	    "element surfel %d\n"
	    "property float cx\n"
	    "property float cy\n"
	    "property float cz\n"
	    "property float nx\n"
	    "property float ny\n"
	    "property float nz\n"
	    "property float major_axisx\n"
	    "property float major_axisy\n"
	    "property float major_axisz\n"
	    "property float major_axis_size\n"
	    "property float minor_axisx\n"
	    "property float minor_axisy\n"
	    "property float minor_axisz\n"
	    "property float minor_axis_size\n"
	    "property float r\n"
	    "property float g\n"
	    "property float b\n"
	    "property float max_error\n"
	    "property float min_error\n"
	    ,pSurfel.size()
	    );


    fprintf(fpout, "end_header\n"	);

    typename Surfel<Real>::LoadPlySurfel v;

    if(binary)
      {

	float t[19];

	for(std::size_t i = 0; i < pSurfel.size(); ++i)
	  {

	    t[ 0] = pSurfel[i].Center().x;
	    t[ 1] = pSurfel[i].Center().y;
	    t[ 2] = pSurfel[i].Center().z;
	    t[ 3] = pSurfel[i].Normal().x;
	    t[ 4] = pSurfel[i].Normal().y;
	    t[ 5] = pSurfel[i].Normal().z;
	    t[ 6] = pSurfel[i].MajorAxis().second.x;
	    t[ 7] = pSurfel[i].MajorAxis().second.y;
	    t[ 8] = pSurfel[i].MajorAxis().second.z;
	    t[ 9] = pSurfel[i].MajorAxis().first;
	    t[10] = pSurfel[i].MinorAxis().second.x;
	    t[11] = pSurfel[i].MinorAxis().second.y;
	    t[12] = pSurfel[i].MinorAxis().second.z;
	    t[13] = pSurfel[i].MinorAxis().first;
	    t[14] = pSurfel[i].Color()[0];
	    t[15] = pSurfel[i].Color()[1];
	    t[16] = pSurfel[i].Color()[2];
	    t[17] = pSurfel[i].MaxError();
	    t[18] = pSurfel[i].MinError();
	    fwrite(t,sizeof(float),19,fpout);

	  }
      }
    else
      {

	for(std::size_t i = 0; i < pSurfel.size(); ++i)
	  {
	    std::cout << pSurfel[i] << std::endl;
	    std::cout << v.cx << std::endl;
	    fprintf(fpout,"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g \n",
		    pSurfel[i].Center().x,
		    pSurfel[i].Center().y,
		    pSurfel[i].Center().z,
		    pSurfel[i].Normal().x,
		    pSurfel[i].Normal().y,
		    pSurfel[i].Normal().z,
		    pSurfel[i].MajorAxis().second.x,
		    pSurfel[i].MajorAxis().second.y,
		    pSurfel[i].MajorAxis().second.z,
		    pSurfel[i].MajorAxis().first,
		    pSurfel[i].MinorAxis().second.x,
		    pSurfel[i].MinorAxis().second.y,
		    pSurfel[i].MinorAxis().second.z,
		    pSurfel[i].MinorAxis().first,
		    pSurfel[i].Color()[0],
		    pSurfel[i].Color()[1],
		    pSurfel[i].Color()[2],
		    pSurfel[i].MaxError(),
		    pSurfel[i].MinError()
		    );
	  }
      }

    fclose(fpout);
    return 0;
  }

  template <class OpenMeshType>
  static bool LoadMask(const char * filename, int &mask)
  {
    vcg::tri::io::PlyInfo pi;
    vcg::ply::PlyFile pf;

    vcg::tri::io::ImporterPLY<OpenMeshType>::LoadMask(filename, mask,pi);


    if( pf.Open(filename,vcg::ply::PlyFile::MODE_READ)==-1 )
      {
	pi.status = pf.GetError();
	return false;
      }

    if( pf.AddToRead(Surfel<Real>::SurfelDesc(0)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(1)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(2)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(3)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(4)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(5)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(6)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(7)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(8)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(9)) !=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(10))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(11))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(12))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(13))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(14))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(15))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(16))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(17))!=-1 &&
	pf.AddToRead(Surfel<Real>::SurfelDesc(18))!=-1
	)
      {
	mask |= Surfel<Real>::IOM_SURFEL;
      }
    return 0;

  }


  virtual ~IOSurfels();
};


#endif /* IMPORTERPLY_HPP_ */
