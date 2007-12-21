/*
        glslkernel.h
        Header for the GLSLKernel class


  Program:   GLSLKernel
  Module:    $RCSfile: glslkernel.h,v $
  Language:  C++
  Date:      $Date: 2006/01/30 19:46:09 $
  Version:   $Revision: 1.3 $

        Created: Jan 30 2006.
*/

#include <GL/glu.h>

/// A simple GLSL Kernel containing at most one vertex shader and one 
/// fragment shader
class GLSLKernel {
	GLuint programObject;    ///< The GLSL program
	GLuint fragmentShader;   ///< Fragment Shader
	GLuint vertexShader;     ///< Vertex Shader
	const char** fragSource; ///< Fragment program source
	const char** vtxSource;  ///< Vertex program source
	const char* fragFileName; ///< Fragment program source filename
	const char* vtxFileName;  ///< Vertex program source filename

public:
	/// Constructor
	/// @arg frag_source array of strings containing fragment program
	/// @arg vtx_source array of strings containing fragment program
	GLSLKernel (const char ** frag_source = 0, const char ** vtx_source = 0);

	/// Sets the name of a fragment shader file
	/// @arg filename name of fragment source file
	void fragment_source (const char* filename);

	/// Sets the name of a vertex shader file
	/// @arg filename name of vertex source file
	void vertex_source (const char* filename);

	/// Tells whether the system has OpenGL SL capabilities
	/// @return true if the system is ready for OpenGL SL
	bool has_GLSL ();

	/// Tells whether the shader program is ready to run
	/// @return true if and only if a program object was built
	bool installed ();

	/// Installs the shaders as the current shaders for the current context
	/// Compiles and links the shaders if this was not done earlier
	/// @arg debug flags the debug information output
	void install (bool debug = false);

	/// Sets the current kernel as the one in use
	/// @arg use_kernel if false, instructs opengl not to use any kernel 
	void use (bool use_kernel = true);
	
	/// Gets an uniform location by name
	/// @arg name name of uniform variable
	/// @return location handle of uniform variable
	GLint get_uniform_location (const GLchar* name);

	/// Gets a n-float uniform value by name
	/// @arg name name of uniform variable.
	/// @arg p pointer to GLfloat array to be filled. The number of floats copied
	///        depends on the size of the uniform as specified in the shader prog.
	void get_uniform (const GLchar* name, GLfloat *p);

	/// Sets a {1|2|3|4}-{integer|float} uniform value by {name|location}
	/// @arg name name of uniform variable
	/// @arg location location handle of uniform variable
	/// @arg a first value
	/// @arg b second value
	/// @arg c third value
	/// @arg d fourth value
	void set_uniform (GLint location, GLint a, GLint b, GLint c, GLint d);
	void set_uniform (GLint location, GLint a, GLint b, GLint c);
	void set_uniform (GLint location, GLint a, GLint b);
	void set_uniform (GLint location, GLint a);
	void set_uniform (const GLchar* name, GLint a, GLint b, GLint c, GLint d);
	void set_uniform (const GLchar* name, GLint a, GLint b, GLint c);
	void set_uniform (const GLchar* name, GLint a, GLint b);
	void set_uniform (const GLchar* name, GLint a);
	void set_uniform (GLint location, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
	void set_uniform (GLint location, GLfloat a, GLfloat b, GLfloat c);
	void set_uniform (GLint location, GLfloat a, GLfloat b);
	void set_uniform (GLint location, GLfloat a);
	void set_uniform (const GLchar* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
	void set_uniform (const GLchar* name, GLfloat a, GLfloat b, GLfloat c);
	void set_uniform (const GLchar* name, GLfloat a, GLfloat b);
	void set_uniform (const GLchar* name, GLfloat a);

	/// Sets a {integer|float} uniform vector value by {name|location}
	/// @arg name name of uniform variable
	/// @arg location location handle of uniform variable
	/// @arg v vector value
	/// @arg nvalues number of values in the vector v
	/// @arg count number of elements of the uniform vector array to be modified
	///            a count of 1 should be used if modifying the value of a single vector
	void set_uniform (const GLchar* name, const GLint* v, GLuint nvalues, GLsizei count=1);
	void set_uniform (const GLchar* name, const GLfloat* v, GLuint nvalues, GLsizei count=1);
	void set_uniform (GLint location, const GLint* v, GLuint nvalues, GLsizei count=1);
	void set_uniform (GLint location, const GLfloat* v, GLuint nvalues, GLsizei count=1);

	/// Sets a uniform matrix value by {name|location}
	/// @arg name name of uniform variable
	/// @arg location location handle of uniform variable
	/// @arg m matrix value with dim^2 values
	/// @arg dim dimension of the matrix m
	/// @arg transpose if transpose is GL_FALSE, each matrix is assumed to be supplied in
	///                column major order, otherwise is in row major order.
	/// @arg count number of elements of the uniform matrix array to be modified
	///            a count of 1 should be used if modifying the value of a single matrix
	void set_uniform_matrix (const GLchar* name, const GLfloat* m, GLuint dim,
			GLboolean transpose, GLsizei count=1);
	void set_uniform_matrix (GLint location, const GLfloat* m, GLuint dim,
			GLboolean transpose, GLsizei count=1);

	/// Gets an attribute location by name
	/// @arg name name of attribute variable
	/// @return location handle of attribute variable
	GLint get_attribute_index (const GLchar* name);

	/// Sets a {1|2|3|4}-{short|float|double} attribute value by {name|index}
	/// @arg name name of attribute variable
	/// @arg index index handle of attribute variable
	/// @arg a first value
	/// @arg b second value
	/// @arg c third value
	/// @arg d fourth value
	void set_attribute (GLint index, GLshort a, GLshort b, GLshort c, GLshort d);
	void set_attribute (GLint index, GLshort a, GLshort b, GLshort c);
	void set_attribute (GLint index, GLshort a, GLshort b);
	void set_attribute (GLint index, GLshort a);
	void set_attribute (const GLchar* name, GLshort a, GLshort b, GLshort c, GLshort d);
	void set_attribute (const GLchar* name, GLshort a, GLshort b, GLshort c);
	void set_attribute (const GLchar* name, GLshort a, GLshort b);
	void set_attribute (const GLchar* name, GLshort a);
	void set_attribute (GLint index, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
	void set_attribute (GLint index, GLfloat a, GLfloat b, GLfloat c);
	void set_attribute (GLint index, GLfloat a, GLfloat b);
	void set_attribute (GLint index, GLfloat a);
	void set_attribute (const GLchar* name, GLfloat a, GLfloat b, GLfloat c, GLfloat d);
	void set_attribute (const GLchar* name, GLfloat a, GLfloat b, GLfloat c);
	void set_attribute (const GLchar* name, GLfloat a, GLfloat b);
	void set_attribute (const GLchar* name, GLfloat a);
	void set_attribute (GLint index, GLdouble a, GLdouble b, GLdouble c, GLdouble d);
	void set_attribute (GLint index, GLdouble a, GLdouble b, GLdouble c);
	void set_attribute (GLint index, GLdouble a, GLdouble b);
	void set_attribute (GLint index, GLdouble a);
	void set_attribute (const GLchar* name, GLdouble a, GLdouble b, GLdouble c, GLdouble d);
	void set_attribute (const GLchar* name, GLdouble a, GLdouble b, GLdouble c);
	void set_attribute (const GLchar* name, GLdouble a, GLdouble b);
	void set_attribute (const GLchar* name, GLdouble a);

	/// Associates an attribute with a specefic index.
  /// @arg name name of attribute variable
	/// @arg location index of attribute variable
	void bind_attribute_location (const GLchar* name, GLint index);
};
