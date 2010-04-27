
// GLSL CODE

/// 1st Vertex Shader

// Projects points to screen space and rotates normal
// stores output on texture

uniform vec3 eye;
uniform int back_face_culling;

varying vec3 minor_axis;
varying vec3 major_axis;
varying float minor_length;
varying float major_length;
varying float depth;
varying float dist_to_eye;

const float PI = 3.14159265358979323846264;

void main(void)
{  
  gl_TexCoord[0] = gl_MultiTexCoord0;

  vec3 normal = normalize(gl_Normal.xyz);
  major_axis = normalize(gl_TexCoord[0].xyz);
  minor_axis = normalize(cross(normal, major_axis));

  vec3 e = gl_Vertex.xyz - eye;

  if ( (back_face_culling == 1) && (normalize(dot(e, normal)) > 0.1 )) {
    minor_length = 0.0;
    major_length = 0.0;

    // for some reason seting the vector to vec4(0.0) drops
    // the performance significantly, at least on the GeForce8800 -- RM 2007-10-19
    gl_Position = vec4(1.0);
  }
  else if (1 == 11) {
    /// Rytz construction

    minor_length = gl_Vertex.w;
    major_length = gl_TexCoord[0].w;

    /// project center and two endpoints of axes
    vec2 M = (gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0)).xy;
    vec2 P = (gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz + major_axis*major_length, 1.0)).xy;
    vec2 Q = (gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz + minor_axis*minor_length, 1.0)).xy;

/*     vec2 M = gl_Vertex.xy; */
/*     vec2 P = gl_Vertex.xy + major_axis.xy*major_length; */
/*     vec2 Q = gl_Vertex.xy + minor_axis.xy*minor_length; */

    /// rotate P in 90 deegres
    float angle = PI*0.5;
    vec2 rot = P - M;
    rot = vec2(rot.x*cos(angle) + rot.y*sin(angle), -rot.x*sin(angle) + rot.y*cos(angle));
    P = M + rot;

    /// midpoint of Rytz circle
    vec2 C = 0.5*(P + Q);

    /// radius of Rytz circle
    float radius = length(C - M);

    /// find intersections points of line passing through P and Q, and the Rytz circle
    vec2 X = C + normalize(P - C) * radius;
    vec2 Y = C + normalize(Q - C) * radius;

    minor_length = length(Q - Y);
    major_length = length(Q - X);

    minor_axis = vec3(normalize(X - M), 0.0);
    major_axis = vec3(normalize(Y - M), 0.0);

/*     if (major_length < minor_length) { */
/*       //      if (alpha < 0) { */
/*       float tmp = major_length; */
/*       major_length = minor_length; */
/*       minor_length = tmp; */
/*       vec3 tmpv; */
/*       tmpv = major_axis;       */
/*       major_axis = minor_axis; */
/*       minor_axis = tmp; */
/*     } */

    dist_to_eye = length(eye - gl_Vertex.xyz);

    // compute depth value without projection matrix, only modelview
    depth = -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z;
    //w = v.w;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);

  }
  else
    {
      // only rotate point and normal if not culled
      vec4 v = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
      
      minor_length = gl_Vertex.w;
      major_length = gl_TexCoord[0].w;

      minor_axis *= minor_length;
      major_axis *= major_length;

      float dot_major = dot(normalize(major_axis), normalize(e));
      float dot_minor = dot(normalize(minor_axis), normalize(e));

      float alpha = 2.0*dot_major*dot_minor;
      float den = (minor_length*minor_length + dot_major*dot_major) 
	- (major_length*major_length + dot_minor*dot_minor);  

      alpha /= den;
      alpha = atan(alpha)*0.5;
      
/*       if (den > 0.0) */
/* 	alpha = mod((alpha + PI*0.5),  PI ); */

      float cos_alpha = cos(alpha);
      float sin_alpha = sin(alpha);

      vec3 rot_major;
      rot_major.x = major_axis.x * cos_alpha + minor_axis.x * sin_alpha;
      rot_major.y = major_axis.y * cos_alpha + minor_axis.y * sin_alpha;
      rot_major.z = major_axis.z * cos_alpha + minor_axis.z * sin_alpha;  

      vec3 rot_minor;
      rot_minor.x = -major_axis.x * sin_alpha + minor_axis.x * cos_alpha;
      rot_minor.y = -major_axis.y * sin_alpha + minor_axis.y * cos_alpha;
      rot_minor.z = -major_axis.z * sin_alpha + minor_axis.z * cos_alpha;

      e = normalize(e);
      vec3 major_proj = rot_major - e * dot(rot_major, e);
      vec3 minor_proj = rot_minor - e * dot(rot_minor, e);

/*       vec3 major_proj = (gl_ModelViewMatrix * vec4(rot_major, 0.0)).xyz; */
/*       vec3 minor_proj = (gl_ModelViewMatrix * vec4(rot_minor, 0.0)).xyz; */

/*       vec3 major_proj = gl_NormalMatrix * rot_major; */
/*       vec3 minor_proj = gl_NormalMatrix * rot_minor; */

      major_length = length(major_proj);
      minor_length = length(minor_proj);
      major_axis = normalize(major_proj);
      minor_axis = normalize(minor_proj);

      if (major_length < minor_length) {
      //      if (alpha < 0) {
	float tmp = major_length;
	major_length = minor_length;
	minor_length = tmp;
	major_axis = normalize(minor_axis);
	minor_axis = normalize(major_axis);
      }

      if (abs(dot(major_axis, minor_axis)) < 0.1)
	  minor_length = 0.0;
	

/*       if (length(major_proj*sin_alpha + minor_proj*cos_alpha) < (length(major_proj*cos_alpha - minor_proj*sin_alpha))) { */
/* 	major_length = length(major_proj.xy); */
/* 	minor_length = length(minor_proj.xy); */
/* 	major_axis = normalize(major_axis); */
/* 	minor_axis = normalize(minor_axis); */
/*       } */
/*       else { */
/* 	major_length = length(minor_proj.xy); */
/* 	minor_length = length(major_proj.xy); */
/* 	major_axis = normalize(minor_axis); */
/* 	minor_axis = normalize(major_axis); */
/*       } */

      dist_to_eye = length(eye - gl_Vertex.xyz);

      // compute depth value without projection matrix, only modelview
      depth = -(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0)).z;
      //w = v.w;

      gl_Position = v;
    }
}
