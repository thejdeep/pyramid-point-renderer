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
  float r,g,b;
  float nx,ny,nz;
  float radius;
  void *other_props;       /* other properties */
} Vertex;

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

PlyProperty vert_props_color[] = { /* list of property information for a vertex */
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
void normalize (vector<Surfeld> * surfels) {

  double max = 0, min = 0;
  Point p = surfels->begin()->Center();
  double xMax=p[0], yMax=p[1], zMax=p[2], xMin=p[0], yMin=p[1], zMin=p[2];

  //compute min and max values for x and y coordinates
  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
    p = it->Center();
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

  Vector n;
  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
    p = it->Center();    
    p = Point ((((p[0] - xMin) / (max - min))) * 2.0 - 1.0 + compX,
	       (((p[1] - yMin) / (max - min))) * 2.0 - 1.0 + compY,
	       (((p[2] - zMin) / (max - min))) * 2.0 - 1.0 + compZ);
    n = it->Normal();
    n.Normalize();
    ((Surfeld*)&(*it))->SetCenter( (Point) p );
    ((Surfeld*)&(*it))->SetNormal( (Vector) n );

  }

}


/// Loads a sls 3d file and insert all points in the kd-tree
/// @param filenam Given plg filename
/// @return 1 if file read with success, 0 otherwise
bool loadSls (const char * filename, vector<Surfeld> *surfels) {  
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
    surfels->push_back ( Surfeld (p, n, r, i) );
  }

  return true;
}

/// Read an normals file, contains positions, normals and splat radius for each point
/// @param filename Given off filename
/// @return 1 if file read with success, 0 otherwise
bool readNormals (const char * filename, vector<Surfeld> *surfels) {  
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
      surfels->push_back ( Surfeld (p, n, r, i) );
    }

  return true;
}

void readPly (const char *filename, vector<Surfeld> *surfels) {

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  const char *elem_name;
  
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
	
	surfels->push_back ( Surfeld (p, n, (double)v.radius, j) );
      }
    }

  }
  close_ply(in_ply);
}

void readPlyTriangles (const char *filename, vector<Surfeld> *surfels,
		       vector<Triangle> *triangles, Point rgb = Point()) {

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
}

void readPlyTrianglesColor (const char *filename, vector<Surfeld> *surfels,
		       vector<Triangle> *triangles) {

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
	if (equal_strings ("r", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[3]);
	if (equal_strings ("g", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[4]);
	if (equal_strings ("b", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[5]);
	if (equal_strings ("nx", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[6]);
	if (equal_strings ("ny", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[7]);
	if (equal_strings ("nz", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[8]);
	if (equal_strings ("radius", prop->name))
	  setup_property_ply (in_ply, &vert_props_color[9]);

      }

      vert_other = get_other_properties_ply (in_ply, 
					     offsetof(Vertex,other_props));

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

}

int readModels (int argc, char **argv, vector<Primitives> *prims, vector<Object> *objs) {

  for (int i = 1; i < argc; ++i) {
    if (strstr(argv[i], ".normals") != NULL) { 
      prims->push_back( Primitives(i-1) );
      readNormals (argv[i], (prims->at(i-1)).getSurfels());
    }
    else if (strstr(argv[i], ".sls") != NULL) { 
      prims->push_back( Primitives(i-1) );
      loadSls (argv[i], (prims->at(i-1)).getSurfels());
    }
    else {
      prims->push_back( Primitives(i-1) );
      readPlyTriangles (argv[i], (prims->at(i-1)).getSurfels(), (prims->at(i-1)).getTriangles());
    }
  }

  int i = 0;
  for (vector<Primitives>::iterator it = prims->begin(); it != prims->end(); ++it, ++i) {
    objs->push_back( Object(i) );
    (objs->at(i)).addPrimitives( it->getId() );
    it->setType( 1.0 );
    it->setRendererType( PYRAMID_POINTS );
  }

  return true;
}



int readPointsAndTriangles(int argc, char **argv, vector<Surfeld> *surfels,
			   vector<Triangle> *triangles){
    
  for (int i = 1; i < argc; ++i)
    readPlyTriangles (argv[i], surfels, triangles);

  return 1;
}

/// Command line argument processor
int readPoints(int argc, char **argv, vector<Surfeld> *surfels) {

  // Use default file
  if (argc < 2) {
    if (loadSls ("mannequin.sls", surfels))
      return 1;
  }

  readPly (argv[1], surfels);

  cout << "ply ok!" << endl;
  return 1;

  if (readNormals (argv[1], surfels))
    return 1;

  //  normalize(surfels);


  std::cerr << "Could not read file : " << argv [2] << "\n";
  std::cerr << "Usage: point_render <normals file>" << "\n";
  return 0;
}
