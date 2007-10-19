/**
 * Splat renderer file io functions
 * Read file format : sls (Bart Adams), normals
 *
 * Author : Ricardo Marroquim
 *
 * Date created : 17-01-2007
 *
 **/

#include "point_render.h"

extern "C" {
#include "ply.h"
}

typedef struct Vertex {
  float x,y,z;
  float nx,ny,nz;
  float radius;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
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

PlyProperty face_props[] = { /* list of property information for a face */
  {"vertex_indices", Int32, Int32, offsetof(Face,verts),
   1, Uint8, Uint8, offsetof(Face,nverts)},
};


/*** the PLY object ***/

static PlyFile *in_ply;
static int nverts, nfaces;
/* static Vertex **vlist; */
/* static Face **flist; */
static PlyOtherProp *vert_other, *face_other;

/// Normalize the input points in the range [-1, 1].
void normalize (vector<Surfel> * surfels) {

  double max = 0, min = 0;
  Point p = surfels->begin()->position();
  double xMax=p.x(), yMax=p.y(), zMax=p.z(), xMin=p.x(), yMin=p.y(), zMin=p.z();

  //compute min and max values for x and y coordinates
  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
    p = it->position();
    if (p[0] > xMax) xMax = p[0];
    if (p[0] < xMin) xMin = p[0];
    if (p[1] > yMax) yMax = p[1];
    if (p[1] < yMin) yMin = p[1];
    if (p[2] > zMax) zMax = p[2];
    if (p[2] < zMin) zMin = p[2];
  }

  //compensation factors for centralizing polygon (avoid deformation in smaller dimension)
  double compX, compY, compZ, center;

  //Max and min points for normalizing polygon points
  if ((xMax - xMin > yMax - yMin) && (xMax - xMin > zMax - zMin)) {
      max = xMax;
      min = xMin;
      compX = 0;
      center = (yMax + yMin)*0.5;
      compY = 0.0 - (((center - yMin) / (max - min)) * 2.0 - 1.0);
      center = (zMax + zMin)*0.5;
      compY = 0.0 - (((center - zMin) / (max - min)) * 2.0 - 1.0);
    }
  else if (yMax - yMin > zMax - zMin) {
      max = yMax;
      min = yMin;
      center = (xMax + xMin)*0.5;
      compX = 0.0 - (((center - xMin) / (max - min)) * 2.0 - 1.0);
      compY = 0;
      center = (zMax + zMin)*0.5;
      compY = 0.0 - (((center - zMin) / (max - min)) * 2.0 - 1.0);
    }
  else {
      max = zMax;
      min = zMin;
      center = (xMax + xMin)*0.5;
      compX = 0.0 - (((center - xMin) / (max - min)) * 2.0 - 1.0);
      center = (yMax + yMin)*0.5;
      compY = 0.0 - (((center - yMin) / (max - min)) * 2.0 - 1.0);
      compZ = 0;
    }

  compX = compY = compZ = 0;

  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
    p = it->position();    
    p = Point ((((p[0] - xMin) / (max - min))) * 2.0 - 1.0 + compX,
	       (((p[1] - yMin) / (max - min))) * 2.0 - 1.0 + compY,
	       (((p[2] - zMin) / (max - min))) * 2.0 - 1.0 + compZ);
    ((Surfel*)&(*it))->setPosition( (Point) p );
  }

}


/// Loads a sls 3d file and insert all points in the kd-tree
/// @param filenam Given plg filename
/// @return 1 if file read with success, 0 otherwise
bool loadSls (const char * filename, vector<Surfel> *surfels) {  
  std::ifstream input (filename);

  if(input.fail())
    return false;

  int number_points;
  input >> number_points;

  double x, y, z, cr, cg, cb, nx, ny, nz, r;
  for (int i = 0; i < number_points; ++i) {
    input >> x >> y >> z >> cr >> cg >> cb >> nx >> ny >> nz >> r;
    Point p (x, y, z);
    Vector n (nx, ny, nz);
    surfels->push_back ( Surfel (p, n, r, i) );
  }

  return true;
}

/// Read an normals file, contains positions, normals and splat radius for each point
/// @param filename Given off filename
/// @return 1 if file read with success, 0 otherwise
bool loadNormals (const char * filename, vector<Surfel> *surfels) {  
  std::ifstream input (filename);

  if(input.fail())
    return false;

  int number_points;
  input >> number_points;

  double x, y, z, nx, ny, nz, r;
  for (int i = 0; i < number_points; ++i)
    {
      input >> x >> y >> z >> nx >> ny >> nz >> r;
      Point p (x, y, z);
      Vector n (nx, ny, nz);
      surfels->push_back ( Surfel (p, n, r, i) );
    }

  return true;
}

void readPly (const char *filename, vector<Surfel> *surfels) {

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  char *elem_name;
  
  for (i = 0; i < in_ply->num_elem_types; i++) {

    /* prepare to read the i'th list of elements */
    elem_name = setup_element_read_ply (in_ply, i, &elem_count);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex vector to hold all the vertices */
      //vlist = (Vertex **) malloc (sizeof (Vertex *) * elem_count);
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
	
	surfels->push_back ( Surfel (p, n, (double)v.radius, j) );
      }
    }

  }
  close_ply(in_ply);
}

/** 
    Creates one surfel per triangle instead of per vertex.
    Each surfel has position at the triangles's barycenter
    and normal as average of the three vertices. The radius
    is two times the distance from the barycenter to a vertex.
 **/
void readPlyHighRes (const char *filename, vector<Surfel> *surfels) {

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  char *elem_name;
  
  vector<Surfel> temp_surfels;
  for (i = 0; i < in_ply->num_elem_types; i++) {

    /* prepare to read the i'th vector of elements */
    elem_name = setup_element_read_ply (in_ply, i, &elem_count);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex vector to hold all the vertices */
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
	
	temp_surfels.push_back ( Surfel (p, n, (double)v.radius, j) );
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
	Surfel s[3];
	s[0] = temp_surfels[ f.verts[0] ];
	s[1] = temp_surfels[ f.verts[1] ];
	s[2] = temp_surfels[ f.verts[2] ];

	Point p = Point( (s[0].p.x() + s[1].p.x() + s[2].p.x()) / 3.0,
			 (s[0].p.y() + s[1].p.y() + s[2].p.y()) / 3.0,
			 (s[0].p.z() + s[1].p.z() + s[2].p.z()) / 3.0);

	Vector n = Vector( (s[0].n.x() + s[1].n.x() + s[2].n.x()) / 3.0,
			   (s[0].n.y() + s[1].n.y() + s[2].n.y()) / 3.0,
			   (s[0].n.z() + s[1].n.z() + s[2].n.z()) / 3.0);

	double max_dist = 0.0;
	for (int k = 0; k < 3; ++k) {
	  double length = (p - s[k].p).length();
	  if (max_dist < length)
	    max_dist = length;
	}

	max_dist *= 2.0;

	surfels->push_back ( Surfel (p, n, max_dist, j) );

      }
    }
    else
      get_other_element_ply (in_ply);
  }
  close_ply(in_ply);
}

void readPlyTriangles (const char *filename, vector<Surfel> *surfels,
		       vector<Triangle> *triangles) {

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  char *elem_name;
  
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
	
	surfels->push_back ( Surfel (p, n, (double)v.radius, j) );
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
	for (int k = 0; k < 3; ++k)
	  t.verts[k] = f.verts[k];
	t.id = j;
	triangles->push_back( t );	
      }      
    }
    else
      get_other_element_ply (in_ply);
  }
  close_ply(in_ply);
}

int readModels (int argc, char **argv, vector<Object> *objs) {

  // For each model passed in command line
  for (int i = 1; i < argc; ++i) {

    objs->push_back( Object(i-1) );
    readPlyTriangles (argv[i], 
		      (objs->at(i-1)).getSurfels(), 
		      (objs->at(i-1)).getTriangles());
  }

  return 1;
}

int readPointsAndTriangles(int argc, char **argv, vector<Surfel> *surfels,
			   vector<Triangle> *triangles){

  for (int i = 1; i < argc; ++i)
    readPlyTriangles (argv[i], surfels, triangles);

  return 1;
}

/// Command line argument processor
int readPoints(int argc, char **argv, vector<Surfel> *surfels) {

  // Use default file
  if (argc < 2) {
    if (loadSls ("mannequin.sls", surfels))
      return 1;
  }

  readPly (argv[1], surfels);

  cout << "ply ok!" << endl;
  return 1;

  if (loadNormals (argv[1], surfels))
    return 1;

  //  normalize(surfels);


  std::cerr << "Could not read file : " << argv [2] << "\n";
  std::cerr << "Usage: point_render <normals file>" << "\n";
  return 0;
}
