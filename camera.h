/**
 * Header for camera class.
 * Camera class controls all manipulation in 3D, including light sources.
 *
 * Author : Ricardo Marroquim
 * Date created : 27-01-07
 *
 **/

#ifndef __CAMERA__
#define __CAMERA__

#include "GL/gl.h"
#include "GL/glu.h"

#include "math.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include "quat.h"

// Rendering mode
enum renderMode {ORTHOGRAPHIC, PERSPECTIVE};

using namespace std;

// Key Frame type
typedef struct _keyframe {
	/// Constructor - void
	_keyframe(void) : rot() {
		pos[0] = pos[1] = pos[2] = 0.0;
	}
	/// Constructor
	_keyframe(double _p[], Quat _r) : rot(_r) {
		pos[0] = _p[0]; pos[1] = _p[1]; pos[2] = _p[2];
	}
	/// I/O operator - output
        inline friend ostream& operator << (ostream& out, const struct _keyframe& k) {
		out << k.rot.x << " " << k.rot.y << " " << k.rot.z << " " << k.rot.a << " "
		    << k.pos[0] << " " << k.pos[1] << " " << k.pos[2];
                return out;
        }
	/// I/O operator - input
        inline friend istream& operator >> (istream& in, struct _keyframe& k) {
		in >> k.rot.x >> k.rot.y >> k.rot.z >> k.rot.a
		   >> k.pos[0] >> k.pos[1] >> k.pos[2];
                return in;
        }
	/// Data
	Quat rot;
	double pos[3];
} keyframe;

/// Camera class
/// Controls manipulation in 3D using a trackball with quaternions.
/// Also keeps light sources and perspective/orthographic view informations.
/// Keeps mouse information.
class Camera {
public:
  
//   /// Default constructor
//   Camera() : screen_width (500), screen_height (500),
// 	     zoom_factor(1.0), fov(50), z_near(1.0) z_far(100.0){
//     view_mode = ORTHOGRAPHIC;
//     static double identity [16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
//     std::copy (identity, identity+16, rotation_matrix);
//   }

  /// Constructor with screen dimensions
  Camera(const int w, const int h) : screen_width (w), screen_height (h), 
				     zoom_factor(1.0), fov(1.0),
				     z_near(0.01), z_far(100.0) {
    view_mode = PERSPECTIVE;
    static double identity [16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    std::copy (identity, identity+16, rotation_matrix);

    q_last.a = 1; q_last.x = 0.0; q_last.y = 0.0; q_last.z = 0.0;
    q_rot.a = 1; q_rot.x = 0.0; q_rot.y = 0.0; q_rot.z = 0.0;

    eye[0] = 0.0; eye[1] = 0.0; eye[2] = 3.0;
    eye_rot[0] = 0.0; eye_rot[1] = 0.0; eye_rot[2] = 3.0;

    position[0] = 0.0; position[1] = 0.0; position[2] = 0.0;
    
    light_position[0] = 0.0; light_position[1] = 0.0; light_position[2] = 1.0; light_position[3] = 0.0;

    radius = 1.0;

    frameVideo = -1.0;
  }

  /// Destructor
  ~Camera();

  /// Initializes modelview and projection matrices for rendering
  void setView ( void );
  /// Resets the view mode properties
  void resetViewMode ( void );
  /// Switch to given view mode
  void switchViewMode ( int vm );
  /// Switch view mode
  void switchViewMode ( void );
  /// Initialize light properties
  void initLight (void);

  /// Return the current view mode
  /// @return current view mode (orthograpic or perspective)
  int viewMode ( void ) const { return view_mode; }

  /// Return screen width
  int screenWidth ( void ) const { return screen_width; }
  /// Return screen height
  int screenHeight ( void ) const { return screen_height; }

  /// Reshape function
  void reshape (int w, int h);

  void updateMouse ( void );

  /// Starts a quat rotation procedure
  void startQuatRotation(int x, int y, Quat*);
  /// Starts a rotation procedure
  void startRotation(int x, int y);
  /// Ends a rotation procedure
  void endRotation( void );
  /// Rotate world
  void rotate (int x, int y);

  /// Auto rotation
  void rotate (void);

  /// Compute new eye position given a rotation quaternion
  void computeEyePosition(Quat q, double *new_eye);

  /// Translate light position
  void lightTranslate (int x, int y);

  /// Starts zooming procedure
  void mouseSetClick (int x, int y);
  /// Zooming procedure
  void zooming (int x, int y);

  /// Translation procedure
  void translate (int x, int y);

  /// Translate a given vector
  void translateVec (int x, int y, double* vec);
  /// Translate a given vector in the z axis
  void zoomingVec (int x, int y, double* vec);
  /// Rotate a given quaternion
  void rotateQuat(int x, int y, Quat *q);

  /// Return rotation matrix
  const double* rotationMatrix ( void );// { return rotation_matrix; }

  /// Return rotation quat
  const Quat rotationQuat ( void ) { return q_rot; }

  /// Return zoom factor
  const double zoom ( void ) const { return zoom_factor; }

  /// Return zoom factor
  const double* positionVector ( void ) const { return &position[0]; }

  /// Return far and near planes
  const double zNear ( void ) const { return z_near; }
  const double zFar ( void ) const { return z_far; }
  const double fieldOfVision ( void ) const { return fov; }

  void lightVec ( double l[] ) const { l[0] = light_position[0]; l[1] = light_position[1]; l[2] = light_position[2]; }
  void eyeVec ( double e[] ) const { e[0] = eye[0]; e[1] = eye[1]; e[2] = eye[2]; };  
  void positionVec ( double e[] ) const { e[0] = position[0]; e[1] = position[1]; e[2] = -position[2]; };

  void createKeyFrame( void ) {
	  keyframe k( position, q_rot );
	  keyFrames.push_back( k );
  }

  void writeKeyFrames( const char* fn ) {
	  ofstream out(fn);
	  if (out.fail()) return;
	  if (keyFrames.size() == 0) return;
	  out << keyFrames.size() << endl;
	  for (uint i=0; i<keyFrames.size(); i++)
		  out << keyFrames[i] << endl;
  }

  void loadKeyFrames( const char* fn ) {
	  ifstream in(fn);
	  if (in.fail()) return;
	  keyFrames.clear();
	  uint numFrames;
	  in >> numFrames;
	  keyframe k;
	  for (uint i=0; i<numFrames; i++) {
		  in >> k;
		  keyFrames.push_back( k );
	  }
	  frameVideo = -1.0;
  }

  void runFrames( void ) { frameVideo = 0.0; }

  bool runningFrames( void ) {
	  return ( (frameVideo >= 0.0) && (keyFrames.size() > 0) );
  }

private:

  // Screen size
  int screen_width, screen_height;

  // Render mode (perspective or orthographic)
  int view_mode;

  // Rotation matrix, current and previous
  double rotation_matrix[16];
  double prev_matrix[16];

  // Trackball center
  double center[3];

  // Translation vector
  double position[3];

  // Trackball radius
  double radius;

  // Trackball zoom
  double zoom_factor;

  // Field of vision
  double fov;

  // Front and back z planes
  double z_near, z_far;

  // Fixed eye position
  double eye[3];

  // Rotated eye
  double eye_rot[3];

  // Quarternion rotation
  Quat q_last;
  // Quaternion accumulation during rotation
  Quat q_rot;

  // Light position
  GLfloat light_position[4];

  /*************** Mouse **************/
  // Mouse rotation
  double mouse_start[3], mouse_curr[3];
  // Mouse button
  int button_pressed;

  vector< keyframe > keyFrames;

  double frameVideo;

  double squaredDistance(const double [3], const double [3]) const;

  Quat getQuaternion(float x1, float y1, float x2, float y2);

  void mapToSphere(const double p_screen[3], double p[], const double r) const;

};

#endif
