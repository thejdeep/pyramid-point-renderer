/**
 * Camera class controls all manipulation in 3D, including light sources.
 * Keeps mouse information.
 *
 * Author : Ricardo Marroquim
 * Date created : 27-01-07
 *
 **/

#include "camera.h"

#define PI 3.14159265

// Conversion from radians to degrees
const double rad_to_deg = 180.0/PI;

/// Initialize light properties
void Camera::initLight (void) {
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  GLfloat ambientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };

  light_position[3] = 0.0;

  glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  
  GLfloat model_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);

  glShadeModel(GL_SMOOTH);
}

/// Sets OpenGL camera
void Camera::setView (void) {
  glViewport(0, 0, screen_width, screen_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  resetViewMode();
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //  GLfloat lposition[] = {light_position[0], light_position[1], light_position[2], light_position[3]};  
  //GLfloat lposition[] = {light_position[0], light_position[1], light_position[2], 1.0};  
  //glLightfv(GL_LIGHT0, GL_POSITION, lposition);
  initLight();

  //glScalef(zoom_factor, zoom_factor, zoom_factor);

  glTranslatef(position[0], position[1], position[2]);
  glTranslatef(-eye[0], -eye[1], -eye[2]);

//   Quat q = q_rot;
//   q.x *= -1;
//   q.y *= -1;
//   q.z *= -1;
//   eye_rot[0] = eye[0];
//   eye_rot[1] = eye[1];
//   eye_rot[2] = eye[2];
//   q.rotate(eye_rot);

    // Convert from quaternion to angle+axis
  double s = 1.0 / sqrt(1 - q_rot.a*q_rot.a);
  double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
  if (q_rot.a == 1) {
    rot[1] = rot[2] = 0.0; rot[3] = 0.0;
  }

  glRotatef(rot[0], rot[1], rot[2], rot[3]);
  //glTranslatef(-eye_rot[0], -eye_rot[1], -eye_rot[2]);
}

/// Resets the view mode properties
void Camera::resetViewMode ( void ) {

  double w = (double)screen_width;
  double h = (double)screen_height;

  double diag = sqrt (w*w + h*h);
  double top = h / diag * 0.5 * fov * z_near;
  double bottom = - top;
  double right = w / diag * 0.5 * fov * z_near;
  double left = -right;

  left *= zoom_factor;
  right *= zoom_factor;
  top *= zoom_factor;
  bottom *= zoom_factor;

  double x = 1.0 * zoom_factor;
  double y = 1.0 * zoom_factor;

  if (view_mode == PERSPECTIVE)
    glFrustum(left, right, bottom, top, z_near, z_far);
  //gluPerspective( 45, (w/h), z_near, z_far );
  else
    glOrtho( -x, x, -y, y, z_near, z_far );

}

/// Switch between orthograpic and perspective modes
void Camera::switchViewMode ( void ) {

  if (view_mode == PERSPECTIVE)
    view_mode = ORTHOGRAPHIC;
  else
    view_mode = PERSPECTIVE;
  setView();
  //  reshape(screen_width, screen_height);
}

/// Switch to given mode
/// @param vm Given view mode
void Camera::switchViewMode ( int vm ) {
  view_mode = vm;
  reshape(screen_width, screen_height);
}

/// Reshape func
/// @param w New window width
/// @param h New window height
void Camera::reshape(int w, int h) {

  screen_width = w;
  screen_height = h;

  // Center of arcball is located at normalized window center
  center[0] = 0.0;
  center[1] = 0.0;
  center[2] = 0.0;

  // Arcball radius
  radius = 1.0;

  glClearColor (0.0, 0.0, 0.0, 1.0);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, (GLsizei) w, (GLsizei) h);

  resetViewMode ();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/// Starts a rotation procedure
/// @param x Mouse screen x coordinate
/// @param y Mouse screen y coordinate
void Camera::startRotation(int x, int y) {

  // Save initial click
  mouse_start[0] = x;
  mouse_start[1] = screen_height - y;
  mouse_start[2] = 0.0;

}

/// Ends a rotation procedure
void Camera::endRotation() {
  q_last = q_rot;
}

/// Maps a clicked point on the screen to the arcball sphere
/// @param p Given point
/// @param Mapped point on sphere
/// @param Sphere radius
void Camera::mapToSphere(const double p_screen[3], double p[], const double r) const {

  p[0] = 2.0 *(p_screen[0]/((double)screen_width)) - 1.0;
  p[1] = 2.0 *(p_screen[1]/((double)screen_height)) - 1.0;

  p[0] = (p[0] - center[0]) / r;
  p[1] = (p[1] - center[1]) / r;

  double sq_len = p[0]*p[0] + p[1]*p[1];

  if (sq_len > 1.0) {
    double d = 1.0 / sqrt(sq_len);
    p[0] *= d;
    p[1] *= d;
    p[2] = 0.0;
  }
  else
    p[2] = sqrt (1.0 - sq_len);

}

void Camera::rotate() {
//   Quat new_rot (0.001, 0.0, 0.0, 1.0);
  
//   // Multiply local rotation by total rotation (order matters!)
//   q_rot = new_rot.composeWith(q_rot);
  // Save initial click

  q_last = q_rot;
  //  Quat new_rot (0.9999, 0.005, 0.001, 0.003);
  Quat new_rot (0.999, 0.00, 0.001, 0.00003);
  
  // Multiply local rotation by total rotation (order matters!)
  q_rot = new_rot.composeWith(q_last);
  q_rot.normalize();
  q_last = q_rot;

}

/// Rotate world
/// @param x Mouse screen x coordinate
/// @param y Mouse screen y coordinate
void Camera::rotate(int x, int y) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  double v0[3], v1[3];
  mapToSphere(mouse_start, v0, radius);
  mapToSphere(mouse_curr, v1, radius);

  // Cross-product (v0, v1)
  double axis[3] = {v0[1]*v1[2] - v0[2]*v1[1], 
		    v0[2]*v1[0] - v0[0]*v1[2], 
		    v0[0]*v1[1] - v0[1]*v1[0]};
  double angle = v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];

  Quat new_rot (angle, axis[0], axis[1], axis[2]);

  // Multiply local rotation by total rotation (order matters!)
  q_rot = new_rot.composeWith(q_last);

  q_rot.normalize();
}

/// Sets mouse button click position
/// @param x Mouse screen x coordinate
/// @param y Mouse screen y coordinate
void Camera::mouseSetClick(int x, int y) {
  mouse_start[0] = x;
  mouse_start[1] = screen_height - y;
  mouse_start[2] = 0.0;
}

/// Translate on z axe.
/// @param x Mouse screen x coordinate
/// @param y Mouse screen y coordinate
void Camera::zooming(int x, int y) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

//   double diff = mouse_curr[1] - mouse_start[1];
//   zoom_factor += diff / screen_height;

  position[2] -= 5.0*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];
}

/// Translate camera position
/// @param x Mouse screen x coordinate
/// @param y Mouse screen y coordinate
void Camera::translate (int x, int y) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  position[0] += (mouse_curr[0] - mouse_start[0]) / screen_width;
  position[1] += (mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];
}

/// Translate ligth position
/// @param x Mouse screen x coordinate
/// @param y Mouse screen y coordinate
void Camera::lightTranslate (int x, int y) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  light_position[0] += 10*(mouse_curr[0] - mouse_start[0]) / screen_width;
  light_position[1] += 10*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];
}

/**
 * Computes the euclidean squared distance between two points.
 * @param p First point.
 * @param q Second point.
 * @return Distance between p and q.
 **/
double Camera::squaredDistance(const double p[3], const double q[3]) const {
  double vec[3];
  vec[0] = q[0] - p[0];
  vec[1] = q[1] - p[1];
  vec[2] = q[2] - p[2];

  return vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
}

/// Returns the rotation matrix
/// @return rotation matrix
const double* Camera::rotationMatrix ( void ) {

  double n, s;
  double xs, ys, zs;
  double wx, wy, wz;
  double xx, xy, xz;
  double yy, yz, zz;

  Quat q = q_rot;

  n = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.a * q.a);
  s = (n > 0.0) ? (2.0 / n) : 0.0;

  xs = q.x * s;  ys = q.y * s;  zs = q.z * s;
  wx = q.a * xs; wy = q.a * ys; wz = q.a * zs;
  xx = q.x * xs; xy = q.x * ys; xz = q.x * zs;
  yy = q.y * ys; yz = q.y * zs; zz = q.z * zs;

  rotation_matrix [0] = 1.0 - (yy + zz);
  rotation_matrix [1] = xy - wz;
  rotation_matrix [2] = xz + wy; 
  rotation_matrix [3] = 0.0;

  rotation_matrix [4] = xy + wz;
  rotation_matrix [5] = 1.0 - (xx + zz);
  rotation_matrix [6] = yz - wx;
  rotation_matrix [7] = 0.0;

  rotation_matrix [8] = xz - wy;
  rotation_matrix [9] = yz + wx;
  rotation_matrix[10] = 1.0 - (xx + yy);
  rotation_matrix[11] = 0.0;

  rotation_matrix[12] = 0.0;
  rotation_matrix[13] = 0.0;
  rotation_matrix[14] = 0.0;
  rotation_matrix[15] = 1.0;

  double xw, yw, zw;
  xx = q.x * q.x;
  xy = q.x * q.y;
  xz = q.x * q.z;
  xw = q.x * q.a;

  yy = q.y * q.y;
  yz = q.y * q.z;
  yw = q.y * q.a;

  zz = q.z * q.z;
  zw = q.z * q.a;

  rotation_matrix[0]  = 1 - 2 * ( yy + zz );
  rotation_matrix[1]  =     2 * ( xy + zw );
  rotation_matrix[2]  =     2 * ( xz - yw );
  rotation_matrix[3] = 0;

  rotation_matrix[4]  =     2 * ( xy - zw );
  rotation_matrix[5]  = 1 - 2 * ( xx + zz );
  rotation_matrix[6]  =     2 * ( yz + xw );
  rotation_matrix[7] = 0;
  
  rotation_matrix[8]  =     2 * ( xz + yw );
  rotation_matrix[9]  =     2 * ( yz - xw );
  rotation_matrix[10] = 1 - 2 * ( xx + yy );
  rotation_matrix[11] = 0;
 
  // Translation vector = -eye
  rotation_matrix[12] = -eye[0];
  rotation_matrix[13] = -eye[1];
  rotation_matrix[14] = -eye[2];

  rotation_matrix[15] = 1;


  return rotation_matrix;
}