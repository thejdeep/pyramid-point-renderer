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
  double xMax=p.x(), yMax=p.y(), zMax=p.z(), xMin=p.x(), yMin=p.y(), zMin=p.z();

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
    n.normalize();
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
bool loadNormals (const char * filename, vector<Surfeld> *surfels) {  
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

/** 
    Creates one surfel per triangle instead of per vertex.
    Each surfel has position at the triangles's barycenter
    and normal as average of the three vertices. The radius
    is two times the distance from the barycenter to a vertex.
 **/
void readPlyHighRes (const char *filename, vector<Surfeld> *surfels) {

  FILE *fp = fopen(filename, "r");
  in_ply = read_ply(fp);

  int i,j;
  int elem_count;
  const char *elem_name;
  
  vector<Surfeld> temp_surfels;
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
	
	temp_surfels.push_back ( Surfeld (p, n, (double)v.radius, j) );
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
	Surfeld s[3];
	s[0] = temp_surfels[ f.verts[0] ];
	s[1] = temp_surfels[ f.verts[1] ];
	s[2] = temp_surfels[ f.verts[2] ];

	Point p = Point( (s[0].Center().x() + s[1].Center().x() + s[2].Center().x()) / 3.0,
			 (s[0].Center().y() + s[1].Center().y() + s[2].Center().y()) / 3.0,
			 (s[0].Center().z() + s[1].Center().z() + s[2].Center().z()) / 3.0);

	Vector n = Vector( (s[0].Normal().x() + s[1].Normal().x() + s[2].Normal().x()) / 3.0,
			   (s[0].Normal().y() + s[1].Normal().y() + s[2].Normal().y()) / 3.0,
			   (s[0].Normal().z() + s[1].Normal().z() + s[2].Normal().z()) / 3.0);

	double max_dist = 0.0;
	for (int k = 0; k < 3; ++k) {
	  double length = (p - s[k].Center()).length();
	  if (max_dist < length)
	    max_dist = length;
	}

	max_dist *= 2.0;

	surfels->push_back ( Surfeld (p, n, max_dist, j) );

      }
    }
    else
      get_other_element_ply (in_ply);
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

int readObjsFiles (const char* filename, vector<Primitives> *prims, vector<Object> *objs, vector<int> *objs_ids, Camera *camera) {
  ifstream in (filename);

  if (in.fail()) return false;

  char comments[255];
  int num_primitives, num_objects;

  in.getline (comments, 255);

  in >> comments >> num_primitives;
  in >> comments >> num_objects;

  int id;
  char ply_file[num_primitives][100];
  double type;
  int renderer_type;
  Point rgb;
  int material;
  vector< char* > filenames;

  for (int i = 0; i < num_primitives; ++i) {   
    in >> id >> ply_file[i] >> type >> renderer_type >> material;
    prims->push_back ( Primitives(id, type) );
    prims->back().setType( type );
    prims->back().setPerVertexColor( 1 );
    prims->back().setMaterial(material);
    filenames.push_back( ply_file[i] );
    
    if (strstr(ply_file[i], ".lod") != NULL ) {

      char fn[200];
      strcpy(fn, ply_file[i]);
      char* ptr = strstr(fn, ".lod");
      strncpy (ptr, "", 4);
      readPlyTriangles (fn, (prims->back()).getSurfels(), (prims->back()).getTriangles(), rgb);
      prims->back().readFileLOD(ply_file[i]);
    }
    else
      readPlyTriangles (ply_file[i], (prims->back()).getSurfels(), (prims->back()).getTriangles(), rgb);

    // Must call after reading file because this next function creates
    // the vertex array or display lists, and needs the surfels structure loaded
    prims->back().setRendererType( renderer_type );
  }

  for (int i = num_primitives; i < num_objects; ++i)
    filenames.push_back( ply_file[0] );

  Quat q;
  double x, y, z;
  int n, prim_id;
  for (int i = 0; i < num_objects; ++i) {
    cout << "pass " << i << endl;
    in >> id >> x >> y >> z >> q.x >> q.y >> q.z >> q.a >> n;

    objs_ids->push_back (id);
    objs->push_back( Object(id, x, y, z, q) );
    cout << "name :   ";
    cout << filenames[i] << endl;
    objs->at(id).setFilename( filenames[i] );    

    for (int j = 0; j < n; ++j) {
      in >> prim_id;
      (objs->at(id)).addPrimitives( prim_id );
    }
  }


  // read camera attributes
  Point camera_pos;
  GLfloat light_pos[3];
  in >> camera_pos[0] >> camera_pos[1] >> camera_pos[2] >> q.x >> q.y >> q.z >> q.a;
  camera->setPositionVector( camera_pos );
  camera->setRotationQuat( q );
  in >> light_pos[0] >> light_pos[1] >> light_pos[2];
  camera->setLightVector( light_pos );

  in.close();
  return num_objects;
}

int readTreeFiles (const char* filename, vector<Primitives> *prims, vector<Object> *objs, vector<int> *objs_ids) {
  ifstream in (filename);

  if (in.fail()) return false;

  char comments[255];
  int num_primitives, num_objects;

  in.getline (comments, 255);

  in >> comments >> num_primitives;
  in >> comments >> num_objects;

  int id;
  char ply_file[100];
  double type;
  int renderer_type;
  Point rgb;

  for (int i = 0; i < num_primitives; ++i) {   
    in >> id >> ply_file >> type >> renderer_type >> rgb[0] >> rgb[1] >> rgb[2];

    prims->push_back ( Primitives(id, type) );
    prims->back().setType( type );
    prims->back().setPerVertexColor( 1 );

    readPlyTriangles (ply_file, (prims->back()).getSurfels(), (prims->back()).getTriangles(), rgb);

    // Must call after reading file because this next function creates
    // the vertex array or display lists, and needs the surfels structure loaded
    prims->back().setRendererType( renderer_type );
  }

  Quat q;
  double x, y, z;
  int n, prim_id;
  for (int i = 0; i < num_objects; ++i) {    
    in >> id >> x >> y >> z >> q.x >> q.y >> q.z >> q.a >> n;
    objs_ids->push_back (id);
    objs->push_back( Object(id, x, y, z, q) );
    for (int j = 0; j < n; ++j) {
      in >> prim_id;
      (objs->at(id)).addPrimitives( prim_id );
    }
  }

  return num_objects;
}

int readModels (int argc, char **argv, vector<Primitives> *prims, vector<Object> *objs) {

  for (int i = 1; i < argc; ++i) {
    if (strstr(argv[i], ".normals") != NULL) { 
      prims->push_back( Primitives(i-1) );
      loadNormals (argv[i], (prims->at(i-1)).getSurfels());
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

  if (loadNormals (argv[1], surfels))
    return 1;

  //  normalize(surfels);


  std::cerr << "Could not read file : " << argv [2] << "\n";
  std::cerr << "Usage: point_render <normals file>" << "\n";
  return 0;
}
