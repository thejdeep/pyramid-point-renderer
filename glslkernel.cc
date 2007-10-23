/*
        glslkernel.cpp
        Base class for GLSL application


  Program:   GLSLKernel
  Module:    $RCSfile: glslkernel.cxx,v $
  Language:  C++
  Date:      $Date: 2006/01/30 19:46:09 $
  Version:   $Revision: 1.3 $

        Created: Jan 30 2006.
*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>

#include "GLee.h"
#include <GL/glu.h>

#include "glslkernel.h"

using namespace std;

static void check_attributes (GLuint prg) {
	GLsizei bufSize = 255, length;
	GLint size;
	GLenum type;
	GLchar name[bufSize];
	GLint num_attribs;

	glGetProgramiv(prg, GL_ACTIVE_ATTRIBUTES, &num_attribs);

	for (GLint i=0; i<num_attribs; ++i) {
		glGetActiveAttrib(prg, i, bufSize, &length, &size, &type, name);
		cout << "attrib[" << i << "]: name = " << name << " ; length = "
			<< length << " ; size = " << size << " type = "
			<< (type==GL_INT?"int":(type==GL_FLOAT?"float":
			(type==GL_FLOAT_VEC4?"vec4":(type==GL_FLOAT_VEC3?"vec3":
			(type==GL_FLOAT_VEC2?"vec2":"_"))))) << endl;
	}
	if (num_attribs) cout << endl;
}


/// Print out the information log for a shader object 
/// @arg obj handle for a shader object
static void printShaderInfoLog(GLuint obj)
{
	GLint infologLength = 0, charsWritten = 0;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 2) {
		GLchar* infoLog = new GLchar [infologLength];
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		cerr << infoLog << endl;
		delete infoLog;
	}
}

/// Print out the information log for a shader object 
/// @arg obj handle for a program object
static void printProgramInfoLog(GLuint obj)
{
	GLint infologLength = 0, charsWritten = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 2) {
		GLchar* infoLog = new GLchar [infologLength];
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		cerr << infoLog << endl;
		delete infoLog;
	}
}

///
/// Returns 1 if an OpenGL error occurred, 0 otherwise.
static int error_check () {
	GLenum glErr;
	int retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cerr << __FILE__ << " glError: " << __LINE__ << " " << gluErrorString(glErr) << endl;
		retCode = 1;
		glErr = glGetError();
	}
	//return retCode;
	return 0; // ...a little problem with nvidia lib 8756...
}

static std::vector <char> buffer; ///< Buffer for storing texts for shaders
static std::vector <char*> line; ///< Points to individual lines inside buffer

/// Utility method for reading a text file one line at a time into array
/// of strings 'line'. Vector 'buffer' provides storage for the text.
/// @arg filename name of text file
static void load_file (const char* filename) {
	ifstream f (filename);
	vector<int> n;
	assert (f);
	char buf [1000];
	buffer.clear ();
	line.clear();
	while (!f.eof()) {
		f.getline (buf,1000);
		n.push_back(strlen(buf));
		copy (buf, buf+n.back(), back_inserter (buffer));
		buffer.push_back ('\n');
		buffer.push_back ('\0');
	}
	int length = 0;
	for (unsigned int i = 0; i < n.size (); i++) {
		line.push_back (&buffer[length]);
		length += n[i]+2;
	}
}

/// Constructor
/// @arg frag_source array of strings containing fragment program
/// @arg vtx_source array of strings containing fragment program
GLSLKernel::GLSLKernel (const char ** frag_source, const char ** vtx_source) 
		: programObject (0), fragmentShader(0), vertexShader(0), 
		fragSource(frag_source), vtxSource(vtx_source), 
		fragFileName(0), vtxFileName(0) {
}

/// Sets the name of a fragment shader file
/// @arg filename name of fragment source file
void GLSLKernel::fragment_source (const char* filename) {
	fragFileName = filename;
}

/// Sets the name of a vertex shader file
/// @arg filename name of vertex source file
void GLSLKernel::vertex_source (const char* filename) {
	vtxFileName = filename;
}

/// Tells whether the system has OpenGL SL capabilities
/// @return true if the system is ready for OpenGL SL
bool GLSLKernel::has_GLSL() {
	return ((GL_VERTEX_SHADER) && (GL_FRAGMENT_SHADER));
}

/// Tells whether the shader program is ready to run
/// @return true if and only if a program object was built
bool GLSLKernel::installed () {
	return programObject != 0;
}

/// Installs the shaders as the current shaders for the current context
/// Compiles and links the shaders if this was not done earlier
/// @arg debug flags the debug information output
void GLSLKernel::install (bool debug) {
	if (!installed ()) {
		assert (vtxSource || fragSource || vtxFileName || fragFileName);
		programObject = glCreateProgram();
		if (fragSource || fragFileName) {
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

			assert(fragmentShader != 0);

			if (fragSource) {
				glShaderSource(fragmentShader, 1, fragSource, NULL);
			} else {
				load_file (fragFileName);
				glShaderSource(fragmentShader, line.size(), (const GLchar**) (&line[0]), NULL);
			}

			assert (!error_check());

			glCompileShader(fragmentShader);

			if (debug) printShaderInfoLog (fragmentShader);
			assert (!error_check());

			GLint compileFrag;
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileFrag);
			assert (compileFrag == GL_TRUE);

			glAttachShader(programObject, fragmentShader);
			assert (!error_check());
		}
		if (vtxSource || vtxFileName) {
			vertexShader = glCreateShader(GL_VERTEX_SHADER);

			assert(vertexShader != 0);

			if (vtxSource) {
				glShaderSource(vertexShader, 1, vtxSource, NULL);
			} else {
				load_file (vtxFileName);
				glShaderSource(vertexShader, line.size(), (const GLchar**) &line[0], NULL);
			}
			glCompileShader(vertexShader);

			if (debug) printShaderInfoLog (vertexShader);
			assert (!error_check());

			GLint compileVertex;
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileVertex);
			assert (compileVertex == GL_TRUE);

			glAttachShader(programObject, vertexShader);
			assert (!error_check());
		}
		// Link the shader into a complete GLSL program.
		glLinkProgram(programObject);
		if (debug) printProgramInfoLog (programObject);
		GLint progLinkSuccess;
		glGetProgramiv(programObject, GL_LINK_STATUS, &progLinkSuccess);
		assert (progLinkSuccess);
		if (debug) check_attributes (programObject);
	}
	assert (installed ());
}

/// Sets the current kernel as the one in use
/// @arg use_kernel if false, instructs opengl not to use any kernel 
void GLSLKernel::use (bool use_kernel) 
{
	glUseProgram(use_kernel?programObject:0);
	assert (!error_check());
}

/// Gets a uniform location by name
/// @arg name name of uniform variable
/// @return location handle of uniform variable
GLint GLSLKernel::get_uniform_location (const GLchar* name) {
	assert (installed());
	return glGetUniformLocation (programObject, name);
}

/// Gets a n-float uniform value by name
/// @arg name name of uniform variable.
/// @arg p pointer to GLfloat array to be filled. The number of floats copied
///        depends on the size of the uniform as specified in the shader prog.
void GLSLKernel::get_uniform (const GLchar* name, GLfloat *p) {
	assert (installed ());
	assert (programObject);
	GLint location = glGetUniformLocation (programObject, name);
	assert (location != -1);
	glGetUniformfv (programObject, location, p);
	assert (!error_check());
}

/// Sets a {1|2|3|4}-{integer|float} uniform value by {name|location}
/// @arg name name of uniform variable
/// @arg location location handle of uniform variable
/// @arg a first value
/// @arg b second value
/// @arg c third value
/// @arg d fourth value

/// location & integer
void GLSLKernel::set_uniform (GLint location, GLint a, GLint b, GLint c, GLint d) {
	assert (location != -1);
	glUniform4i (location, a, b, c, d);
	assert (!error_check());
}
void GLSLKernel::set_uniform (GLint location, GLint a, GLint b, GLint c) {
	assert (location != -1);
	glUniform3i (location, a, b, c);
	assert (!error_check());
}
void GLSLKernel::set_uniform (GLint location, GLint a, GLint b) {
	assert (location != -1);
	glUniform2i (location, a, b);
	assert (!error_check());
}
void GLSLKernel::set_uniform (GLint location, GLint a) {
	assert (location != -1);
	glUniform1i (location, a);
	assert (!error_check());
}
/// name & integer
void GLSLKernel::set_uniform (const GLchar* name, GLint a, GLint b, GLint c, GLint d) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a, b, c, d);
}
void GLSLKernel::set_uniform (const GLchar* name, GLint a, GLint b, GLint c) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a, b, c);
}
void GLSLKernel::set_uniform (const GLchar* name, GLint a, GLint b) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a, b);
}
void GLSLKernel::set_uniform (const GLchar* name, GLint a) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a);
}
/// location & float
void GLSLKernel::set_uniform (GLint location, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
	assert (location != -1);
	glUniform4f (location, a, b, c, d);
	assert (!error_check());
}
void GLSLKernel::set_uniform (GLint location, GLfloat a, GLfloat b, GLfloat c) {
	assert (location != -1);
	glUniform3f (location, a, b, c);
	assert (!error_check());
}
void GLSLKernel::set_uniform (GLint location, GLfloat a, GLfloat b) {
	assert (location != -1);
	glUniform2f (location, a, b);
	assert (!error_check());
}
void GLSLKernel::set_uniform (GLint location, GLfloat a) {
	assert (location != -1);
	glUniform1f (location, a);
	assert (!error_check());
}
/// name & float
void GLSLKernel::set_uniform (const GLchar* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a, b, c, d);
}
void GLSLKernel::set_uniform (const GLchar* name, GLfloat a, GLfloat b, GLfloat c) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a, b, c);
}
void GLSLKernel::set_uniform (const GLchar* name, GLfloat a, GLfloat b) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a, b);
}
void GLSLKernel::set_uniform (const GLchar* name, GLfloat a) {
	GLint location = get_uniform_location (name);
	set_uniform (location, a);
}

/// Sets a {integer|float} uniform vector value by {name|location}
/// @arg name name of uniform variable
/// @arg location location handle of uniform variable
/// @arg v vector value
/// @arg nvalues number of values in the vector v
/// @arg count number of elements of the uniform vector array to be modified
///            a count of 1 should be used if modifying the value of a single vector

/// location and integer
void GLSLKernel::set_uniform (GLint location, const GLint* v, GLuint nvalues, GLsizei count) {
	assert (location != -1);
	assert ((nvalues > 0) && (nvalues < 5));
	switch (nvalues) {
		case 1: glUniform1iv (location, count, v);
		case 2: glUniform2iv (location, count, v);
		case 3: glUniform3iv (location, count, v);
		case 4: glUniform4iv (location, count, v);
	}
	assert (!error_check());
}
/// location and float
void GLSLKernel::set_uniform (GLint location, const GLfloat* v, GLuint nvalues, GLsizei count) {
	assert (location != -1);
	assert ((nvalues > 0) && (nvalues < 5));
	switch (nvalues) {
		case 1: glUniform1fv (location, count, v);
		case 2: glUniform2fv (location, count, v);
		case 3: glUniform3fv (location, count, v);
		case 4: glUniform4fv (location, count, v);
	}
	assert (!error_check());
}
/// name and integer
void GLSLKernel::set_uniform (const GLchar* name, const GLint* v, GLuint nvalues, GLsizei count) {
	GLint location = get_uniform_location (name);
	set_uniform (location, v, nvalues, count);
}
/// name and float
void GLSLKernel::set_uniform (const GLchar* name, const GLfloat* v, GLuint nvalues, GLsizei count) {
	GLint location = get_uniform_location (name);
	set_uniform (location, v, nvalues, count);
}

/// Sets a uniform matrix value by {name|location}
/// @arg name name of uniform variable
/// @arg location location handle of uniform variable
/// @arg m matrix value with dim^2 values
/// @arg dim dimension of the matrix m
/// @arg transpose if transpose is GL_FALSE, each matrix is assumed to be supplied in
///                column major order, otherwise is in row major order.
/// @arg count number of elements of the uniform matrix array to be modified
///            a count of 1 should be used if modifying the value of a single matrix

/// location
void GLSLKernel::set_uniform_matrix (GLint location, const GLfloat* m, GLuint dim,
		GLboolean transpose, GLsizei count) {
	assert (location != -1);
	assert ((dim > 1) && (dim < 5));
	switch (dim) {
		case 2: glUniformMatrix2fv (location, count, transpose, m);
		case 3: glUniformMatrix3fv (location, count, transpose, m);
		case 4: glUniformMatrix4fv (location, count, transpose, m);
	}
	assert (!error_check());
}
/// name
void GLSLKernel::set_uniform_matrix (const GLchar* name, const GLfloat* m, GLuint dim,
		GLboolean transpose, GLsizei count) {
	GLint location = get_uniform_location (name);
	set_uniform_matrix (location, m, dim, transpose, count);
}

/// Gets an attribute index by name
/// @arg name name of attribute variable
/// @return location handle of attribute variable
GLint GLSLKernel::get_attribute_index (const GLchar* name) {
	assert (installed());
	assert (programObject);
	return glGetAttribLocation (programObject, name);
}

/// Sets a {1|2|3|4}-{short|float|double} attribute value by {name|index}
/// @arg name name of attribute variable
/// @arg index index handle of attribute variable
/// @arg a first value
/// @arg b second value
/// @arg c third value
/// @arg d fourth value

/// index & short
void GLSLKernel::set_attribute (GLint index, GLshort a, GLshort b, GLshort c, GLshort d) {
	assert (index != -1);
	glVertexAttrib4s (index, a, b, c, d);
	assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLshort a, GLshort b, GLshort c) {
	assert (index != -1);
	glVertexAttrib3s (index, a, b, c);
	assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLshort a, GLshort b) {
	assert (index != -1);
	glVertexAttrib2s (index, a, b);
	assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLshort a) {
	assert (index != -1);
	glVertexAttrib1s (index, a);
	assert (!error_check());
}
/// name & short
void GLSLKernel::set_attribute (const GLchar* name, GLshort a, GLshort b, GLshort c, GLshort d) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b, c, d);
}
void GLSLKernel::set_attribute (const GLchar* name, GLshort a, GLshort b, GLshort c) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b, c);
}
void GLSLKernel::set_attribute (const GLchar* name, GLshort a, GLshort b) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b);
}
void GLSLKernel::set_attribute (const GLchar* name, GLshort a) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a);
}
/// index & float
void GLSLKernel::set_attribute (GLint index, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
	assert (index != -1);
	glVertexAttrib4f (index, a, b, c, d);
	//assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLfloat a, GLfloat b, GLfloat c) {
	assert (index != -1);
	glVertexAttrib3f (index, a, b, c);
	//assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLfloat a, GLfloat b) {
	assert (index != -1);
	glVertexAttrib2f (index, a, b);
	//assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLfloat a) {
	assert (index != -1);
	glVertexAttrib1f (index, a);
	//assert (!error_check());
}
/// name & float
void GLSLKernel::set_attribute (const GLchar* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b, c, d);
}
void GLSLKernel::set_attribute (const GLchar* name, GLfloat a, GLfloat b, GLfloat c) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b, c);
}
void GLSLKernel::set_attribute (const GLchar* name, GLfloat a, GLfloat b) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b);
}
void GLSLKernel::set_attribute (const GLchar* name, GLfloat a) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a);
}
/// index & double
void GLSLKernel::set_attribute (GLint index, GLdouble a, GLdouble b, GLdouble c, GLdouble d) {
	assert (index != -1);
	glVertexAttrib4d (index, a, b, c, d);
	assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLdouble a, GLdouble b, GLdouble c) {
	assert (index != -1);
	glVertexAttrib3d (index, a, b, c);
	assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLdouble a, GLdouble b) {
	assert (index != -1);
	glVertexAttrib2d (index, a, b);
	assert (!error_check());
}
void GLSLKernel::set_attribute (GLint index, GLdouble a) {
	assert (index != -1);
	glVertexAttrib1d (index, a);
	assert (!error_check());
}
/// name & double
void GLSLKernel::set_attribute (const GLchar* name, GLdouble a, GLdouble b, GLdouble c, GLdouble d) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b, c, d);
}
void GLSLKernel::set_attribute (const GLchar* name, GLdouble a, GLdouble b, GLdouble c) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b, c);
}
void GLSLKernel::set_attribute (const GLchar* name, GLdouble a, GLdouble b) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a, b);
}
void GLSLKernel::set_attribute (const GLchar* name, GLdouble a) {
	GLint index = get_attribute_index (name);
	set_attribute (index, a);
}

/// Associates an attribute with a specefic index.
/// @arg name name of attribute variable
/// @arg location index of attribute variable
void GLSLKernel::bind_attribute_location (const GLchar* name, GLint index)
{
  glBindAttribLocation(programObject, index, name);
}

