/*****Visual interface global vars*****/
surfelVectorIter selected_surfel;
int selected_point_id;

double reconstruction_filter_size;
double prefilter_size;

GLint render_mode;
bool show_kd_tree;
bool show_points;
int show_splats;
bool show_screen_info;

bool elliptical_weight;
bool depth_culling;
bool rotating;
bool color_model;

int button_pressed;
bool active_shift;

int analysis_filter_size;

int output_type;
const int num_output_types = 9;

double max_radius;

// Frames per sencond and Surfels per second
double fps, sps;
int fps_loop;
double start_time, end_time;
int timing_profile;

/*************************************/

/// OpenGL Write text to screen
/// @param x X screen coordinat
/// @param y Y screen coordinate
/// @param str Text to write to screen
void glWrite(GLdouble x, GLdouble y, char *str) {
  glRasterPos2d(x, y);
  for (char *s = str; *s; ++s)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);
}

/// Screen text with commands info
void screenText(int w, int h) {

  GLfloat text_normal_color[4] = {0.1, 0.1, 0.1, 1.0};
  GLfloat text_highlight_color[4] = {0.0, 0.0, 1.0, 1.0};

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0,0, w, h);

  gluOrtho2D(-2.0, 2.0, -2.0, 2.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4fv(text_normal_color);
  ostringstream fps_text, sps_text, num_pts_text;
  fps_text << "FPS : " << fps;
  sps_text << "Surfels per sec : " << sps << "K";
  
  num_pts_text << "number of points : " << number_surfels << endl;
  glWrite(1.1, 1.60, (char*) num_pts_text.str().c_str());
  glWrite(1.1, 1.45, (char*) fps_text.str().c_str());
  glWrite(1.1, 1.25, (char*) sps_text.str().c_str());
  
 if (show_screen_info) {
    ostringstream show_screen_info_text;
    show_screen_info_text << "(h) show info/help : ";
    if (show_screen_info)
      show_screen_info_text << "on";
    else
      show_screen_info_text << "off";
    
    
    glWrite(-1.8, 1.55, (char*) show_screen_info_text.str().c_str());    

 

    ostringstream reconstruction_filter_text, reconstruction_filter_change_text, 
      prefilter_text, prefilter_change_text,
      left_button_text, middle_button_text, right_button_text;

    reconstruction_filter_text << "reconstruction filter size : " << reconstruction_filter_size;
    reconstruction_filter_change_text << "+/- keys";
    
    prefilter_text << "prefilter size : " << prefilter_size;
    prefilter_change_text << "(+SHIFT) +/- keys";

    glColor4fv(text_highlight_color);
    glWrite(0.7, -1.65, (char*) reconstruction_filter_text.str().c_str());
    glColor4fv(text_normal_color);
    glWrite(1.5, -1.65, (char*) reconstruction_filter_change_text.str().c_str());

    glColor4fv(text_highlight_color);
    glWrite(0.7, -1.75, (char*) prefilter_text.str().c_str());
    glColor4fv(text_normal_color);
    glWrite(1.5, -1.75, (char*) prefilter_change_text.str().c_str());


    left_button_text << "left button : rotate object";
    middle_button_text << "middle button : zoom in/out / (+SHIFT) translate object";
    right_button_text << "right button : translate light position";

    glWrite(-1.8, -1.60, (char*) left_button_text.str().c_str());
    glWrite(-1.8, -1.70, (char*) middle_button_text.str().c_str());
    glWrite(-1.8, -1.80, (char*) right_button_text.str().c_str());

    ostringstream view_mode_text, show_points_text, ppr_text, ewa_text;

    view_mode_text << "(v) view mode : ";
    if (camera->viewMode() == PERSPECTIVE)
      view_mode_text << "perspective";
    else
      view_mode_text << "orthogonal";

    show_points_text << "(p) render points : ";
    if (show_points)
      show_points_text << "on";
    else
      show_points_text << "off";

    ppr_text << "(1) Pyramid Point Render : ";
    if ((show_splats == 1) || (show_splats == 2))
      ppr_text << "on";
    else
      ppr_text << "off";

    ewa_text << "(2) EWA Surface Splatting : ";
    if ((show_splats == 3) || (show_splats == 4))
      ewa_text << "on";
    else
      ewa_text << "off";

    glWrite(-1.8, 1.45, (char*) view_mode_text.str().c_str());
    //glWrite(-1.8, 1.75, (char*) show_points_text.str().c_str());
    //glWrite(-1.8, 1.65, (char*) ppr_text.str().c_str());
    //glWrite(-1.8, 1.55, (char*) ewa_text.str().c_str());

  }


    //if (show_splats)
    //glWrite(0.5, 1.65, (char*) sps_text.str().c_str());


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  camera->resetViewMode ();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/// Resolve click in one of the screen flag buttons
/// @param click World position of clicked mouse
void screenButtons (int x, int y) {

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glViewport(0,0, 1024, 1024);
  //  glOrtho( -2.0, 2.0, -2.0, 2.0, -2.0, 2.0 );
  gluOrtho2D( -2.0, 2.0, -2.0, 2.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Returns the unprojected point in ortho that text was writen
  Point click = unproject(Point (x, y, 0.0));

  camera->resetViewMode ();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  double h_click = 1.87; // help
  double v_click = 1.77; // view mode
//   double p_click = 1.77; // show points
//   double s_click = 1.67; // show splats
  double t = 0.025;
  char key = '0';

  // X coordinate must be between -1.26 and -1.22
  if ((click.x() >= -1.8) && (click.x() <= -1.73)) {

    if ((click.y() >= h_click - t ) && (click.y() <= h_click + t)) 
      key = 'h';
    if ((click.y() >= v_click - t ) && (click.y() <= v_click + t)) 
      key = 'v';
//     if ((click.y() >= p_click - t ) && (click.y() <= p_click + t)) 
//       key = 'p';
//     if ((click.y() >= s_click - t ) && (click.y() <= s_click + t)) 
//       key = 's';
  }

  if (key != '0')
    keyboard (key, x, y);
}
