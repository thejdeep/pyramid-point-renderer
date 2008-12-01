/**
 * Splat renderer file io functions
 * Read file format : sls (Bart Adams), normals
 *
 * Author : Ricardo Marroquim
 *
 * Date created : 17-01-2007
 *
 **/


#include "application.h"

extern "C" {
#include "ply.h"
}

typedef struct Vertex {
  float x,y,z;
  float r,g,b,a;
  float nx,ny,nz;
  float radius;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Vertex_vcg {
  float x,y,z;
  float red,green,blue,alpha;
  float nx,ny,nz;
  float radius;
  void *other_props;       /* other properties */
} Vertex_vcg;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

const char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", Float32, Float32, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(Vertex,z), 0, 0, 0, 0},
  {"nx", Float32, Float32, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", Float32, Float32, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", Float32, Float32, offsetof(Vertex,nz), 0, 0, 0, 0},
  {"radius", Float32, Float32, offsetof(Vertex,radius), 0, 0, 0, 0},
};

PlyProperty vert_props_color2[] = { /* list of property information for a vertex */
  {"x", Float32, Float32, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(Vertex,z), 0, 0, 0, 0},
  {"r", Float32, Float32, offsetof(Vertex,r), 0, 0, 0, 0},
  {"g", Float32, Float32, offsetof(Vertex,g), 0, 0, 0, 0},
  {"b", Float32, Float32, offsetof(Vertex,b), 0, 0, 0, 0},
  {"nx", Float32, Float32, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", Float32, Float32, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", Float32, Float32, offsetof(Vertex,nz), 0, 0, 0, 0},
  {"radius", Float32, Float32, offsetof(Vertex,radius), 0, 0, 0, 0},
};


PlyProperty vert_props_color[] = { /* list of property information for a vertex */
  {"x", Float32, Float32, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(Vertex,z), 0, 0, 0, 0},
  {"nx", Float32, Float32, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", Float32, Float32, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", Float32, Float32, offsetof(Vertex,nz), 0, 0, 0, 0},
  {"red", Float32, Float32, offsetof(Vertex,r), 0, 0, 0, 0},
  {"green", Float32, Float32, offsetof(Vertex,g), 0, 0, 0, 0},
  {"blue", Float32, Float32, offsetof(Vertex,b), 0, 0, 0, 0},
  {"alpha", Float32, Float32, offsetof(Vertex,a), 0, 0, 0, 0}
};

PlyProperty face_props[] = { /* list of property information for a face */
  {"vertex_indices", Int32, Int32, offsetof(Face,verts),
   1, Uint8, Uint8, offsetof(Face,nverts)},
};


/*** the PLY object ***/

/* static PlyFile *in_ply; */
/* static int nverts, nfaces; */
/* /\* static Vertex **vlist; *\/ */
/* /\* static Face **flist; *\/ */
/* static PlyOtherProp *vert_other, *face_other; */


//compensation factors for centralizing polygon (avoid deformation in smaller dimension)
static double compX, compY, compZ;
static double c_max, c_min;
static Point p_max, p_min;
Point p;

void computeNormFactors (vector<Surfeld> * surfels) {

  double center = 0.0;

  p = surfels->begin()->Center();
  c_max = c_min = 0;
  p_max = p;
  p_min = p;

  //compute min and max values for x and y coordinates
  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
    p = it->Center();
    if (p[0] > p_max[0]) p_max[0] = p[0];
    if (p[0] < p_min[0]) p_min[0] = p[0];
    if (p[1] > p_max[1]) p_max[1] = p[1];
    if (p[1] < p_min[1]) p_min[1] = p[1];
    if (p[2] > p_max[2]) p_max[2] = p[2];
    if (p[2] < p_min[2]) p_min[2] = p[2];
  }

  //Max and min points for normalizing polygon points
  if ((p_max[0] - p_min[0] > p_max[1] - p_min[1]) && (p_max[0] - p_min[0] > p_max[2] - p_min[2])) {
	c_max = p_max[0];
	c_min = p_min[0];
	compX = 0;
	center = (p_max[1] + p_min[1])*0.5;
	compY = 0.0 - (((center - p_min[1]) / (c_max - c_min)) * 2.0 - 1.0);
	center = (p_max[2] + p_min[2])*0.5;
	compY = 0.0 - (((center - p_min[2]) / (c_max - c_min)) * 2.0 - 1.0);
  }
  else if (p_max[1] - p_min[1] > p_max[2] - p_min[2]) {
	c_max = p_max[1];
	c_min = p_min[1];
	center = (p_max[0] + p_min[0])*0.5;
	compX = 0.0 - (((center - p_min[0]) / (c_max - c_min)) * 2.0 - 1.0);
	compY = 0;
	center = (p_max[2] + p_min[2])*0.5;
	compY = 0.0 - (((center - p_min[2]) / (c_max - c_min)) * 2.0 - 1.0);
  }
  else {
	c_max = p_max[2];
	c_min = p_min[2];
	center = (p_max[0] + p_min[0])*0.5;
	compX = 0.0 - (((center - p_min[0]) / (c_max - c_min)) * 2.0 - 1.0);
	center = (p_max[1] + p_min[1])*0.5;
	compY = 0.0 - (((center - p_min[1]) / (c_max - c_min)) * 2.0 - 1.0);
	compZ = 0;
  }

  //compX = compY = compZ = 0.00;
}

/* /\* /// Normalize the input points in the range [-1, 1]. *\/ */
void normalize (vector<Surfeld> * surfels) {
 
/*   Point p; */
  p = surfels->begin()->Center();
  Vector n;
  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
    p = it->Center();
    Point c = Point ((((p[0] - p_min[0]) / (c_max - c_min))) * 2.0 - 1.0 + compX,
			   (((p[1] - p_min[1]) / (c_max - c_min))) * 2.0 - 1.0 + compY,
			   (((p[2] - p_min[2]) / (c_max - c_min))) * 2.0 - 1.0 + compZ);
    n = it->Normal();
    n.Normalize();
	
	it->SetCenter( c );
	it->SetNormal( n );
  }

}



void readPlyTriangles (const char *filename, vector<Surfeld> *surfels,
					   vector<Triangle> *triangles, Point rgb = Point()) {

  PlyFile *in_ply;
  int nverts, nfaces;
  PlyOtherProp *vert_other=0, *face_other=0;

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  const char *elem_name;
  
  for (i = 0; i < in_ply->num_elem_types; i++) {

    /* prepare to read the i'th list of elements */
    elem_name = setup_element_read_ply (in_ply, i, &elem_count);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      nverts = elem_count;

      /* set up for getting vertex elements */
      setup_property_ply (in_ply, &vert_props[0]);
      setup_property_ply (in_ply, &vert_props[1]);
      setup_property_ply (in_ply, &vert_props[2]);

      for (j = 0; j < in_ply->elems[i]->nprops; j++) {
		PlyProperty *prop;
		prop = in_ply->elems[i]->props[j];
		if (equal_strings ("nx", prop->name))
		  setup_property_ply (in_ply, &vert_props[3]);
		if (equal_strings ("ny", prop->name))
		  setup_property_ply (in_ply, &vert_props[4]);
		if (equal_strings ("nz", prop->name))
		  setup_property_ply (in_ply, &vert_props[5]);
		if (equal_strings ("radius", prop->name))
		  setup_property_ply (in_ply, &vert_props[6]);
      }

      vert_other = get_other_properties_ply (in_ply, 
											 offsetof(Vertex,other_props));

      Vertex v;
      /* grab all the vertex elements */
      for (j = 0; j < elem_count; j++) {

        get_element_ply (in_ply, (void *) &v);

		Point p (v.x, v.y, v.z);
		Vector n (v.nx, v.ny, v.nz);
	
		surfels->push_back ( Surfeld (p, n, rgb, (double)v.radius, j) );
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      nfaces = elem_count;

      /* set up for getting face elements */
      setup_property_ply (in_ply, &face_props[0]);
      face_other = get_other_properties_ply (in_ply,
											 offsetof(Face,other_props));
      Face f;
      /* grab all the face elements */
      for (j = 0; j < elem_count; j++) {
		get_element_ply (in_ply, (void *) &f);
		Triangle t;
		for (int k = 0; k < (int)f.nverts; ++k)
		  t.verts[k] = f.verts[k];
		if ((int)f.nverts == 2)
		  t.verts[2] = f.verts[1];
		t.id = j;
		triangles->push_back( t );
      }      
    }
    else
      get_other_element_ply (in_ply);
  }
  close_ply(in_ply);
  delete in_ply;
  delete vert_other;
  delete face_other;
}

void readPlyTrianglesColor (const char *filename, vector<Surfeld> *surfels,
							vector<Triangle> *triangles) {

  PlyFile *in_ply;
  int nverts, nfaces;
  PlyOtherProp *vert_other=0, *face_other=0;

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  const char *elem_name;

  if (!in_ply) {
    cerr << "bad filename!" << endl;
    return;
  }

  for (i = 0; i < in_ply->num_elem_types; i++) {

    /* prepare to read the i'th list of elements */
    elem_name = setup_element_read_ply (in_ply, i, &elem_count);
	vert_other = get_other_properties_ply (in_ply, 
										   offsetof(Vertex,other_props));
    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      nverts = elem_count;

      /* set up for getting vertex elements */
      setup_property_ply (in_ply, &vert_props_color[0]);
      setup_property_ply (in_ply, &vert_props_color[1]);
      setup_property_ply (in_ply, &vert_props_color[2]);

      for (j = 0; j < in_ply->elems[i]->nprops; j++) {
		PlyProperty *prop;
		prop = in_ply->elems[i]->props[j];
		if (equal_strings ("nx", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[3]);
		if (equal_strings ("ny", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[4]);
		if (equal_strings ("nz", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[5]);
		if (equal_strings ("red", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[6]);
		if (equal_strings ("green", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[7]);
		if (equal_strings ("blue", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[8]);
		if (equal_strings ("alpha", prop->name))
		  setup_property_ply (in_ply, &vert_props_color[9]);

      }

      Vertex_vcg v;
      /* grab all the vertex elements */
      for (j = 0; j < elem_count; j++) {

        get_element_ply (in_ply, (void *) &v);

		Point p (v.x, v.y, v.z);
		Vector n (v.nx, v.ny, v.nz);
		Color c (v.red/255.0, v.green/255.0, v.blue/255.0);

		//		double r = v.radius;
		double r = 0.001;
      
		surfels->push_back ( Surfeld (p, n, c, r, (unsigned int)j) );
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      nfaces = elem_count;

      /* set up for getting face elements */
      setup_property_ply (in_ply, &face_props[0]);
	  face_other = get_other_properties_ply (in_ply,
											 offsetof(Face,other_props));
      Face f;
      /* grab all the face elements */
      for (j = 0; j < elem_count; j++) {
		get_element_ply (in_ply, (void *) &f);
		Triangle t;
		for (int k = 0; k < f.nverts; ++k)
		  t.verts[k] = f.verts[k];
		t.id = j;
		triangles->push_back( t );	
      }
    }
    else
      get_other_element_ply (in_ply);
  }
  close_ply(in_ply);
  delete in_ply;
  delete vert_other;
  delete face_other;
}


void MYreadPlyTrianglesColor (const char *filename, vector<Surfeld> *surfels,
							vector<Triangle> *triangles) {

  PlyFile *in_ply;
  int nverts, nfaces;
  //  PlyOtherProp *vert_other=0, *face_other=0;

  FILE *fp = fopen(filename, "rb");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  const char *elem_name;

  if (!in_ply) {
    cerr << "bad filename!" << endl;
    return;
  }

  for (i = 0; i < in_ply->num_elem_types; i++) {

    /* prepare to read the i'th list of elements */
    elem_name = setup_element_read_ply (in_ply, i, &elem_count);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      nverts = elem_count;

      /* set up for getting vertex elements */
      setup_property_ply (in_ply, &vert_props_color2[0]);
      setup_property_ply (in_ply, &vert_props_color2[1]);
      setup_property_ply (in_ply, &vert_props_color2[2]);

      for (j = 0; j < in_ply->elems[i]->nprops; j++) {
		PlyProperty *prop;
		prop = in_ply->elems[i]->props[j];
		if (equal_strings ("r", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[3]);
		if (equal_strings ("g", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[4]);
		if (equal_strings ("b", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[5]);
		if (equal_strings ("nx", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[6]);
		if (equal_strings ("ny", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[7]);
		if (equal_strings ("nz", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[8]);
		if (equal_strings ("radius", prop->name))
		  setup_property_ply (in_ply, &vert_props_color2[9]);

      }

	  //      vert_other = get_other_properties_ply (in_ply, offsetof(Vertex,other_props));
      Vertex v;
      /* grab all the vertex elements */
      for (j = 0; j < elem_count; j++) {

        get_element_ply (in_ply, (void *) &v);

		Point p (v.x, v.y, v.z);
		Vector n (v.nx, v.ny, v.nz);
		Color c (v.r/255.0, v.g/255.0, v.b/255.0);
		double r = v.radius;
      
		surfels->push_back ( Surfeld (p, n, c, r, (unsigned int)j) );
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      nfaces = elem_count;

      /* set up for getting face elements */
      setup_property_ply (in_ply, &face_props[0]);
	  //      face_other = get_other_properties_ply (in_ply, offsetof(Face,other_props));
      Face f;
      /* grab all the face elements */
      for (j = 0; j < elem_count; j++) {
		get_element_ply (in_ply, (void *) &f);
		Triangle t;
		for (int k = 0; k < f.nverts; ++k)
		  t.verts[k] = f.verts[k];
		t.id = j;
		triangles->push_back( t );	
      }
    }
    else
      get_other_element_ply (in_ply);
  }
  close_ply(in_ply);
  free_ply(in_ply);
}
