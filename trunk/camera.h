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

#include <cmath>
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
    light_pos[0] = light_pos[1] = light_pos[2] = light_pos[3] = 0.0;
    reconstruction_filter = prefilter = 0.0;
  }
  /// Constructor
  _keyframe(double _p[], Quat _r) : rot(_r) {
    pos[0] = _p[0]; pos[1] = _p[1]; pos[2] = _p[2];
    light_pos[0] = light_pos[1] = light_pos[2] = light_pos[3] = 0.0;
    reconstruction_filter = prefilter = 0.0;
  }
  /// Constructor
  _keyframe(double _p[], Quat _r, GLfloat _lp[], double& _rf, double& _pf) : rot(_r) {
    pos[0] = _p[0]; pos[1] = _p[1]; pos[2] = _p[2];
    light_pos[0] = _lp[0]; light_pos[1] = _lp[1]; light_pos[2] = _lp[2]; light_pos[3] = _lp[3];
    reconstruction_filter = _rf;
    prefilter = _pf;
  }
  /// I/O operator - output
  inline friend ostream& operator << (ostream& out, const struct _keyframe& k) {
    out << k.rot.x << " " << k.rot.y << " " << k.rot.z << " " << k.rot.a << " "
	<< k.pos[0] << " " << k.pos[1] << " " << k.pos[2] << " " << k.light_pos[0]
	<< " " << k.light_pos[1] << " " << k.light_pos[2] << " " << k.light_pos[3] 
	<< " " << k.reconstruction_filter << " " << k.prefilter;
    return out;
  }
  /// I/O operator - input
  inline friend istream& operator >> (istream& in, struct _keyframe& k) {
    in >> k.rot.x >> k.rot.y >> k.rot.z >> k.rot.a
       >> k.pos[0] >> k.pos[1] >> k.pos[2] >> k.light_pos[0] >>  k.light_pos[1] >>
      k.light_pos[2] >> k.light_pos[3] >> k.reconstruction_filter >> k.prefilter;
    return in;
  }

  /// Data
  Quat rot;
  double pos[3];
  GLfloat light_pos[4];
  double reconstruction_filter;
  double prefilter;

} keyframe;

/// Camera class
/// Controls manipulation in 3D using a trackball with quaternions.
/// Also keeps light sources and perspective/orthographic view informations.
/// Keeps mouse information.
class Camera {
public:
 
  /// Constructor with screen dimensions
  Camera(const int w, const int h);

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
  void startQuatRotation(int x, int y, Quat*, double[3]);
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
  void rotateQuat(int x, int y, Quat *q, double[3]);

  /// Return rotation matrix
  const double* rotationMatrix ( void );// { return rotation_matrix; }

  /// Return rotation quat
  const Quat rotationQuat ( void ) { return q_rot; }
  void setRotationQuat ( Quat q ) { q_rot = q; }

  /// Return zoom factor
  const double zoom ( void ) const { return zoom_factor; }

  /// Return position
  const double* positionVector ( void ) const { return &position[0]; }
  void setPositionVector ( double p[3] ) { 
    position[0] = p[0]; position[1] = p[1]; position[2] = p[2]; }
  
  /// Return light position
  const GLfloat* lightVector ( void ) const { return &light_position[0]; }
  void setLightVector ( GLfloat p[3] ) { 
    light_position[0] = p[0]; light_position[1] = p[1]; light_position[2] = p[2]; }
  

  /// Return far and near planes
  const double zNear ( void ) const { return z_near; }
  const double zFar ( void ) const { return z_far; }
  const double fieldOfVision ( void ) const { return fov; }

  void lightVec ( double l[] ) const { l[0] = light_position[0]; l[1] = light_position[1]; l[2] = light_position[2]; }
  void positionVec ( double e[] ) const { e[0] = position[0]; e[1] = position[1]; e[2] = -position[2]; };

  void createKeyFrame( double reconstruction_filter, double prefilter ) {
    keyframe k( position, q_rot, light_position, reconstruction_filter, prefilter );
    keyFrames.push_back( k );
    cout << "Key frame recorded." << endl;
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
    frame_video = -1.0;
  }

  void runFrames( void ) { frame_video = 0.0; }

  void clearFrames( void ) { 
    frame_video = -1.0; 
    keyFrames.clear();
  }

  bool runningFrames( void ) {
    return ( (frame_video >= 0.0) && (keyFrames.size() > 0) );
  }

  double getKeyFrameReconstructionFilter ( void ) {
    return frame_reconstruction_filter;
    //return keyFrames[(unsigned int)frame_video].reconstruction_filter;
  }

  double getKeyFramePrefilter ( void ) {
    return frame_prefilter;
    //return keyFrames[(unsigned int)frame_video].prefilter;
  }

  void switchKeyFrameInterpolationMode ( void ) {
    keyFrameInterpolation = !keyFrameInterpolation;      
  }

private:

  void computeKeyFrame ( void );

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

  // Array with key frames
  vector< keyframe > keyFrames;

  double frame_video;

  bool keyFrameInterpolation;

  /*********************************************************/
  // Specific application variables to record on a key frame
  double frame_reconstruction_filter;
  double frame_prefilter;
  /*********************************************************/

  double squaredDistance(const double [3], const double [3]) const;

  Quat getQuaternion(float x1, float y1, float x2, float y2);

  void mapToSphere(const double p_screen[3], double p[], const double r) const;

  void projectToScreen(double p[3], double* screen_pos);
};

#endif
