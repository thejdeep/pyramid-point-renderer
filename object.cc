#include "object.h"

#define PI 3.14159265

// Conversion from radians to degrees
const double rad_to_deg = 180.0/PI;

/**
 * Render object using designed rendering system.
 **/
void Object::render ( void ) {

  glTranslatef(center[0], center[1], center[2]);

    // Convert from quaternion to angle+axis
  double s = 1.0 / sqrt(1 - q_rot.a*q_rot.a);
  double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
  if (q_rot.a == 1) {
    rot[1] = rot[2] = 0.0; rot[3] = 0.0;
  }
  glRotatef(rot[0], rot[1], rot[2], rot[3]);
}
