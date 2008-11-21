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
#include "surfels.h"

// Rendering mode
enum renderMode {ORTHOGRAPHIC, PERSPECTIVE};

using namespace std;

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

  void setTranslation (void);
  void setRotation (void);


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
  void computeEyePosition(Quat q, Point *new_eye);

  /// Translate light position
  void lightTranslate (int x, int y);

  /// Starts zooming procedure
  void mouseSetClick (int x, int y);
  /// Zooming procedure
  void zooming (int x, int y);

  /// Translation procedure
  void translate (int x, int y);

  /// Translate a given vector
  void translateVec (int x, int y, Point* );
  /// Translate a given vector in the z axis
  void zoomingVec (int x, int y, Point* );
  /// Rotate a given quaternion
  void rotateQuat(int x, int y, Quat *q, Point* );

  /// Return rotation matrix
  const double* rotationMatrix ( void );// { return rotation_matrix; }

  /// Return rotation quat
  const Quat rotationQuat ( void ) { return q_rot; }
  void setRotationQuat ( Quat q ) { q_rot = q; }

  /// Return zoom factor
  const double zoom ( void ) const { return zoom_factor; }
  /// Return position
  const Point positionVector ( void ) const { return position; }
  void setPositionVector ( Point p ) { position = p; }
  
  /// Return light position
  const Point lightVector ( void ) const { return light_position; }
  void setLightVector ( GLfloat p[3] ) { 
    light_position[0] = p[0]; light_position[1] = p[1]; light_position[2] = p[2];}

  const Vector ambientLight ( void ) const { return ambient_light; }
  const Vector diffuseLight ( void ) const { return diffuse_light; }
  const Vector specularLight ( void ) const { return specular_light; }
  
  /// Return far and near planes
  const double zNear ( void ) const { return z_near; }
  const double zFar ( void ) const { return z_far; }
  const double fieldOfVision ( void ) const { return fov; }

  //void lightVec ( double l[] ) const { l[0] = light_position[0]; l[1] = light_position[1]; l[2] = light_position[2]; }
  void positionVec ( double e[] ) const { e[0] = position.x; e[1] = position.y; e[2] = -position.z; }

  const double getRadius ( void ) const { return radius; }

private:

  // Screen size
  int screen_width, screen_height;

  // Render mode (perspective or orthographic)
  int view_mode;

  // Rotation matrix, current and previous
  double rotation_matrix[16];
  double prev_matrix[16];

  // Translation vector
  Point position;

  // Normal vector = side translation direction
  Vector normal;

  // View vector - target - position
  Vector view;

  // Up vector
  Vector up;

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
  Quat q_lookAt;
  Quat q_last_lookAt;

  // Light position
  Point light_position;

  Vector ambient_light;
  Vector diffuse_light;
  Vector specular_light;


  /*************** Mouse **************/
  // Mouse rotation
  Point mouse_start, mouse_curr;
  // Mouse button
  int button_pressed;

  double angle_h, angle_v;
  Point target;

  double squaredDistance(const double [3], const double [3]) const;

  Quat getQuaternion(float x1, float y1, float x2, float y2);

  void mapToSphere(Point &, const double r) const;

  void projectToScreen(Point* p, Point &screen_pos);

  void newTarget( const Point* p );

  void normalizeCoordinates(Point& p);

  Point projectToWorld(const Point& p);

};

#endif
