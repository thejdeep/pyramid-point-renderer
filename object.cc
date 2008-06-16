#include "object.h"

// Conversion from radians to degrees
const double rad_to_deg = 180.0/M_PI;

/**
 * Render object using designed rendering system.
 **/
void Object::render ( void ) {

  glTranslatef(center.x(), center.y(), center.z());

    // Convert from quaternion to angle+axis
  double s = 1.0 / sqrt(1 - q_rot.a*q_rot.a);
  double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
  if (q_rot.a == 1) {
    rot[1] = rot[2] = rot[3] = 0.0;
  }

  glRotatef(rot[0], rot[1], rot[2], rot[3]);
}

void Object::rotate ( void ) {

    // Convert from quaternion to angle+axis
  double s = 1.0 / sqrt(1.0 - q_rot.a*q_rot.a);

  if (abs(s) < 0.0005)
    s = 1.0;

  double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
  //  if (q_rot.a == 1)
  //    rot[1] = rot[2] = 0.0; rot[3] = 0.0;

  glRotatef(rot[0], rot[1], rot[2], rot[3]);
}
void Object::translate ( void ) {
  glTranslatef(center.x(), center.y(), center.z());
}

void Object::render ( Point camera_pos ) {

  Point c = Point (center.x() + camera_pos.x(),
		   center.y() + camera_pos.y(),
		   center.z() + camera_pos.z());

  glTranslatef(c.x(), c.y(), c.z());

  rotate();

//     // Convert from quaternion to angle+axis
//   double s = 1.0 / sqrt(1 - q_rot.a*q_rot.a);
//   double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
// 		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
//   if (q_rot.a == 1) {
//     rot[1] = rot[2] = 0.0; rot[3] = 0.0;
//   }
//   glRotatef(rot[0], rot[1], rot[2], rot[3]);
}
