/**
 * Camera class controls all manipulation in 3D, including light sources.
 * Also keeps mouse information.
 *
 * Author : Ricardo Marroquim
 * Date created : 27-01-07
 *
 **/

#include "camera.h"

// Conversion from radians to degrees
const double rad_to_deg = 180.0/PI;

/**
 * Constructor with screen dimensions
 * @param w Screen width.
 * @param h Screen height.
 **/
Camera::Camera(const int w, const int h) : screen_width (w), screen_height (h), 
				   zoom_factor(1.0), fov(1.0),
				   z_near(0.001), z_far(100.0) {
  view_mode = PERSPECTIVE;

  static double identity [16] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
  std::copy (identity, identity+16, rotation_matrix);

  q_last.a = 1.0; q_last.x = 0.0; q_last.y = 0.0; q_last.z = 0.0;
  q_rot.a = 1.0; q_rot.x = 0.0; q_rot.y = 0.0; q_rot.z = 0.0;

  position = Point(0.0, 0.0, -3.0);
    
  light_position[0] = 0.0; light_position[1] = 0.0; light_position[2] = 1.0; light_position[3] = 0.0;

  radius = 1.0;

  frame_video = -1.0;

  keyFrameInterpolation = 0;
}

/**
 * Destructor.
 **/
Camera::~Camera() {
  clearFrames();
}

/**
 * Initialize light properties.
 **/
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

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

  glShadeModel(GL_SMOOTH);
}

/** 
 * Computes camera properties while interpolating key frame.
 **/
void Camera::computeKeyFrame( void ) {
  unsigned int frame = (unsigned int)frame_video;
  double step = frame_video - frame;

  if ( keyFrames.size() - 2 == frame ) {
    frame_video = -1.0;
  }
  else {
    position[0] = keyFrames[frame].pos[0] + (keyFrames[frame+1].pos[0]-keyFrames[frame].pos[0])*(step);
    position[1] = keyFrames[frame].pos[1] + (keyFrames[frame+1].pos[1]-keyFrames[frame].pos[1])*(step);
    position[2] = keyFrames[frame].pos[2] + (keyFrames[frame+1].pos[2]-keyFrames[frame].pos[2])*(step);
    light_position[0] = keyFrames[frame].light_pos[0] + 
      (keyFrames[frame+1].light_pos[0]-keyFrames[frame].light_pos[0])*(step);
    light_position[1] = keyFrames[frame].light_pos[1] + 
      (keyFrames[frame+1].light_pos[1]-keyFrames[frame].light_pos[1])*(step);
    light_position[2] = keyFrames[frame].light_pos[2] + 
      (keyFrames[frame+1].light_pos[2]-keyFrames[frame].light_pos[2])*(step);

    frame_reconstruction_filter = keyFrames[frame].reconstruction_filter + 
      (keyFrames[frame+1].reconstruction_filter-keyFrames[frame].reconstruction_filter)*(step);
    frame_prefilter = keyFrames[frame].prefilter + 
      (keyFrames[frame+1].prefilter-keyFrames[frame].prefilter)*(step);

    if (!keyFrameInterpolation)
      q_rot = slerp(keyFrames[frame].rot, keyFrames[frame+1].rot, step);
    else {
      if (frame > 1)
	q_rot = cubicInterpolation(keyFrames[frame-1].rot, keyFrames[frame].rot, 
				   keyFrames[frame+1].rot, keyFrames[frame+2].rot, step);
      else
	q_rot = cubicInterpolation(keyFrames[frame].rot, keyFrames[frame].rot, 
				   keyFrames[frame+1].rot, keyFrames[frame+2].rot, step);
    }

    frame_video += 0.005;
  }
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

  initLight();

  if ( runningFrames() )
    computeKeyFrame();
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
}

/**
 * Resets the view mode properties.
 * Sets camera projection to perspective or orthographic.
 **/
void Camera::resetViewMode ( void ) {

  double w = (double)screen_width;
  double h = (double)screen_height;

//   double diag = sqrt (w*w + h*h);
//   double top = h / diag * 0.5 * fov * z_near;
//   double bottom = - top;
//   double right = w / diag * 0.5 * fov * z_near;
//   double left = -right;

//   left *= zoom_factor;
//   right *= zoom_factor;
//   top *= zoom_factor;
//   bottom *= zoom_factor;
  double x = 1.0 * zoom_factor;
  double y = 1.0 * zoom_factor;

  if (view_mode == PERSPECTIVE)
    //  glFrustum(left, right, bottom, top, z_near, z_far);
    gluPerspective( 45, (w/h), z_near, z_far );
  else
    glOrtho( -x, x, -y, y, z_near, z_far );
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

  // Arcball radius
  radius = 1.0;

  glClearColor (0.0, 0.0, 0.0, 1.0);

  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  resetViewMode ();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/**
 * Starts camera rotation procedure.
 * Stores initial mouse position.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::startRotation(int x, int y) { 
  mouse_start[0] = x;
  mouse_start[1] = screen_height - y;
  mouse_start[2] = 0.0;
}

/**
 * Starts a rotation procedure for a given object
 * other than the camera itself.
 * @param x Mouse screen x coordinate.
 * @param y Mouse screen y coordinate.
 * @param q Object's rotation quaternion.
 * @param obj_center Position of object being rotated.
 **/
void Camera::startQuatRotation(int x, int y, Quat* q, Point* obj_center) {

  q_last = *q;

  double screen_obj_center[3];
  projectToScreen(obj_center, screen_obj_center);

  mouse_start[0] = x - screen_obj_center[0];
  mouse_start[1] = screen_height - (y - screen_obj_center[1]);
  mouse_start[2] = 0.0;
}

/**
 * Finalizes rotation procedure.
 **/
void Camera::endRotation( void ) {
  q_last = q_rot;
}

/**
 * Maps a clicked point on the screen to the arcball sphere
 * @param p Given point
 * @param Mapped point on sphere
 * @param Sphere radius
 **/
void Camera::mapToSphere(const double p_screen[3], double p[], const double r) const {

  p[0] = 2.0 *(p_screen[0]/((double)screen_width)) - 1.0;
  p[1] = 2.0 *(p_screen[1]/((double)screen_height)) - 1.0;

  p[0] = (p[0] - position.x()) / r;
  p[1] = (p[1] - position.y()) / r;

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

/**
 * Projects a given point in world coordinates to screen coordinates.
 * @param p Given point in world coordinates.
 * @param screen_pos Screen position of given point.
 **/
void Camera::projectToScreen(Point* p, double* screen_pos) {

  GLdouble pos_x, pos_y, pos_z;

  setView();

  GLdouble model_view[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
	
  // get 2D coordinates based on 3D coordinates

  gluProject(p->x(), p->y(), p->z(),
	     model_view, projection, viewport,
	     &pos_x, &pos_y, &pos_z);

  screen_pos[0] = pos_x - screen_width*0.5;
  screen_pos[1] = screen_height - pos_y - screen_height*0.5;
  screen_pos[2] = pos_z;

}

/**
 * Auto-rotate camera.
 **/
void Camera::rotate( void ) {

  q_last = q_rot;
  Quat new_rot (0.00, 0.001, 0.00003, 0.999);
  
  // Multiply local rotation by total rotation (order matters!)
  q_rot = new_rot.composeWith(q_last);
  q_rot.normalize();
  q_last = q_rot;
}

/**
 * Rotate world
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
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

  Quat new_rot (axis[0], axis[1], axis[2], angle);

  // Multiply local rotation by total rotation (order matters!)
  q_rot = new_rot.composeWith(q_last);
  q_rot.normalize();

}

/**
 * Rotate quaternion of an object other than the camera.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 * @param q Object's rotation quaternion.
 * @param obj_center Position of object being rotated.
 **/
void Camera::rotateQuat(int x, int y, Quat *q, Point* obj_center) {

  double screen_obj_center[3];
  projectToScreen(obj_center, screen_obj_center);

  mouse_curr[0] = x - screen_obj_center[0];
  mouse_curr[1] = screen_height - (y - screen_obj_center[1]);
  mouse_curr[2] = 0.0;

  double v0[3], v1[3];
  mapToSphere(mouse_start, v0, radius);
  mapToSphere(mouse_curr, v1, radius);

  // Cross-product (v0, v1)
  double axis[3] = {v0[1]*v1[2] - v0[2]*v1[1], 
		    v0[2]*v1[0] - v0[0]*v1[2], 
		    v0[0]*v1[1] - v0[1]*v1[0]};
  double angle = v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];

  Quat new_rot (axis[0], axis[1], axis[2], angle);

  // Multiply local rotation by total rotation (order matters!)
  *q = new_rot.composeWith(q_last);

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

//   Quat q_new_rot = q;
//   q_new_rot = q_new_rot.composeWith(q_rot);

  q_new_rot = q_new_rot.inverse();

  // increment given vector by oriented displacement
  (*new_eye)[0] = new_eye->x() + position[0];
  (*new_eye)[1] = new_eye->y() + position[1];
  (*new_eye)[2] = new_eye->z() + position[2];

  //cout << new_eye[0] << " " << new_eye[1] << " " << new_eye[2] << endl;

  q_new_rot.rotate(new_eye);
}

/**
 * Sets mouse button click position
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::mouseSetClick(int x, int y) {
  mouse_start[0] = x;
  mouse_start[1] = screen_height - y;
  mouse_start[2] = 0.0;
}

/*
 * Translate on z axe.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
void Camera::zooming(int x, int y) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  position[2] -= 10.0*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];
}

/**
 * Translate camera position
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
**/
void Camera::translate (int x, int y) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  position[0] += 5.0*(mouse_curr[0] - mouse_start[0]) / screen_width;
  position[1] += 5.0*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];
}

/**
 * Translate a given vector representing an object position other
 * than the camera.
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 * @param vec Object's position vector.
 **/
void Camera::translateVec (int x, int y, Point* center) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  // translation displacement vector
  Point rotVec;

  rotVec[0] = 10*(mouse_curr[0] - mouse_start[0]) / screen_width;
  rotVec[1] = 10*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];

  // rotates displacement to align with object axis
  Quat q = q_rot;
  q.x *= -1; q.y *= -1; q.z *= -1;
  q.rotate(&rotVec);

  // increment given vector by oriented displacement
  (*center)[0] = center->x() + rotVec[0];
  (*center)[1] = center->y() + rotVec[1];
  (*center)[2] = center->z() + rotVec[2];
}

/**
 * Updates the mouse coordinates.
 **/
void Camera::updateMouse ( void ) {

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];
}

/**
 * Translate a given vector on the z axis (zoom),
 * for objects other than the camera.
 * @param x Mouse screen x coordinate.
 * @param y Mouse screen y coordinate.
 * @param vec Object's position vector.
**/
void Camera::zoomingVec (int x, int y, Point* center) {

  mouse_curr[0] = x;
  mouse_curr[1] = screen_height - y;
  mouse_curr[2] = 0.0;

  // translation displacement vector
  Point rotVec;

  rotVec[2] -= 15.0*(mouse_curr[1] - mouse_start[1]) / screen_height;

  mouse_start[0] = mouse_curr[0];
  mouse_start[1] = mouse_curr[1];
  mouse_start[2] = mouse_curr[2];

  // rotates displacement to align with object axis
  Quat q = q_rot;
  q.x *= -1; q.y *= -1; q.z *= -1;
  q.rotate(&rotVec);

  // increment given vector by oriented displacement
  (*center)[0] = center->x() + rotVec[0];
  (*center)[1] = center->y() + rotVec[1];
  (*center)[2] = center->z() + rotVec[2];
}

/**
 * Translate ligth position
 * @param x Mouse screen x coordinate
 * @param y Mouse screen y coordinate
 **/
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

/**
 * Returns the rotation matrix
 * @return rotation matrix
**/
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
 
  // Translation vector = position
  rotation_matrix[12] = position[0];
  rotation_matrix[13] = position[1];
  rotation_matrix[14] = position[2];

  rotation_matrix[15] = 1;


  return rotation_matrix;
}