/*
 * ImpoterPLY.hpp
 *
 *  Created on: Nov 2, 2009
 *      Author: fmc
 */

#ifndef IMPOTERPLY_HPP_
#define IMPOTERPLY_HPP_

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

// Local
#include "Math/BoundingBox3.hpp"
#include "Surfel.hpp"

class CEdge;
class CFaceO;
class CVertexO;

//Vert Mem Occupancy  --- 40b ---

class CVertexO  : public vcg::VertexSimp2< CVertexO, CEdge, CFaceO,
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
  >{
};

//
class CEdge : public vcg::EdgeSimp2<CVertexO,CEdge,CFaceO, vcg::edge::EVAdj> {
public:
	inline CEdge(){};
	inline CEdge( CVertexO * v0, CVertexO * v1)
	{
		V(0)= v0 ; V(1)= v1;
	};
	static inline CEdge OrderedEdge(CVertexO* v0,CVertexO* v1)
	{
		if(v0<v1)
		{
			return CEdge(v0,v1);
		}
		else {
			return CEdge(v1,v0);
		}
	}
};

//Face Mem Occupancy  --- 32b ---

class CFaceO    : public vcg::FaceSimp2<  CVertexO, CEdge, CFaceO,
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

class CMesh    : public vcg::tri::TriMesh< vcg::vertex::vector_ocf<CVertexO>, vcg::face::vector_ocf<CFaceO> > {
public :
	vcg::Matrix44f Tr; // Usually it is the identity. It is applied in rendering and filters can or cannot use it. (most of the filter will ignore this)
	const vcg::Box3f &trBB()
	{
		static vcg::Box3f bb;
		bb.SetNull();
		bb.Add(Tr,bbox);
		return bb;
	}
};


namespace Celer {

template<class Real = float> class IOSurfels{
public:

	typedef ::vcg::ply::PropDescriptor PropDescriptor ;

	typedef typename CMesh::VertexIterator VertexIterator;

	IOSurfels()
	{

	}


	static int SaveMesh (const char * filename,
			std::vector<Surfel<Real> >& pSurfel,vcg::CallBackPos *cb)
	{
		CMesh mesh;
        VertexIterator vi = vcg::tri::Allocator<CMesh>::AddVertices(mesh,pSurfel.size());
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

		int result = vcg::tri::io::ExporterPLY<CMesh>::Save(mesh,filename,binary,info,cb);


	}
	static int LoadMesh (
			const char * filename,
			std::vector<Surfel<Real> >& pSurfel,
			Celer::BoundingBox3<Real>& pBox,vcg::CallBackPos *cb)
	{
		CMesh mesh;

		int mask = 0;

		mesh.Clear();

		if (cb != NULL) 	(*cb)(0, "Loading Model...");


		vcg::tri::io::PlyInfo info;
		info.cb = cb;

		vcg::tri::io::ImporterPLY<CMesh>::LoadMask(filename, mask,info);
		int i = vcg::tri::io::ImporterPLY<CMesh>::Open(mesh,filename,info);


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

		for (CMesh::VertexIterator vit = mesh.vert.begin(); vit != mesh.vert.end(); ++vit)
		{
		        Celer::Vector3<Real> v = Celer::Vector3<Real> ((*vit).P().X(),(*vit).P().Y(),(*vit).P().Z());
		        Celer::Vector3<Real> n = Celer::Vector3<Real> ((*vit).N().X(),(*vit).N().Y(),(*vit).N().Z());

		        Celer::Color c = Celer::Color (0.2,0.2,0.2);
		        if(color_per_vertex)
		        {
		        	 c = Celer::Color ((*vit).C().X(),(*vit).C().Y(),(*vit).C().Z());
		        }

		        Real radius = 0.25;
		        if (radius_per_vertex)
		        {
		        	radius = static_cast<Real> (((*vit).R()));
		        }

		        s = Surfel<Real> (v, n, c,radius,pos);

		        s.SetRadius(radius);

		        pSurfel.push_back (s);
				pBox = pBox + Celer::BoundingBox3<Real>(s.Center(0),s.Center(1),s.Center(2),
													    s.Center(0),s.Center(1),s.Center(2));
		        ++pos;

		  }
		std::cout << "Surfel " << pSurfel.size() << std::endl;
		if (cb != NULL)	(*cb)(99, "Done");
		mesh.Clear();
		return 0;
	}

	static int LoadSurfels (const char * filename,std::vector<Surfel<Real> >& pSurfel,vcg::CallBackPos *cb)
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
			if( pf.AddToRead(Surfel<>::SurfelDesc(i))==-1 )
			{
				found = false;

				break;
			}
		}

		if(!found)
			return pi.status;


		for(i=0;i<int(pf.elements.size());i++)
		{
			int n = pf.ElemNumber(i);

			if( !strcmp( pf.ElemName(i),"surfel" ) )
			{
				pf.SetCurElement(i);

				typename Surfel<Real>::LoadPlySurfel vs;

				for(int j=0;j<n;++j)
				{
					if( pf.Read( (void *)&(vs) )==-1 )
					{
						pi.status = vcg::tri::io::PlyInfo::E_SHORTFILE;
						return pi.status;

					}

					pSurfel.push_back(Celer::Surfel<Real>(vs));
				}

				break;
			}
		}

		return 0;
	}


	static int SaveSurfels(std::vector<Surfel<Real> >& pSurfel,  const char * filename,vcg::CallBackPos *cb, bool binary=0)	// V1.0
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
				"property float nx\n"
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
				v = pSurfel[i];
				t[ 0] = (float)v.cx;
				t[ 1] = (float)v.cy;
				t[ 2] = (float)v.cz;
				t[ 3] = (float)v.nx;
				t[ 4] = (float)v.ny;
				t[ 5] = (float)v.nz;
				t[ 6] = (float)v.major_axisx;
				t[ 7] = (float)v.major_axisy;
				t[ 8] = (float)v.major_axisz;
				t[ 9] = (float)v.major_axis_size;
				t[10] = (float)v.minor_axisx;
				t[11] = (float)v.minor_axisy;
				t[12] = (float)v.minor_axisz;
				t[13] = (float)v.minor_axis_size;
				t[14] = (float)v.r;
				t[15] = (float)v.g;
				t[16] = (float)v.b;
				t[17] = (float)v.max_error;
				t[18] = (float)v.min_error;
				fwrite(t,sizeof(float),19,fpout);

			}
		}
		else
		{

			for(std::size_t i = 0; i < pSurfel.size(); ++i)
			{
				v = pSurfel[i];
				fprintf(fpout,"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g \n",
						v.cx,
						v.cy,
						v.cz,
						v.nx,
						v.ny,
						v.nz,
						v.major_axisx,
						v.major_axisy,
						v.major_axisz,
						v.major_axis_size,
						v.minor_axisx,
						v.minor_axisy,
						v.minor_axisz,
						v.minor_axis_size,
						v.r,
						v.g,
						v.b,
						v.max_error,
						v.min_error
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

		if( pf.AddToRead(Surfel<>::SurfelDesc(0)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(1)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(2)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(3)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(4)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(5)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(6)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(7)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(8)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(9)) !=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(10))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(11))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(12))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(13))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(14))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(15))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(16))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(17))!=-1 &&
				pf.AddToRead(Surfel<>::SurfelDesc(18))!=-1
		)
		{
			mask |= Surfel<>::IOM_SURFEL;
		}
		return 0;

	}


	virtual ~IOSurfels();
};

}

#endif /* IMPOTERPLY_HPP_ */
