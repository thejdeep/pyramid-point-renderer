/**
 * ewa_surface_splatting.cc Software EWA Surface Splatting.
 *
 * This is the splatting pipeline as implemented
 * by Jussi Rasanen in his M.Sc. thesis :
 * "Surface Splatting: Theory, Extensions and Implementation"
 * Helsinki University of Technology, 2002
 *
 * Also, see Bart Adam's implementation of Jussi's code
 * in his Surfel Viewer application:
 *
 * See Appendix A for source code.
 *
 * history:	created  04-Jul-07
 **/

#include "ewa_surface_splatting.h"
#include "matrix.cc"

/**
 * Default constructor.
 **/
EWASurfaceSplatting::EWASurfaceSplatting() : PointBasedRender() {
  fb = new GLfloat[canvas_width * canvas_height * 3];
  interpolate_normals = 0;
}

EWASurfaceSplatting::EWASurfaceSplatting(int w, int h, bool interp) : PointBasedRender(w, h) {
  fb = new GLfloat[canvas_width * canvas_height * 3];
  interpolate_normals = interp;
}

EWASurfaceSplatting::~EWASurfaceSplatting() {
  delete fb;
}

Vector EWASurfaceSplatting::perpendicular( const Vector &v ) {
  Vector t;
  // select the shortest of projections of axes on v
  // (the closest to perpendicular to v),
  // and project it to the plane defined by v

  if( fabs(v.x()) < fabs(v.y()) ) { // x < y
    if( fabs(v.x()) < fabs(v.z()) ) { // x < y && x < z
      t = Vector (1.0f - v.x() * v.x(), -v.x() * v.y(), -v.x() * v.z());
      return t;
    }
  }
  else { // y <= x
    if( fabs(v.y()) < fabs(v.z()) ) { // y <= x && y < z
      t = Vector( -v.y() * v.x(), 1.0f - v.y() * v.y(), -v.y() * v.z());
      return t;
    }
  }

  // z <= x && z <= y
  t = Vector(-v.z() * v.x(), -v.z() * v.y(), 1.0f - v.z() * v.z());
  return t;
}

/// Computes the splat color using phong lighting model
void EWASurfaceSplatting::phongShader ( double n[3], double c[]) {
  
  // Diffuse color
  double light[3] = {light_dir[0], light_dir[1], light_dir[2]};

  double n_len = 1.0/sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
  n[0] *= n_len;
  n[1] *= n_len;
  n[2] *= n_len;

  double oolight_len = 1.0/sqrt(light[0]*light[0] + light[1]*light[1] + light[2]*light[2]);
  light[0] *= oolight_len;
  light[1] *= oolight_len;
  light[2] *= oolight_len;
  
  double color[4] = {Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]};

  double light_dot_n = n[0]*light[0] + n[1]*light[1] + n[2]*light[2];
  if (light_dot_n > 0.0) {
    double s = pow(light_dot_n, (double)Mats[material_id][12]);
    double diffuse[4] = {Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]};
    double specular[4] = {Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]};
    for (int i = 0; i < 4; ++i) {
      color[i] += diffuse[i] * light_dot_n;
      color[i] += specular[i] * s;
    }
  }

  c[0] = color[0];
  c[1] = color[1];
  c[2] = color[2];
}

/// Computes the splat color using phong lighting model
void EWASurfaceSplatting::splatShader ( surfelVectorIter s, double mv[][4], double c[]) {
  
  // Diffuse color
  double light[3] = {light_dir[0], light_dir[1], light_dir[2]};
  Vector sn = s->Normal();

  // Using arrays instead of cgal vector just to make it faster - RM - 02-02-07
  double n[3] = { sn.x(), sn.y(), sn.z() };

  // Multiply normal by modelview matrix, current orientation on object space
  double n_mv[3] = {n[0]*mv[0][0] + n[1]*mv[1][0] + n[2]*mv[2][0],
		    n[0]*mv[0][1] + n[1]*mv[1][1] + n[2]*mv[2][1],
		    n[0]*mv[0][2] + n[1]*mv[1][2] + n[2]*mv[2][2]};

  double n_len = 1.0/sqrt(n_mv[0]*n_mv[0] + n_mv[1]*n_mv[1] + n_mv[2]*n_mv[2]);
  n_mv[0] *= n_len;
  n_mv[1] *= n_len;
  n_mv[2] *= n_len;

  double oolight_len = 1.0/sqrt(light[0]*light[0] + light[1]*light[1] + light[2]*light[2]);
  light[0] *= oolight_len;
  light[1] *= oolight_len;
  light[2] *= oolight_len;
  
  double color[4] = {Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]};

  double light_dot_n = n_mv[0]*light[0] + n_mv[1]*light[1] + n_mv[2]*light[2];
  if (light_dot_n > 0.0) {
    double s = pow(light_dot_n, (double)Mats[material_id][12]);
    double diffuse[4] = {Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]};
    double specular[4] = {Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]};
    for (int i = 0; i < 4; ++i) {
      color[i] += diffuse[i] * light_dot_n;
      color[i] += specular[i] * s;
    }
  }

  c[0] = color[0];
  c[1] = color[1];
  c[2] = color[2];
}

void EWASurfaceSplatting::setVertices( vector<Surfeld> *s ) {
  surfels = s;

  zbuffer.clear();
  zbuffer.resize( canvas_width * canvas_height );

  zBufferPixel pixel;
  pixel.r = bg_color.x();
  pixel.g = bg_color.y();
  pixel.b = bg_color.z();
  pixel.weight = 0.0;
  pixel.z = -HUGE;
  pixel.zmax = HUGE;
  pixel.radius = 0.0;
  pixel.dx = 0.0;
  pixel.dy = 0.0;

  for (int i = 0; i < canvas_width * canvas_height; ++i)
    zbuffer.push_back(pixel);
}

/// Splat renderer as described in Jussi Hasanen Master Thesis
/// "Surface Splatting: Theory, Extensions and Implementation"
/// Appendix A
/// @param surfels List of surfels.
void EWASurfaceSplatting::draw ( void ) {

#ifdef DEBUG
  static int sta_sh=0, end_sh=0; 
  sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

  double modelview[4][4];
  glGetDoublev(GL_MODELVIEW_MATRIX, &modelview[0][0]);

  double projection[4][4];
  glGetDoublev(GL_PROJECTION_MATRIX, &projection[0][0]);

  /// Mapping from object-space to screen-space (ModelviewProjection) transposed
  double object_to_screen[4][4];
  multiply (modelview, projection, object_to_screen);
 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, canvas_width, 0, canvas_height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

#ifdef DEBUG
  end_sh = glutGet(GLUT_ELAPSED_TIME);
  cout << "init1 time : " << (end_sh - sta_sh) / 1000.0 << endl;
  sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

  // Set up the accumulation buffer
  for (int i = 0; i < canvas_width * canvas_height; ++i) {
    zbuffer[i].weight = 1.0;
    zbuffer[i].z = HUGE;
    zbuffer[i].r = bg_color.x();
    zbuffer[i].g = bg_color.y();
    zbuffer[i].b = bg_color.z();
  }

#ifdef DEBUG
  end_sh = glutGet(GLUT_ELAPSED_TIME);
  cout << "buffer1 time : " << (end_sh - sta_sh) / 1000.0 << endl;
  double loop_sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

  // F = Size of reconstruction filter * 1/2
  // Affine approximation parameter, truncates the resampling filter
  // page 46 : 3.6.2 Truncating the Resampling Filter
  double F = reconstruction_filter_size;
  if (reconstruction_filter_size <= 0.0)
    F = 0.1;

  /// Ill-condition threshold
  double condition_threshold = 100.0;

  /// Scaling factor from homogenous screen-space to viewport
  double vp_scale[2] = {canvas_width*0.5, canvas_height*0.5};

  /// Coordinates of the viewport
  double vp_mid[2] = {canvas_width*0.5, canvas_height*0.5};

  // Exponential loookup table
  int exp_table_size = 256;
  double exp_table[exp_table_size];
  for (int i = 0; i < 256; i++) {
    exp_table[i] = exp(-0.5f * F * double(i)/256.0f);
  }

  double exp_table_scale = ((double)exp_table_size)/F;

  // one over view port scale
  double oovp_scale[2] = {1.0/vp_scale[0], 1.0/vp_scale[1]};

  // Prefilter Variance matrix 
  double prefilter_size_temp = prefilter_size;
  // -- page 57 : 4.1.1 Splat Setup (4.13)
  double Vh [2][2] = {{prefilter_size_temp * oovp_scale[0] * oovp_scale[0], 0.0},
		      {0.0, prefilter_size_temp * oovp_scale[1] * oovp_scale[1]}};

  // Eye vector : for back face culling
  // Multiply by inverse of modelview, turn in opposite direction
  // from points rotation
  // In this case the eye is being rotate and the points are fixed
  Point eye_vec (eye[0], eye[1], eye[2]);

#ifdef DEBUG
  end_sh = glutGet(GLUT_ELAPSED_TIME);
  cout << "init2 time : " << (end_sh - sta_sh) / 1000.0 << endl;
  sta_sh = glutGet(GLUT_ELAPSED_TIME);
  double setup_time=0, shader_time=0, raster_time=0, culling_time=0;
  int cnt_surfs = 0;
#endif


  /****************** Variables used during loop - avoid reallocation*** ********/
  // Splat variables
  Point p0;
  Vector n, dir;
  double d_len, cos;

  // Rasterization variables
  double z, weight, pixelZ, radius, uv[2]; //r_factor
  zBufferPixel pixel;
  /******************************************************************************/


  int clipped_surfs = 0;
  /******************************* Splat Setup Loop ********************************************/
  for (surfelVectorIter it = surfels->begin(); it != surfels->end(); ++it) {
#ifdef DEBUG
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

    p0 = it->Center();
    n = it->Normal();

    // Back-face culling
    dir = eye_vec - p0; 
    d_len = sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
    cos = (dir * n) / d_len;

#ifdef DEBUG
    end_sh = glutGet(GLUT_ELAPSED_TIME);
    culling_time += (end_sh - sta_sh) / 1000.0;
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

    if (cos < 0.0)
      continue;

#ifdef DEBUG
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
    ++cnt_surfs;
#endif


    // Tangent plane in object space
    Vector tu (perpendicular(n));
    tu.normalize();
    Vector tv = n ^ tu;
    tv.normalize();

    // Transform the tangent vectors and the splat center to screen-space
    Vector tuh ( tu.x() * object_to_screen[0][0] + tu.y() * object_to_screen[1][0] + tu.z() * object_to_screen[2][0],
		 tu.x() * object_to_screen[0][1] + tu.y() * object_to_screen[1][1] + tu.z() * object_to_screen[2][1],
		 tu.x() * object_to_screen[0][2] + tu.y() * object_to_screen[1][2] + tu.z() * object_to_screen[2][2]);
    
    Vector tvh ( tv.x() * object_to_screen[0][0] + tv.y() * object_to_screen[1][0] + tv.z() * object_to_screen[2][0],
		 tv.x() * object_to_screen[0][1] + tv.y() * object_to_screen[1][1] + tv.z() * object_to_screen[2][1],
		 tv.x() * object_to_screen[0][2] + tv.y() * object_to_screen[1][2] + tv.z() * object_to_screen[2][2]);

    Point p0h ( p0.x() * object_to_screen[0][0] + p0.y() * object_to_screen[1][0] + p0.z() * object_to_screen[2][0] + object_to_screen[3][0],
		p0.x() * object_to_screen[0][1] + p0.y() * object_to_screen[1][1] + p0.z() * object_to_screen[2][1] + object_to_screen[3][1],
		p0.x() * object_to_screen[0][2] + p0.y() * object_to_screen[1][2] + p0.z() * object_to_screen[2][2] + object_to_screen[3][2]);

    double tuh_w = tu.x() * object_to_screen[0][3] + tu.y() * object_to_screen[1][3] + tu.z() * object_to_screen[2][3];
    double tvh_w = tv.x() * object_to_screen[0][3] + tv.y() * object_to_screen[1][3] + tv.z() * object_to_screen[2][3];
    double p0h_w = p0.x() * object_to_screen[0][3] + p0.y() * object_to_screen[1][3] + p0.z() * object_to_screen[2][3] + object_to_screen[3][3];
      

    // View-frustum culling, skip splats that are totally outside the view frustum
    double r = it->Radius();
    if ((p0h.x() + r < -p0h_w) || (p0.x() - r > p0h_w) ||
	(p0h.y() + r < -p0h_w) || (p0.y() - r > p0h_w) ||
	(p0h.z() + r < -p0h_w) || (p0.z() - r > p0h_w)) {
      ++clipped_surfs;
      continue;	
    }

    // Mapping from splat coodinate system to screen space
    // See page 21 : 3.1.3 Rational Linear Interpolant
    // equation (3.14) shows u and v on splat space, and here tu and tv are on screen space
     double M[4][4] = {{tuh.x(), tuh.y(), tuh_w, 0.0},
		       {tvh.x(), tvh.y(), tvh_w, 0.0},
		       {p0h.x(), p0h.y(), p0h_w, 0.0},
		       {0.0, 0.0, 0.0, 1.0}};


    if (fabs(det4x4(M)) < EPS)
      continue;

    // Mapping from screen to splat coordinate system
    double Mi[4][4];
    inverse2 (M, Mi);


    // Discard the splat if the mapping condition number is too high
    double condition_number = infNorm (M) * infNorm (Mi);
    if ( condition_number > condition_threshold) 
      continue;

    // Variance matrix for circular reconstruction kernel
    // page 54 : 4.1.1 Transformations (4.6)
    double Vr[2][2] = {{r, 0.0}, 
		       {0.0, r}};

    // Central Conic Matrix has the form :
    // | A B | = inv(V*V_t) = inv(V*V) since V=V_t
    // | B C |
    // Central Conic : Ax^2 + 2Bxy + Cy^2 = F
    // Q is the inverse of the variance matrix of the splat
    double Q[2][2];
    Q[0][0] = 1.0/(Vr[0][0]*Vr[0][0]);
    Q[0][1] = 0.0;
    Q[1][0] = 0.0;
    Q[1][1] = Q[0][0];

    // Homogenous conic on the splat plane
    // -- page 34 : 3.4.1 Implicit Conics
    // | A  B  D |
    // | B  C  E |
    // | D  E -F |
    double Qh[4][4] = {{ Q[0][0], Q[0][1], 0.0, 0.0 },
		       { Q[1][0], Q[1][1], 0.0, 0.0 },
		       {     0.0,     0.0,  -F, 0.0 },
		       {     0.0,     0.0, 0.0, 1.0 }};


    // Mapping of a conic Qhs = Mi * Qh * Mi_t
    // Qhs = Q'h in text
    // Qhs is the homogenous conic in screen space
    // -- page 37 : 3.4.5 Linear, Affine and Projective Mappings of Conics (3.60)
    // The multiplication order is inverted since this are row-major matrices,
    // and the method expects column major
    // Column-major = (Row-major)_t
    // Rule : (A * B)_t = B_t * A_t
    // Thus just by inverting the multiplication it works
    double tempQhs [4][4];
    double Qhs [4][4];
    double Mi_t [4][4];
    transpose (Mi, Mi_t);
    multiply (Qh, Mi, tempQhs);
    multiply (Mi_t, tempQhs, Qhs);

    // Draw only ellipses : delta = A*C - B*B > 0 : delta = determinant = discriminant
    double delta = Qhs[0][0] * Qhs[1][1] - Qhs[0][1] * Qhs[1][0];
    if (delta <= 0.0)
      continue;

    double oodelta = 1.0 / delta;

    // Computing offsets : transforming general conic to central conic 
    // -- page 35 : 3.4.2 Transforming a General Conic to a Central Conic
    // xt = (BE - CD) / delta
    // yt = (BD - AE) / delta
    double xt = (Qhs[0][1] * Qhs[1][2] - Qhs[1][1] * Qhs[0][2]) * oodelta;
    double yt = (Qhs[0][1] * Qhs[0][2] - Qhs[0][0] * Qhs[1][2]) * oodelta;

    // page 39 : 3.4.6 Central Form of a Projective Conic (3.63)
    // Qs = Affine approximated conic matrix
    // Qs = Q''' in text
    double Qs[2][2] = {{Qhs[0][0], Qhs[0][1]},
		       {Qhs[1][0], Qhs[1][1]}};

    // page 39 : 3.4.6 Central Form of a Projective Conic (3.64)
    // -F - Dxt - Eyt > 0 for ellipses : same sign as discriminant (3.67)
    // !!  RM 31-01-07 -- must check, in text f - dxt - eyt --> f is positive
    double f = -Qhs[2][2] - Qhs[0][2] * xt - Qhs[1][2] * yt;
    if ( f <= 0.0 )
      continue;

    // Retaining original scaling : page 39 : 3.4.6 Central Form of a Projective Conic (3.66)
    double Ff = F/f;
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j)
	Qs[i][j] *= Ff;

    // Make the resampling filter : Qrho = Invert (Invert(Qs) + Vh)
    // page 57 : 4.1.4 Splat Setup
    // Vrho = Vh + Qsi (4.15) see also page 45 (3.84)
    // Variance of resampling filter rho = variance of prefilter + affinely approximated reconstruction filter
    
     if (det(Qs) <= 0.0)
       continue; //draw only ellipses
     double Qrho[2][2];
     double Qsi [2][2], Vrho[2][2];
     inverse (Qs, Qsi);
     add (Qsi, Vh, Vrho);
     inverse (Vrho, Qrho);
    
    // Explicit code as in Appendix A, same as above
    // but runs a little faster
//     double Qrho[2][2];
//     double detQs = det (Qs);
//     if (detQs <= 0.0)
//       continue; //draw only ellipses
//     Qs[0][0] += detQs * Vh[1][1];
//     Qs[0][1] -= detQs * Vh[0][1];
//     Qs[1][0] -= detQs * Vh[1][0];
//     Qs[1][1] += detQs * Vh[0][0];

//     double newDet = detQs / det(Qs);
//     Qrho[0][0] = Qs[0][0] * newDet;
//     Qrho[0][1] = Qs[0][1] * newDet;
//     Qrho[1][0] = Qs[1][0] * newDet;
//     Qrho[1][1] = Qs[1][1] * newDet;

    // Draw only ellipses
    delta = det (Qrho);
    if (delta <= 0.0)
      continue;

    // Compute bounding box min and max values
    // page 37 : 3.4.4 Bounding Rectangle of a Conic (3.55)
    // xt += sqrt ( (C(F - Dxt - Eyt))/delta )
    // yt += sqrt ( (A(F - Dxt - Eyt))/delta )
    // page 57 : 4.1.4 Splat Setup D=E=0 --> Central Conic
    oodelta = 1.0 / delta;
    double bx = sqrt (Qrho[1][1] * F * oodelta);
    double by = sqrt (Qrho[0][0] * F * oodelta);
    double xmin = xt - bx, xmax = xt + bx;
    double ymin = yt - by, ymax = yt + by;

    // Integer bounding rectangle
    // page 57 : 4.1.4 Splat Setup (Bounding Rectangle)
    xmin = xmin * vp_scale[0] + vp_mid[0];
    xmax = xmax * vp_scale[0] + vp_mid[0];
    ymin = ymin * vp_scale[1] + vp_mid[1];
    ymax = ymax * vp_scale[1] + vp_mid[1];
    int sx = (int)floor (xmin), ex = (int)ceil (xmax);
    int sy = (int)floor (ymin), ey = (int)ceil (ymax);

    // Scisoring
    if( sx >= (int)canvas_width || ex <= 0 ) continue;
    if( ex - sx == 0 ) continue;
    if( sy >= (int)canvas_height || ey <= 0 ) continue;
    if( ey - sy == 0 ) continue;

    // Late view frustum culling, cuts part os splat outside buffer range
    sx = max(0, sx);
    ex = min(ex, (int)canvas_width - 1);
    sy = max(0, sy);
    ey = min(ey, (int)canvas_height - 1);
    
    // Depth Interpolation setup
    // -- page 47 : 3.6.4 Depth Values of the Resampling Filter
    double zmin = p0h.z();
    double zmax = p0h.z();
    double zx = 0.0, zy = 0.0;
    double zt = p0h.z();

    // -- page 49 (3.99)
    // since D = E = 0
    // l = sqrt ( F / (A*tuhz^2 + 2B*tuhz*tvhz + C*tvhz^2) )
    // compute divisor first and check if it is not zero
    double l = Q[0][0]*tuh.z()*tuh.z() + 2.0*Q[0][1]*tuh.z()*tvh.z() + Q[1][1]*tvh.z()*tvh.z();

    if ( l != 0.0 ) {
      // calculate zmin and zmax
      l = sqrt ( max (0.0, F/l) );
      zmin = p0h.z() - l * (tuh.z()*tuh.z() + tvh.z()*tvh.z()); 
      zmax = p0h.z() + l * (tuh.z()*tuh.z() + tvh.z()*tvh.z()); 
      // z coordinate at center of the conic, linear interpolationn -- page 48
      zt = (zmin + zmax) * 0.5;

      // Calculate the depth gradient, needed for surface reconstruction using z
      zx = -tvh.z() * M[0][1] + tuh.z() * M[1][1];
      zy =  tvh.z() * M[0][0] - tuh.z() * M[1][0];
      double dz = F*(zy*zy*Qrho[0][0] - 2*zy*zx*Qrho[0][1] + zx*zx*Qrho[1][1]);
      if ( dz != 0.0)
	dz = (zmax - zt) * sqrt (max(0.0, delta / dz));
      zx *= dz;
      zy *= dz;
    }

#ifdef DEBUG
    end_sh = glutGet(GLUT_ELAPSED_TIME);
    setup_time += (end_sh - sta_sh) / 1000.0;
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

    // Splat shader
    double color[3] = { 0.0, 1.0, 0.0};
    if (interpolate_normals) {
      Vector n = it->Normal();
      Vector rot_n (  n[0]*modelview[0][0] + n[1]*modelview[1][0] + n[2]*modelview[2][0],
		      n[0]*modelview[0][1] + n[1]*modelview[1][1] + n[2]*modelview[2][1],
		      n[0]*modelview[0][2] + n[1]*modelview[1][2] + n[2]*modelview[2][2]);
      color[0] = rot_n.x();
      color[1] = rot_n.y(); 
      color[2] = rot_n.z();
    }
    else {
      splatShader(it, modelview, color);
    }

#ifdef DEBUG
    end_sh = glutGet(GLUT_ELAPSED_TIME);
    shader_time += (end_sh - sta_sh) / 1000.0;
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif

    // Rasterization loop    
    for(int y = sy; y < ey; ++y) {
      for(int x = sx; x < ex; ++x) {
	uv[0] = ((double)x - vp_mid[0]) * oovp_scale[0] - xt;
	uv[1] = ((double)y - vp_mid[1]) * oovp_scale[1] - yt;
	
	radius = Qrho[0][0]*uv[0]*uv[0] + 2.0*Qrho[0][1]*uv[0]*uv[1] + Qrho[1][1]*uv[1]*uv[1];

	// Check if pixel is inside the elliptical splat
	// -- page 39 : 3.4.6 Central From of a Projective Mapped Conic (3.64)
	// radius = (x - xt) Qs (x - xt)_t <= F
	if( radius < F ) {

	  z = zt + uv[0] * zx + uv[1] * zy;
	  
	  pixelZ = zbuffer[x + y*canvas_width].z / zbuffer[x + y*canvas_width].weight;	 	  
	  
	  // Fragment near current surface -- Blend fragment with current surface
	  if (fabs(z - pixelZ) < it->Radius()) {
	    weight = exp_table[max(0, (int)floor(radius * exp_table_scale))]; // rasanem
	    pixel = zbuffer[x + y*canvas_width];

	    pixel.weight += weight;
	    pixel.r += color[0] * weight;
	    pixel.g += color[1] * weight;
	    pixel.b += color[2] * weight;
	    pixel.z += weight * z;
	  }
	  // Empty pixel or new surface in front of current surface -- replace with new fragment
	  else if ((!depth_test) || (z < pixelZ) )
	    {	      
	      weight = exp_table[max(0, (int)floor(radius * exp_table_scale))]; // rasanem
	      pixel = zbuffer[x + y*canvas_width];
	      
	      pixel.weight = weight;
	      pixel.r = color[0] * weight;
	      pixel.g = color[1] * weight;
	      pixel.b = color[2] * weight;
	      pixel.z = weight * z;
	  }
	  else
	    continue;
	  if ((x < (int)canvas_width) && (y < (int)canvas_height)) {
	    zbuffer[x + y*canvas_width] = pixel;
	  }
	}
      }
    }

#ifdef DEBUG
    end_sh = glutGet(GLUT_ELAPSED_TIME);
    raster_time += (end_sh - sta_sh) / 1000.0;
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
#endif
  }

#ifdef DEBUG
  if (clipped_surfs > 0)
    cout << "clipped : " << clipped_surfs << endl;

  double loop_end_sh = glutGet(GLUT_ELAPSED_TIME);
  cout << "loop time : " << (loop_end_sh - loop_sta_sh) / 1000.0 << endl;
  sta_sh = glutGet(GLUT_ELAPSED_TIME);

  cout << "culling : " << culling_time << endl;
  cout << "setup : " << setup_time << endl;
  cout << "shader : " << shader_time << endl;
  cout << "raster : " << raster_time << endl;
  cout << "surfs :  " << cnt_surfs << endl;

#endif

  // Render the framebuffer using a texture

  if (interpolate_normals) {
    double color[3] = {0.0, 0.0, 0.0};
    for (int i = 0; i < canvas_width * canvas_height; ++i) {
      double w = 1.0/zbuffer[i].weight;
      
      double n[3] = {zbuffer[i].r * w, zbuffer[i].g * w, zbuffer[i].b * w};
      if (zbuffer[i].z != HUGE)
	phongShader(n, color);
      else
	color[0] = color[1] = color[2] = 1.0;
      
      fb[i*3 + 0] = color[0];
      fb[i*3 + 1] = color[1];
      fb[i*3 + 2] = color[2];
    }
  }
  else {
    for (int i = 0; i < canvas_width * canvas_height; ++i) {
      double w = 1.0/zbuffer[i].weight;
      fb[i*3 + 0] = zbuffer[i].r * w;
      fb[i*3 + 1] = zbuffer[i].g * w;
      fb[i*3 + 2] = zbuffer[i].b * w;
    }    
  }
    
  glRasterPos2i(0, 0);
  glDrawPixels (canvas_width, canvas_height, GL_RGB, GL_FLOAT, &fb[0]);

  glFinish();

#ifdef DEBUG
  end_sh = glutGet(GLUT_ELAPSED_TIME);
  cout << "render time : " << (end_sh - sta_sh) / 1000.0 << endl << endl;
#endif


}
