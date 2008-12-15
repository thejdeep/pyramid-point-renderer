/**
 * Camera class controls all manipulation in 3D, including light sources.
 * Also keeps mouse information.
 *
 * Author : Ricardo Marroquim
 * Date created : 27-01-07
 *
 **/

#include "camera.h"
#include "matrix.cc"

// Conversion from radians to degrees
const double rad_to_deg = 180.0/M_PI;

/**
 * Constructor with screen dimensions
 * @param w Screen width.
 * @param h Screen height.
 **/
Camera::Camera(const int w, const int h) : screen_width (w), screen_height (h), 
				   zoom_factor(1.0), fov(1.0),
				   z_near(0.01), z_far(100.0) {
  view_mode = PERSPECTIVE;

  static double identity [16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
  std::copy (identity, identity+16, rotation_matrix);

  q_rot = Quat(0.0, 0.0, 0.0, 1.0);
  q_last = q_rot;
  q_lookAt = q_rot;

  // Initialize values
  q_lookAt.normalize();
  //  position = q_lookAt.rotate(Vector(0.0, 0.0, 1.0));

   up = q_lookAt.rotate(Vector(0.0, 1.0, 0.0));
   view_vector = q_lookAt.rotate(Vector(0.0, 0.0, 1.0));
//   up = Vector (0.0, 1.0, 0.0);
//   view_vector = Vector(0.0, 0.0, 1.0);
  position = Point (0.0, 0.0, -5.0);

  track_center = Point (0.0, 0.0, 0.0);

  view_vector.Normalize();
  up.Normalize();

  angle_h = M_PI/2.0;
  angle_v = M_PI/2.0;

  light_position = Point(0.0, 0.0, 1.0);

  radius = 1.0;

}

/**
 * Destructor.
 **/
Camera::~Camera() {
}

/**
 * Initialize light properties.
 **/
void Camera::initLight (void) {
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  ambient_light = Vector( 0.0, 0.0, 0.0 );
  diffuse_light = Vector( 1.0, 1.0, 1.0 );
  specular_light = Vector( 1.0, 1.0, 1.0 );

  GLfloat al[] = {ambient_light[0], ambient_light[1],
		  ambient_light[2], 1.0};

  GLfloat dl[] = {diffuse_light[0], diffuse_light[1],
		  diffuse_light[2], 1.0};

  GLfloat sl[] = {specular_light[0], specular_light[1],
		  specular_light[2], 1.0};

  GLfloat pos[] = {light_position[0], light_position[1],
		  light_position[2], 0.0};

  glLightfv(GL_LIGHT0, GL_AMBIENT, al);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dl);
  glLightfv(GL_LIGHT0, GL_SPECULAR, sl);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

  glShadeModel(GL_SMOOTH);
}

/**
 * Resets the view mode properties.
 * Sets camera projection to perspective or orthographic.
 **/
void Camera::resetViewMode ( void ) {

  double w = (double)screen_width;
  double h = (double)screen_height;

  double x = 1.0 * zoom_factor;
  double y = 1.0 * zoom_factor;

  if (view_mode == PERSPECTIVE)
    gluPerspective( 45, (w/h), z_near, z_far );
  else
    glOrtho( -x, x, -y, y, z_near, z_far );
}

/**
 * Sets OpenGL camera.
 * Initialize viewport and resets projection and modelview matrices.
 * Translates and rotates camera direction and position.
 **/
void Camera::setView (void) {

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, screen_width, screen_height);
  resetViewMode();
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(position[0], position[1], position[2]);

  //initLight();

  //glTranslatef(track_center[0], track_center[1], track_center[2]);
  //glMultMatrixd ( rotationMatrix() );
  //glTranslatef(-track_center[0], -track_center[1], -track_center[2]);

//   target = position + view_vector;
//   gluLookAt(position[0], position[1], position[2],
// 	    target[0], target[1], target[2],
// 	    up[0], up[1], up[2]);
}

/**
 * Sets OpenGL camera.
 * Initialize viewport and resets projection and modelview matrices.
 * Translates and rotates camera direction and position.
 **/
void Camera::setTranslation (void) {
  glTranslatef(position[0], position[1], position[2]);
}

/**
 * Sets OpenGL camera.
 * Initialize viewport and resets projection and modelview matrices.
 * Translates and rotates camera direction and position.
 **/
void Camera::setRotation (void) {

  // Convert from quaternion to angle+axis
  double s = 1.0 / sqrt(1 - q_rot.a*q_rot.a);
  double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
  if (q_rot.a == 1) {
    rot[1] = rot[2] = 0.0; rot[3] = 0.0;
  }

  glRotatef(rot[0], rot[1], rot[2], rot[3]);
}



/**
 * Switch between orthographic and perspective modes
 **/
void Camera::switchViewMode ( void ) {
  if (view_mode == PERSPECTIVE)
    view_mode = ORTHOGRAPHIC;
  else
    view_mode = PERSPECTIVE;
  setView();
}

/** 
 * Switch to given view mode.
 * @param vm Given view mode
 **/
void Camera::switchViewMode ( int vm ) {
  view_mode = vm;
  reshape(screen_width, screen_height);
}

/**
 * Reshape func
 * @param w New window width
 * @param h New window height
 **/
void Camera::reshape(int w, int h) {

  screen_width = w;
  screen_height = h;

  glClearColor (0.0, 0.0, 0.0, 1.0);

  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  resetViewMode ();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void Camera::normalizeCoordinates(Point& p)
{
  double w = screen_width;
  double h = screen_height;

  p[0] = 2.0 * p[0] / w  - 1.0;
  if (p[0] < -1.0) p[0] = -1.0;
  if (p[0] >  1.0) p[0] =  1.0;

  p[1] = -(2.0 * p[1] / h - 1.0);
  if (p[1] < -1.0) p[1] = -1.0;
  if (p[1] >  1.0) p[1] =  1.0;
}

/**
 * Starts camera rotation procedure.
 * Stores initial mouse position.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::startRotation(int x, int y) { 
  mouse_start = Point(x, y, 0.0);
  mouse_curr = mouse_start;
  q_last = q_rot;
  q_last_lookAt = q_lookAt;
}

/**
 * Finalizes rotation procedure.
 **/
void Camera::endRotation( void ) {
  q_last = q_rot;
  q_last_lookAt = q_lookAt;
}

/**
 * Auto-rotate camera.
 **/
void Camera::rotate( void ) {

  mouse_curr[0] += 5;

  Point d_curr = mouse_curr;
  Point d_start = mouse_start;

  normalizeCoordinates(d_curr);
  normalizeCoordinates(d_start);

  mapToSphere(d_start, 1.0);
  mapToSphere(d_curr, 1.0);

  Quat q0 (d_start[0], d_start[1], d_start[2], 0.0);
  Quat q1 (d_curr[0], d_curr[1], d_curr[2], 0.0);

  Quat inc = q0*q1;

  q_lookAt = (q_last_lookAt*inc*q_last_lookAt.conjugate())*q_last_lookAt;
  q_lookAt.normalize();

  view_vector = q_lookAt.rotate(Vector(0.0, 0.0, 1.0));
  up = q_lookAt.rotate(Vector(0.0, 1.0, 0.0));
}

/**
 * Rotate world
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::rotate(int x, int y) {

//   mouse_curr = Point(x, y, 0.0);

//   Point d_curr = mouse_curr;
//   Point d_start = mouse_start;

//   normalizeCoordinates(d_curr);
//   normalizeCoordinates(d_start);

//   mapToSphere(d_start, 1.0);
//   mapToSphere(d_curr, 1.0);

//   Point axis = (d_curr - track_center) ^ (d_start - track_center);

//   float angle = sqrt(d_start.SquaredDistance(d_curr)) / radius;

//   Quat inc(axis[0], axis[1], axis[2], -angle);

//   q_lookAt = inc*q_last;
  //  q_lookAt.normalize();

  mouse_curr = Point(x, y, 0.0);

  Point d_curr = mouse_curr;
  Point d_start = mouse_start;

  normalizeCoordinates(d_curr);
  normalizeCoordinates(d_start);

  d_start[2] = 0.0;
  d_curr[2] = 0.0;

  mapToSphere(d_start, 1.0);
  mapToSphere(d_curr, 1.0);

  Quat q0 (d_start[0], d_start[1], d_start[2], 0.0);
  Quat q1 (d_curr[0], d_curr[1], d_curr[2], 0.0);

  Quat inc = q0*q1;
  q_lookAt = (q_last_lookAt*inc*q_last_lookAt.conjugate())*q_last_lookAt;
  q_lookAt.normalize();

  view_vector = q_lookAt.rotate(Vector(0.0, 0.0, 1.0));
  up = q_lookAt.rotate(Vector(0.0, 1.0, 0.0));

  view_vector.Normalize();
  up.Normalize();
}

/**
 * Starts a rotation procedure for a given object
 * other than the camera itself.
 * @param x Mouse screen x coordinate.
 * @param y Mouse screen y coordinate.
 * @param q Object's rotation quaternion.
 * @param obj_center Position of object being rotated.
 **/
void Camera::startQuatRotation(int x, int y, Quat* q) {
  mouse_start = Point(x, y, 0.0);
  mouse_curr = mouse_start;
  q_last = *q;
}

/**
 * Rotate quaternion of an object other than the camera.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 * @param q Object's rotation quaternion.
 * @param obj_center Position of object being rotated.
 **/
void Camera::rotateQuat(int x, int y, Quat *q, Point* obj_center) {

//   Point screen_obj_center;
//   projectToScreen(obj_center, screen_obj_center);

  mouse_curr = Point(x, y, 0.0);

  Point d_curr = mouse_curr;
  Point d_start = mouse_start;

//   normalizeCoordinates(d_curr);
//   normalizeCoordinates(d_start);

  mapToSphere(d_start, radius);
  mapToSphere(d_curr, radius);


  Point axis = (d_curr - track_center) ^ (d_start - track_center);

  float angle = sqrt(d_start.SquaredDistance(d_curr)) / radius;

  Quat inc(axis[0], axis[1], axis[2], -angle);

  *q = inc*q_last;

  // Quat q0 (d_start[0], d_start[1], d_start[2], 0.0);
  // Quat q1 (d_curr[0], d_curr[1], d_curr[2], 0.0);
  
  // Quat inc = q0*q1;

  // *q = (q_rot.conjugate()*(q_lookAt*inc*q_lookAt.conjugate())*q_rot)*q_last;

  q->normalize();

}

/**
 * Computes inverse rotation for eye position,
 * composes given rotation with current camera rotation.
 * Used to enable back face culling before projecting the vertices.
 * @param q Given rotation.
 * @param new_eye New computed eye position.
 **/
void Camera::computeEyePosition(Quat q, Point *new_eye) {
  Quat q_new_rot = q_rot;
  q_new_rot = q_new_rot.composeWith(q);

  q_new_rot = q_new_rot.inverse();

  // increment given vector by oriented displacement
  *new_eye = (*new_eye) + position;

  //Vector eye = Vector(new_eye->x(), new_eye->y(), new_eye->z());

  //  *new_eye = q_new_rot.rotate(eye);
}

/**
 * Sets mouse button click position
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::mouseSetClick(int x, int y) {
  mouse_start = Point(x, y, 0.0);
}

/*
 * Translate on z axe.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::zooming(int x, int y) {

  mouse_curr = Point(x, y, 0.0);

  Vector dist = 5.0*(mouse_start - mouse_curr);
  dist[0] = 0.0;
  dist[1] /= screen_height;
  dist[2] = 0.0;

  //position -= view_vector * dist[1];
  radius += dist[1];

  mouse_start = mouse_curr;
}

/**
 * Translate camera position
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
**/
void Camera::translate (int x, int y) {

  mouse_curr = Point(x, y, 0.0);

  GLdouble modelview[4][4];
  
  glPushMatrix();  
  setView();
  glGetDoublev(GL_MODELVIEW_MATRIX, &modelview[0][0]);
  glPopMatrix();

  double Mi[4][4];
  inverse2 (modelview, Mi);

  // view point
  double v[4] = {0.0, 0.0, 0.0, 1.0};
  multiply (v, Mi);
  Point vp (v[0], v[1], v[2]);
  Vector vp_vec = vp - Point(0.0,0.0,0.0);

  // view plane
  Vector plane_norm = vp - track_center;
  plane_norm.Normalize();
  double plane_D = plane_norm * (track_center - Point(0.0,0.0,0.0));

  // first line from window (mouse_curr)
  Point line_p;
  Point p (mouse_curr[0], mouse_curr[1], 0.0);
  line_p = projectToWorld(p);

  Vector line_dir = (line_p - vp);
  line_dir.Normalize();

  // intersect line and view plane
  double k = plane_norm * line_dir;
  double r = (plane_D - (plane_norm*vp_vec)) / k;

  Point pi1 = vp + line_dir*r;

  // first line from window (mouse_curr)
  p = Point (mouse_start[0], mouse_start[1], 0.0);
  line_p = projectToWorld(p);
  line_dir = (line_p - vp);
  line_dir.Normalize();

  // intersect line and view plane
  k = plane_norm * line_dir;
  r = (plane_D - (plane_norm*vp_vec)) / k;
  Point pi2 = vp + line_dir*r;

  //cout << pi2 - pi1 << endl ;

  mouse_curr = Point(x, y, 1.0);
  mouse_start[2] = mouse_curr[2];

  Point start = projectToWorld(mouse_start);
  Point current = projectToWorld(mouse_curr);

  Quat rot = q_lookAt;
  rot.inverse();

  Vector rot_vec = current - start;

  track_center += rot.rotate(rot_vec);
  
  mouse_start = mouse_curr;

}

/**
 * Translate a given vector representing an object position other
 * than the camera.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 * @param vec Object's position vector.
 **/
void Camera::translateVec (int x, int y, Point* center) {

  mouse_curr = Point(x, y, 1.0);
  mouse_start[2] = mouse_curr[2];

  Point start = projectToWorld(mouse_start);
  Point current = projectToWorld(mouse_curr);

  *center += (current - start)*0.1;

  mouse_start = mouse_curr;
}

/**
 * Updates the mouse coordinates.
 **/
void Camera::updateMouse ( void ) {
  mouse_start = mouse_curr;
}

/**
 * Translate a given vector on the z axis (zoom),
 * for objects other than the camera.
 * @param x Mouse screen x coordinate.
 * @param y Mouse screen y coordinate.
 * @param vec Object's position vector.
**/
void Camera::zoomingVec (int x, int y, double* factor) {

  mouse_curr = Point(x, y, 1.0);

  Vector dist = 5.0*(mouse_start - mouse_curr);
  dist[0] = 0.0;
  dist[1] /= screen_height;
  dist[2] = 0.0;

  (*factor) += dist[1];

  mouse_start = mouse_curr;

//   mouse_curr = Point(x, y, 1.0);
//   mouse_start[2] = mouse_curr[2];

//   Point start =  projectToWorld(mouse_start);
//   Point current =  projectToWorld(mouse_curr);

//   Vector view = (position*radius) - *center;
//   view.Normalize();

//   (*center) -= view*(current[1] - start[1])*0.1;
}

/**
 * Translate ligth position
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::lightTranslate (int x, int y) {
  mouse_curr = Point(x, screen_height - y, 0.0);

  light_position[0] += 10*(mouse_curr[0] - mouse_start[0]) / screen_width;
  light_position[1] += 10*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start = mouse_curr;
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

/**
 * Returns the rotation matrix
 * @return rotation matrix
**/
const GLdouble* Camera::rotationMatrix ( void ) {

  //  double n, s;
  double xs, ys, zs;
  double wx, wy, wz;
  double xx, xy, xz;
  double yy, yz, zz;

  Quat q = q_lookAt;

//   n = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.a * q.a);
//   s = (n > 0.0) ? (2.0 / n) : 0.0;

  //  xs = q.x * s;  ys = q.y * s;  zs = q.z * s;
  xs = q.x + q.x; ys = q.y + q.y; zs = q.z + q.z;
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

  rotation_matrix[12] = track_center[0];
  rotation_matrix[13] = track_center[1];
  rotation_matrix[14] = track_center[2];
//   rotation_matrix[12] = 0.0;
//   rotation_matrix[13] = 0.0;
//   rotation_matrix[14] = 0.0;
  rotation_matrix[15] = 1.0;

//   double xw, yw, zw;
//   xx = q.x * q.x;
//   xy = q.x * q.y;
//   xz = q.x * q.z;
//   xw = q.x * q.a;

//   yy = q.y * q.y;
//   yz = q.y * q.z;
//   yw = q.y * q.a;

//   zz = q.z * q.z;
//   zw = q.z * q.a;

//   rotation_matrix[0]  = 1 - 2 * ( yy + zz );
//   rotation_matrix[1]  =     2 * ( xy + zw );
//   rotation_matrix[2]  =     2 * ( xz - yw );
//   rotation_matrix[3] = 0;

//   rotation_matrix[4]  =     2 * ( xy - zw );
//   rotation_matrix[5]  = 1 - 2 * ( xx + zz );
//   rotation_matrix[6]  =     2 * ( yz + xw );
//   rotation_matrix[7] = 0;
  
//   rotation_matrix[8]  =     2 * ( xz + yw );
//   rotation_matrix[9]  =     2 * ( yz - xw );
//   rotation_matrix[10] = 1 - 2 * ( xx + yy );
//   rotation_matrix[11] = 0;
 
//   //  Translation vector = position
//   rotation_matrix[12] = track_center[0];
//   rotation_matrix[13] = track_center[1];
//   rotation_matrix[14] = track_center[2];
//   rotation_matrix[15] = 1;

  return rotation_matrix;
}


/**
 * Maps a clicked point on the screen to the arcball sphere
 * @param p Given point
 * @param Mapped point on sphere
 * @param Sphere radius
 **/
void Camera::mapToSphere(Point &p, const double r) const {

  double sq_len =  p[0]*p[0] + p[1]*p[1];
  if (sq_len > r / 2.0)
    p[2] = r / (2*sqrt(sq_len));
  else
    p[2] = sqrt (r - sq_len);

  sq_len = sqrt (p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
  p /= sq_len;
}

/**
 * Projects a given point in world coordinates to screen coordinates.
 * @param p Given point in world coordinates.
 * @param screen_pos Screen position of given point.
 **/
void Camera::projectToScreen(Point* p, Point& screen_pos) {

  GLdouble pos_x, pos_y, pos_z;

  glPushMatrix();
  
  setView();

  GLdouble model_view[16];
  GLdouble projection[16];
  GLint viewport[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  // get 2D coordinates based on 3D coordinates
  gluProject((*p)[0], (*p)[1], (*p)[2],
	     model_view, projection, viewport,
	     &pos_x, &pos_y, &pos_z);

  glPopMatrix();

  screen_pos = Point(pos_x - screen_width*0.5,
		     screen_height*0.5 - pos_y,
		     pos_z);
}

/**
 * Converts a 2D (viewport space) point into 3D (world space) point
 * @param p The point to be converted
 */
Point Camera::projectToWorld(const Point& p) {
  GLdouble xo, yo, zo;

  glPushMatrix();
  setView();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glPopMatrix();

  xo = (p[0]- viewport[0])/ (viewport[2]/2.0) - 1;
  yo = (p[1]- viewport[1])/ (viewport[3]/2.0) - 1;
  zo = 2*p[2] - 1;

//   glPushMatrix();

//   setView();

//   GLdouble model_view[16];
//   GLdouble projection[16];
//   GLint viewport[4];
//   glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
//   glGetDoublev(GL_PROJECTION_MATRIX, projection);
//   glGetIntegerv(GL_VIEWPORT, viewport);

//   gluUnProject(p[0], viewport[3] - p[1], p[2], model_view, projection, viewport, &xo, &yo, &zo);

//   glPopMatrix();

  return Point(xo, yo, zo);
}
