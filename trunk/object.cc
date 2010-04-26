#include "object.h"
#include "point_based_renderer.h"


Object::~Object() {
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &color_buffer);
	glDeleteBuffers(1, &normal_buffer);
	glDeleteBuffers(1, &u_buffer);
	glDeleteBuffers(1, &v_buffer);
}

/**
 * Render object using designed rendering system.
 **/
void Object::render ( void ) const {

  if (renderer_type == PYRAMID_POINTS) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL);   

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);   
  }
  else if (renderer_type == PYRAMID_ELIPSES) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, u_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL);

    //glEnableClientState(GL_COLOR_ARRAY);
    //glBindBuffer(GL_ARRAY_BUFFER, v_buffer);
    //glColorPointer(4, GL_FLOAT, 0, NULL);

	//glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //glEnable(GL_TEXTURE_2D);
    //glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer);
    glTexCoordPointer(4, GL_FLOAT, 0, NULL);

    glDrawArrays(GL_POINTS, 0, number_points);
  }
  else if ((renderer_type == PYRAMID_POINTS_COLOR) ||
      (renderer_type == PYRAMID_TEMPLATES)) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);   
  }
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  check_for_ogl_error("Primitives render");

}

/**
 * Changes the renderer type.
 * @param rtype Given renderer type.
 **/
void Object::setRendererType ( int rtype ) {

  renderer_type = rtype;

//   glDisableClientState(GL_VERTEX_ARRAY);
//   glDisableClientState(GL_COLOR_ARRAY);
//   glDisableClientState(GL_NORMAL_ARRAY);

//   glDeleteBuffers(1, &vertex_buffer);
//   glDeleteBuffers(1, &color_buffer);
//   glDeleteBuffers(1, &normal_buffer);

  if (renderer_type == PYRAMID_POINTS)
    setPyramidPointsArrays();
  else if (renderer_type == PYRAMID_ELIPSES)
    setPyramidElipsesArrays();
  else if (renderer_type == PYRAMID_POINTS_COLOR)
    setPyramidPointsArraysColor();
  else if (renderer_type == PYRAMID_TEMPLATES)
    setPyramidPointsArraysColor();
}

/**
 * Create arrays and VBO for elliptical primitives.
 **/
void Object::setPyramidElipsesArrays ( void ) {

	/// Create the coordinates array, and the two principal axes arrays
	GLfloat *vertex_array, *u_array, *v_array;
	number_points = surfels.size();
	vertex_array = new GLfloat[number_points * 4];
	u_array = new GLfloat[number_points * 3];
	v_array = new GLfloat[number_points * 4];

	int pos = 0;
	for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

		vertex_array[pos*4 + 0] = (GLfloat)(it->Center()[0]);
		vertex_array[pos*4 + 1] = (GLfloat)(it->Center()[1]);
		vertex_array[pos*4 + 2] = (GLfloat)(it->Center()[2]);

		//minor axis
		u_array[pos*3 + 0] = (GLfloat)(it->MinorAxis().second[0]);
		u_array[pos*3 + 1] = (GLfloat)(it->MinorAxis().second[1]);
		u_array[pos*3 + 2] = (GLfloat)(it->MinorAxis().second[2]);
		vertex_array[pos*4 + 3] = (GLfloat)(it->MinorAxis().first);

		//major axis
		v_array[pos*4 + 0] = (GLfloat)(it->MajorAxis().second[0]);
		v_array[pos*4 + 1] = (GLfloat)(it->MajorAxis().second[1]);
		v_array[pos*4 + 2] = (GLfloat)(it->MajorAxis().second[2]);
		v_array[pos*4 + 3] = (GLfloat)(it->MajorAxis().first);

		++pos;
	}

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(float), (const void*)vertex_array, GL_STATIC_DRAW);
	delete [] vertex_array;

	glGenBuffers(1, &u_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, u_buffer);
	glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(float), (const void*)u_array, GL_STATIC_DRAW);
	delete [] u_array;

	glGenBuffers(1, &v_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, v_buffer);
	glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(float), (const void*)v_array, GL_STATIC_DRAW);
	delete [] v_array;
}

/**
 * Create arrays and VBO.
 **/
void Object::setPyramidPointsArrays ( void ) {

  GLfloat *vertex_array, *normal_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->Center()[0]);
    vertex_array[pos*4 + 1] = (GLfloat)(it->Center()[1]);
    vertex_array[pos*4 + 2] = (GLfloat)(it->Center()[2]);
    vertex_array[pos*4 + 3] = (GLfloat)(it->Radius());

    normal_array[pos*3 + 0] = (GLfloat)(it->Normal()[0]);
    normal_array[pos*3 + 1] = (GLfloat)(it->Normal()[1]);
    normal_array[pos*3 + 2] = (GLfloat)(it->Normal()[2]);

    ++pos;
  }
  
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(float), (const void*)vertex_array, GL_STATIC_DRAW);
  delete [] vertex_array;

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(float), (const void*)normal_array, GL_STATIC_DRAW);
  delete [] normal_array;
}

/**
 * Create arrays and VBO for pyramid point renderer color class.
 **/
void Object::setPyramidPointsArraysColor ( void ) {

  GLfloat *vertex_array, *normal_array;
  GLubyte *color_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLubyte[number_points * 4];

  double mean = 0.0;
  double standard_deviation = 0.0;
  double max_quality = 0.0;
  double min_quality = 10000.0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
	double q = (double)it->Quality();
	if (q < 10000)
	  mean += (double)it->Quality();
  }
  mean /= double(surfels.size());

  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
	double q = (double)it->Quality();
	if (q < 10000)
	  standard_deviation += ((double)it->Quality() - mean) * ((double)it->Quality() - mean);
  }
  standard_deviation = sqrt ( (1.0/double(surfels.size())) * standard_deviation );

  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

	double q = (double)it->Quality();
	//q = (q - mean) / (standard_deviation / two_pi);
	if (q < 10000) {
	  if (q > max_quality)
		max_quality = q;
	  if (q < min_quality)
		min_quality = q;
	}
  }

//   cout << "Mean :  " << mean << endl;
//   cout << "Standard Deviation :  " << standard_deviation << endl;
//   cout << "max - min : " << max_quality << " : " << min_quality << endl;

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->Center()[0]);
    vertex_array[pos*4 + 1] = (GLfloat)(it->Center()[1]);
    vertex_array[pos*4 + 2] = (GLfloat)(it->Center()[2]);
    vertex_array[pos*4 + 3] = (GLfloat)(it->Radius());

    normal_array[pos*3 + 0] = (GLfloat)(it->Normal()[0]);
    normal_array[pos*3 + 1] = (GLfloat)(it->Normal()[1]);
    normal_array[pos*3 + 2] = (GLfloat)(it->Normal()[2]);

	color_array[pos*4 + 0] = (GLubyte)(it->Color()[0]);
	color_array[pos*4 + 1] = (GLubyte)(it->Color()[1]);
	color_array[pos*4 + 2] = (GLubyte)(it->Color()[2]);

	//double q = it->color()[3] / max_quality;

	double q = (double)it->Quality();

	q = (q - min_quality) / (max_quality - min_quality);
	q = min (1.0, q);
	q = max (0.0, q);

	color_array[pos*4 + 3] = (GLubyte)(q * 255.0);
	
    ++pos;
  }

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(GLfloat), (const void*)vertex_array, GL_STATIC_DRAW);

  glGenBuffers(1, &color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(GLubyte), (const void*)color_array, GL_STATIC_DRAW);

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(GLfloat), (const void*)normal_array, GL_STATIC_DRAW);

  delete [] vertex_array;
  delete [] color_array;
  delete [] normal_array;

  check_for_ogl_error("Array Buffers Creation");
}

void Object::clearSurfels ( void ) {  
  surfels.clear();
}
