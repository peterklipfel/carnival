/*
 *  Projections
 *  'm' to switch modes (projections)
 *  'a' to toggle globals.axes
 *  '0' snaps angles to 0,0
 *  arrows to rotate the world
 *  PgUp/PgDn zooms in/out
 *  +/- changes field of view of rperspective
 */
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdarg.h>
// #include <math.h>
#include "utilities.h"
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
// #include <GL/glut.h>


//Lighting
Globals globals;
LightingStruct lighting_struct;
void initialize(){
   globals.axes=0;       //  Display globals.axes
   globals.th=0;         //  Azimuth of view angle
   globals.ph=0;         //  Elevation of view angle
   globals.fov=55;       //  Field of view (for perspective)
   globals.num_lights=0;
   globals.spokes = 5;
   globals.asp=1;     //  Aspect ratio
   globals.dim=7.0;   //  Size of world
   globals.rotation = 0;
   globals.vel_division = 900000.0;
   globals.earthquake = 0;

   lighting_struct.lamp      =   1;
   lighting_struct.one       =   1;  // Unit value
   lighting_struct.distance  =   5;  // Light lighting_struct.distance
   lighting_struct.inc       =  10;  // Ball increment
   lighting_struct.smooth    =   1;  // Smooth/Flat shading
   lighting_struct.local     =   0;  // Local Viewer Model
   lighting_struct.emission  =   0;  // Emission intensity (%)
   lighting_struct.ambient   =   0;  // Ambient intensity (%)
   lighting_struct.diffuse   = 100;  // Diffuse intensity (%)
   lighting_struct.specular  =   0;  // Specular intensity (%)
   lighting_struct.shininess =   0;  // Shininess (power of two)
   lighting_struct.zh        =  90;  // Light azimuth
   lighting_struct.ylight  =   3;  // Elevation of light


}
// Textures
unsigned int texture[20];
int texture_num = 0;

//shadows
#define Dfloor  40
#define Yfloor -0.95
float N[] = {0, -1, 0}; // Normal vector for the plane
float E[] = {0, Yfloor, 0 }; // Point of the plane


//  Macro for sin & cos in degrees
// #define Cos(th) cos(3.1415927/180*(th))
// #define Sin(th) sin(3.1415927/180*(th))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters lighting_struct.one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

void ShadowProjection(float L[4], float E[4], float N[4])
{
   float mat[16];
   float e = E[0]*N[0] + E[1]*N[1] + E[2]*N[2];
   float l = L[0]*N[0] + L[1]*N[1] + L[2]*N[2];
   float c = e - l;
   //  Create the matrix.
   mat[0] = N[0]*L[0]+c; mat[4] = N[1]*L[0];   mat[8]  = N[2]*L[0];   mat[12] = -e*L[0];
   mat[1] = N[0]*L[1];   mat[5] = N[1]*L[1]+c; mat[9]  = N[2]*L[1];   mat[13] = -e*L[1];
   mat[2] = N[0]*L[2];   mat[6] = N[1]*L[2];   mat[10] = N[2]*L[2]+c; mat[14] = -e*L[2];
   mat[3] = N[0];        mat[7] = N[1];        mat[11] = N[2];        mat[15] = -l;
   //  Multiply modelview matrix
   glMultMatrixf(mat);
}


static void outer_frame(double center_x, double center_y, 
                        double center_z, double r, 
                        double segments, double step)
{
   int i;
   glBegin(GL_LINE_STRIP);
   glColor3f(1.0, 0.7, 0.5);
      for (i=0;i<segments;i++)
      {
         double x = r * cos(i*step+globals.rotation);
         double y = r * sin(i*step+globals.rotation);
         double x2 = r * cos((i+1)*step+globals.rotation);
         double y2 = r * sin((i+1)*step+globals.rotation);
         glVertex3f(x, y, center_z);
         glVertex3f(x2, y2, center_z); //close the circle
      }
   glEnd(); 
}

static void beam(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th1, double th2, double ph, double ph2,
                 unsigned int texture)
{
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*lighting_struct.emission,1.0};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,lighting_struct.shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th1,ph,ph2,th2);
   glScaled(dx,dy,dz);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   // glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture);

   //  passenger_box
   glBegin(GL_QUADS);
   //  Front
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   //  Right
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
   //  Left
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   //  Top
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   //  Bottom
   glNormal3f( 0,-lighting_struct.one, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*lighting_struct.emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glMaterialfv(GL_FRONT,GL_SHININESS,lighting_struct.shinyvec);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texture[2]);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=lighting_struct.inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*lighting_struct.inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+lighting_struct.inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

// static void cylinder(double x,double y,double z,double r, double height)

static void person(double x, double y , double z, double r, double height)
{
   ball(z, y, z, r);
   height += 1; // no unused variable warning
   // int h,ph;
   // float yellow[] = {1.0,1.0,0.0,1.0};
   // float Emission[]  = {0.0,0.0,0.01*lighting_struct.emission,1.0};
   // //  Save transformation
   // glPushMatrix();
   // //  Offset, scale and rotate
   // glTranslated(x,y,z);
   // glScaled(r,r,r);
   // //  White ball
   // glMaterialfv(GL_FRONT,GL_SHININESS,lighting_struct.shinyvec);
   // glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   // glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   // //  Bands of latitude
   // for (ph=-90;ph<90;ph+=lighting_struct.inc)
   // {
   //    glBegin(GL_QUAD_STRIP);
   //    for (h = -height/2; h < height/2; h+=lighting_struct.inc)
   //    {
   //       glNormal3d(Cos(ph),Sin(ph),h);
   //       glVertex3d(Cos(ph),Sin(ph),h);
   //       glNormal3d(Cos(ph+lighting_struct.inc),Sin(ph+lighting_struct.inc),h+lighting_struct.inc);
   //       glVertex3d(Cos(ph+lighting_struct.inc),Sin(ph+lighting_struct.inc),h+lighting_struct.inc);
   //       // Vertex(th,ph);
   //       // Vertex(th,ph+lighting_struct.inc);
   //    }
   //    glEnd();
   // }
   // //  Undo transofrmations
   // glPopMatrix();

}

static void passenger_box(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th, unsigned int texture)
{
   //  Set lighting_struct.specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*lighting_struct.emission,1.0};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,lighting_struct.shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);  
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   person(0, +1.5, 0, 0.3, 0.5);
   // Textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture);
   //  passenger_box
   // ball(0, +1.5, 0, 0.5);
   //  Front
   // glBindTexture(GL_TEXTURE_2D,texture[0]);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   glEnd();
   //  Back
   // glBindTexture(GL_TEXTURE_2D,texture[0]);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   glEnd();
   //  Right
   // glBindTexture(GL_TEXTURE_2D,texture[0]);
   glBegin(GL_QUADS);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
   glEnd();
   //  Left
   // glBindTexture(GL_TEXTURE_2D,texture[0]);
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Bottom
   // glBindTexture(GL_TEXTURE_2D,texture[0]);
   glBegin(GL_QUADS);
   glNormal3f( 0,-lighting_struct.one, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   glEnd();
   //  End
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

static void ferris_wheel(double x, double y , double z, double size)
{
   double pi = 3.14159265358979323846;
   double step = 2*pi/10;
   double r = 3;
   int i;
   //  Draw passenger_boxes
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(size,size,size);
   for (i=0;i<10;i++)
   {
      double passenger_box_x = r * cos(i*step+globals.rotation);
      double passenger_box_y = r * sin(i*step+globals.rotation);
      passenger_box(passenger_box_x, passenger_box_y,0 , 0.3,0.3,0.3 , 0, texture[7]);
   }

   // Draw Circle
   outer_frame(0, 0, -0.31, r, 10, step);
   outer_frame(0, 0, 0.31, r, 10, step);


   // Draw the globals.spokes
   glColor3f(0.5,0.5,0.5);
   double conversion = 180/pi;
   int current_spoke;
   int num_spokes = globals.spokes;

   double offset = 0;
   for(current_spoke = 0; current_spoke < num_spokes; current_spoke++)
   {
      glColor3f(0.8,0.8,0.8);
      offset = current_spoke*(180/num_spokes);
      beam(0,0,-0.3, r-0.1, 0.1, 0.1,offset + conversion*globals.rotation, offset+conversion*globals.rotation, 0, 1, texture[9]);
      beam(0,0,0.3, r-0.1, 0.1, 0.1,offset + conversion*globals.rotation, offset+conversion*globals.rotation, 0, 1, texture[9]);
   }

   // center axis
   glColor3f(0.5,0.5,0.5);
   beam(0,0,0, 1, 0.1, 0.1, 90, 0, 0, 1, texture[7]);

   // braces
   glColor3f(0.5,0.5,0.5);
   beam(1.7,-1.7,-1, 2.5, 0.1, 0.1, 135, 135, 0, 1, texture[7]);
   beam(-1.7,-1.7,-1, 2.5, 0.1, 0.1, 45, 45, 0, 1, texture[7]);
   beam(1.7,-1.7,1, 2.5, 0.1, 0.1, 135, 135, 0, 1, texture[7]);
   beam(-1.7,-1.7,1, 2.5, 0.1, 0.1, 45, 45, 0, 1, texture[7]);


   if(globals.num_lights){
      int spacing;
      r = r-0.1;
      num_spokes = num_spokes*2;
      double x_angle, y_angle, radius;
      for(current_spoke = 0; current_spoke < num_spokes; current_spoke++)
      {
         offset = current_spoke*2*pi/num_spokes;
         for(spacing = 1; spacing < globals.num_lights+1; spacing++)
         {
            // light(spacing*r*cos(offset + globals.rotation)/globals.num_lights, 
            //       spacing*r*sin(offset + globals.rotation)/globals.num_lights, 0.35, 0.1, 90, 270);
            radius = spacing*r/globals.num_lights;
            x_angle = cos(offset + globals.rotation);
            y_angle = sin(offset + globals.rotation);

            light(radius*x_angle, radius*y_angle, 0.35, 0.1, 0, 180);
            light(radius*x_angle, radius*y_angle, -0.35, 0.1, -180, 0);
         }
      }
   }
   glPopMatrix();
}

static void scrambler(double x, double y , double z, double size) {
   int num_spokes = globals.spokes % 2 ? globals.spokes + 1 : globals.spokes;
   double pi = 3.14159265358979323846;
   double step = pi/num_spokes;
   double r = 4;
   int i;
   //  Draw passenger_boxes
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(size,size,size);

   // Draw the globals.spokes
   glColor3f(0.5,0.5,0.5);
   double conversion = 180/pi;
   int current_spoke;
   int speed = 15;
   double spin = globals.rotation*speed;
   double offset = 0;
   for(current_spoke = 0; current_spoke < num_spokes; current_spoke++)
   {
      offset = current_spoke*(180/num_spokes);
      beam(0,2.75,0, 4, 0.15, 0.15,offset + conversion*spin, 0, 0, 90, texture[0]);
   }
   for (i=0;i<num_spokes*2;i++)
   {
      glColor3f(1,1,1);
      double passenger_box_x = r * sin(i*step+spin);
      double passenger_box_z = r * cos(i*step+spin);
      passenger_box(passenger_box_x, 1, passenger_box_z , 0.3,0.3,0.3 , 0, texture[5]);
      beam(passenger_box_x,2.2,passenger_box_z, 0.05, .4, 0.05, 90, 0, 0, 1, texture[1]);

      beam(passenger_box_x-0.3,1.4,passenger_box_z, 0.01, .4, 0.01, 90, 0, 0, 1, texture[1]);
      beam(passenger_box_x+0.3,1.4,passenger_box_z, 0.01, .4, 0.01, 90, 0, 0, 1, texture[1]);
      beam(passenger_box_x,1.4,passenger_box_z-0.3, 0.01, .4, 0.01, 90, 0, 0, 1, texture[1]);
      beam(passenger_box_x,1.4,passenger_box_z+0.3, 0.01, .4, 0.01, 90, 0, 0, 1, texture[1]);
    
      beam(passenger_box_x,1.9,passenger_box_z, 0.4, 0.05, 0.05, 90, 0, 0, 1, texture[1]);
      beam(passenger_box_x,1.9,passenger_box_z, 0.05, 0.05, 0.4, 90, 0, 0, 1, texture[1]);

      if(globals.num_lights){
         int spacing;
         double x_angle, z_angle, radius, offset2;
     
         offset2 = i*pi/num_spokes;
         for(spacing = 1; spacing < globals.num_lights+1; spacing++)
         {
            radius = spacing*r/globals.num_lights - 0.1;
            x_angle = sin(offset2 + spin);
            z_angle = cos(offset2 + spin);

            light(radius*x_angle, 2.9, radius*z_angle, 0.1, 0, 360);
         }
      }
   }

   // center axis
   glColor3f(1,1,1);
   beam(0,0,0, 1, 1.5, 1, 90, 0, 0, 1, texture[5]);
   beam(0, 2.75, 0, 0.5, 1.5, 0.5, speed*conversion*globals.rotation, 0, 0, 90, texture[1]);

   glPopMatrix();
}

static void tower(double x, double y, double z, double size)
{
   int num_spokes = 4;
   double pi = 3.14159265358979323846;
   double r = 4.1;
   int i;
   //  Draw passenger_boxes
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(size,size,size);

   // Draw the globals.spokes
   glColor3f(0.5,0.5,0.5);
   int speed = 5;

   double height_osc = -sin(globals.rotation*speed)*14;
   height_osc = height_osc < -13 ? -13 : height_osc;

   glColor3f(1, 1, 1);
   cylinder(0, 14, 0, 5, 0, 0, 0, 0, 0, 360, 0.3, 3, 0.3, texture[11]);
   double x_angle, z_angle, radius, offset2, spin;
   spin = globals.rotation*speed;
   x_angle = sin(globals.rotation*speed);
   z_angle = cos(globals.rotation*speed);
   
   glColor3f(1, 1, 1);
   cylinder(0, height_osc + 14, 0, 1, spin*20*pi, 0, spin*speed*20*pi, 0, 0, 360, 4, 1, 4, texture[4]);
   // cylinder(0, height_osc + 14, 0, 1, x_angle*180, 0, 0, 0, 0, 360, 4, 1, 4, texture[2]);

   if(globals.num_lights){
      for (i=0;i<num_spokes*2;i++)
      {
         int spacing;
     
         offset2 = i*pi/num_spokes;
         x_angle = sin(offset2 + spin);
         z_angle = cos(offset2 + spin);
         for(spacing = 1; spacing < globals.num_lights+1; spacing++)
         {
            radius = spacing*r/globals.num_lights - 0.1;

            light(radius*x_angle, height_osc + 13, radius*z_angle, 0.1, 0, 360);
            light(radius*x_angle, height_osc + 15, radius*z_angle, 0.1, 0, 360);
         }
      }
   }

   glPopMatrix();
}

static void hut(double x, double y, double z, double size)
{
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(size,size,size);

   glColor3f(1,1,1);

   cone(0, 0.7, 0, 1.5, 90, -90, 0, 0, 0, 360, 1, 1, 1, texture[2]);
// cylinder(double x, double y, double z, double r,
//                   double th, double thX, double thY, double thZ,
//                   double thStart, double thEnd,
//                   double dx, double dy, double dz, unsigned int texture)

   glColor3f(1,1,1);
   cylinder(0, 0, 0, 1, 0, 0, 0, 0, 0, 310, 1, 0.7, 1, texture[10]);

   glPopMatrix();
}

static void draw_lamp(){
   glShadeModel(GL_SMOOTH);
      //  Translate intensity to color vectors
   float Ambient[]   = {0.01*lighting_struct.ambient ,0.01*lighting_struct.ambient ,0.01*lighting_struct.ambient ,1.0};
   float Diffuse[]   = {0.01*lighting_struct.diffuse ,0.01*lighting_struct.diffuse ,0.01*lighting_struct.diffuse ,1.0};
   float Specular[]  = {0.01*lighting_struct.specular,0.01*lighting_struct.specular,0.01*lighting_struct.specular,1.0};
   //  Light position
   float Position[]  = {lighting_struct.distance*Cos(lighting_struct.zh),lighting_struct.ylight,lighting_struct.distance*Sin(lighting_struct.zh),1.0};
   //  Draw light position as ball (still no lighting here)
   glColor3f(0,0,0);
   ball(Position[0],Position[1],Position[2] , 0.1);
   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Enable lighting
   glEnable(GL_LIGHTING);
   //  Location of viewer for lighting_struct.specular calculations
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,lighting_struct.local);
   //  Two sided mode
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);
   //  glColor sets lighting_struct.ambient and lighting_struct.diffuse color materials
   glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   //  Enable light 0
   glEnable(GL_LIGHT0);
   //  Set lighting_struct.ambient, lighting_struct.diffuse, lighting_struct.specular complighting_struct.onents and position of light 0
   glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
   glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
   glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
   glLightfv(GL_LIGHT0,GL_POSITION,Position);
   //  Set spotlight parameters
   glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,180);
   glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,0);
   //  Set attenuation
   glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION ,2.0);
   glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION   ,0.0);
   glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0);
}

void scene()
{
   int damping = 10;
   double ferris1x = damping*10;
   double ferris2x = damping*-15;
   double scrambler1x = damping*19;
   double scrambler2x = damping*-14;
   if (globals.earthquake)
   {
      ferris1x = ferris1x+sin(500*globals.rotation);
      ferris2x = ferris2x+cos(630*globals.rotation);

      scrambler1x = scrambler1x+sin(520*globals.rotation);
      scrambler2x = scrambler2x+sin(604*globals.rotation);
   }

   ferris_wheel(ferris1x/damping, 2.4, -20, 1);
   ferris_wheel(ferris2x/damping, 2.4, 0, 1);

   scrambler(scrambler1x/damping, 0, 5, 1);
   // scrambler(0, 0, 0, 1);
   scrambler(scrambler2x/damping, 0, 15, 1);

   tower(0, -1, -20, 0.75);

   hut(30, 0, 10, 1);
   hut(-10, 0, 10, 1);
   hut(-30, 0, -15, 1);
   hut(-10, 0, -26, 1);
   
   hut(13, 0, -6, 1);
   hut(6, 0, 18, 1);
   hut(1, 0, -6, 1);
   hut(-17, 0, 20, 1);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=1.5;  //  Length of globals.axes
   float Position[]  = {lighting_struct.distance*Cos(lighting_struct.zh),lighting_struct.ylight,lighting_struct.distance*Sin(lighting_struct.zh),1.0};

   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   double Ex = -2*globals.dim*Sin(globals.th)*Cos(globals.ph);
   double Ey = +2*globals.dim        *Sin(globals.ph);
   double Ez = +2*globals.dim*Cos(globals.th)*Cos(globals.ph);
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(globals.ph),0);

   //  Flat or lighting_struct.smooth shading
   glShadeModel(lighting_struct.smooth ? GL_SMOOTH : GL_FLAT);

   // Light switch
   if (lighting_struct.lamp)
   {
      draw_lamp();   
   }
   else
     glDisable(GL_LIGHTING);

   sky(-1,-1,-1,40, 90, 270);
   ground(-1, -1, -1, 10, 0, 10, 0, 0, 0, 80, lighting_struct, texture[3]);

   scene();

   //shadows
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_LIGHTING);
   //  Enable stencil operations
   glEnable(GL_STENCIL_TEST);

   /*
    *  Step 1:  Set stencil buffer to 1 where there are shadows
    */
   //  Existing value of stencil buffer doesn't matter
   glStencilFunc(GL_ALWAYS,1,0xFFFFFFFF);
   //  Set the value to 1 (REF=1 in StencilFunc)
   //  only if Z-buffer would allow write
   glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
   //  Make Z-buffer and color buffer read-only
   glDepthMask(0);
   glColorMask(0,0,0,0);
   //  Draw flattened scene
   glPushMatrix();
   ShadowProjection(Position,E,N);
   scene();
   glPopMatrix();
   //  Make Z-buffer and color buffer read-write
   glDepthMask(1);
   glColorMask(1,1,1,1);

   /*
    *  Step 2:  Draw shadow masked by stencil buffer
    */
   //  Set the stencil test draw where stencil buffer is > 0
   glStencilFunc(GL_LESS,0,0xFFFFFFFF);
   //  Make the stencil buffer read-only
   glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
   //  Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glColor4f(0,0,0,0.5);
   //  Draw the shadow over the entire floor
   glBegin(GL_QUADS);
   glVertex3f(-Dfloor,Yfloor,-Dfloor);
   glVertex3f(+Dfloor,Yfloor,-Dfloor);
   glVertex3f(+Dfloor,Yfloor,+Dfloor);
   glVertex3f(-Dfloor,Yfloor,+Dfloor);
   glEnd();
   glPopAttrib();

   //  Draw globals.axes
   glColor3f(1,1,1);
   if (globals.axes)
   {
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label globals.axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f FOV=%d vel=%.5f",globals.th,globals.ph,globals.dim,globals.fov,10000000/globals.vel_division);
   
   if (lighting_struct.lamp)
   {
      glWindowPos2i(5,45);
      Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f Rotation=%.5f",lighting_struct.smooth?"Smooth":"Flat",lighting_struct.local?"On":"Off",lighting_struct.distance,lighting_struct.ylight, globals.rotation);
      glWindowPos2i(5,25);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",lighting_struct.ambient,lighting_struct.diffuse,lighting_struct.specular,lighting_struct.emission,lighting_struct.shinyvec[0]);
   }   

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      globals.th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      globals.th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      globals.ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      globals.ph -= 5;
   //  PageUp key - increase globals.dim
   else if (key == GLUT_KEY_PAGE_UP)
      globals.dim += 0.1;
   //  PageDown key - decrease globals.dim
   else if (key == GLUT_KEY_PAGE_DOWN && globals.dim>1)
      globals.dim -= 0.1;
   //  Keep angles to +/-360 degrees
   globals.th %= 360;
   globals.ph %= 360;
   //  Update projection
   Project(45, globals.asp, globals.dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == 'r' || ch == 'R')
   {
      globals.th = globals.ph = 0;
      globals.fov=55;
      globals.num_lights=1;
      globals.asp=1;     //  Aspect ratio
      globals.dim=5.0;
   }
   //  Toggle globals.axes
   else if (ch == 'g' || ch == 'G')
      globals.axes = 1-globals.axes;
   //  Change field of view angle
   else if (ch == '-' && ch>1)
      globals.fov--;
   else if (ch == '+' && ch<179)
      globals.fov++;
   else if (ch == 'l')
   {
      if (globals.num_lights > 0)
      {
         globals.num_lights = globals.num_lights - 1;
      }
      else{
         globals.num_lights = 0;
      }
   }
   else if (ch == 'L')
      globals.num_lights = globals.num_lights + 1;
   else if (ch == 's')
      if (globals.spokes > 1){
         globals.spokes = globals.spokes - 1;
      }
      else{
         globals.spokes = 1;
      }
   else if (ch == 'S')
   {
      if (globals.spokes < 6){
         globals.spokes++;
      }
      else{
         globals.spokes = 6;
      }
   }
   else if (ch == 'V')
      if(globals.vel_division > 50000.0){
         globals.vel_division -= 50000.0;
      }
      else {
         globals.vel_division = 1;
      }
   else if (ch == 'v')
      globals.vel_division += 50000.0;
   else if (ch == 't' || ch == 'T')
      globals.earthquake = 1 - globals.earthquake;
   else if (ch == 'B' || ch == 'b')
      lighting_struct.lamp = 1 - lighting_struct.lamp;
      else if (ch=='a' && lighting_struct.ambient>0)
      lighting_struct.ambient -= 5;
   else if (ch=='A' && lighting_struct.ambient<100)
      lighting_struct.ambient += 5;
   //  Diffuse level
   else if (ch=='d' && lighting_struct.diffuse>0)
      lighting_struct.diffuse -= 5;
   else if (ch=='D' && lighting_struct.diffuse<100)
      lighting_struct.diffuse += 5;
   //  Specular level
   else if (ch=='c' && lighting_struct.specular>0)
      lighting_struct.specular -= 5;
   else if (ch=='C' && lighting_struct.specular<100)
      lighting_struct.specular += 5;
   //  Emission level
   else if (ch=='e' && lighting_struct.emission>0)
      lighting_struct.emission -= 5;
   else if (ch=='E' && lighting_struct.emission<100)
      lighting_struct.emission += 5;
   //  Shininess level
   else if (ch=='n' && lighting_struct.shininess>-1)
      lighting_struct.shininess -= 1;
   else if (ch=='N' && lighting_struct.shininess<7)
      lighting_struct.shininess += 1;
   else if (ch=='q')
      texture_num = (texture_num+1) % 12;
   else if (ch=='Q')
      texture_num = ((texture_num < 0 ? -texture_num : texture_num) - 1)%12;
   //  Translate lighting_struct.shininess power to value (-1 => 0)
   lighting_struct.shinyvec[0] = lighting_struct.shininess<0 ? 0 : pow(2.0,lighting_struct.shininess);

   //  Reproject
   Project(45, globals.asp, globals.dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   globals.asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(45,globals.asp,globals.dim);
}

void idle()
{
   //  Get elapsed (wall) time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/globals.vel_division;
   double t2 = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   //  Calculate spin angle 90 degrees/second
   globals.rotation = fmod(90*t,360);
   lighting_struct.zh = fmod(90*t2,360.0);
   //  Request display update
   glutPostRedisplay();
}


/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //initialize globals
   initialize();
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(1920,1080);
   glutCreateWindow("Ferris Wheel");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   texture[0] = LoadTexBMP("textures/crate.bmp");
   texture[1] = LoadTexBMP("textures/metal1.bmp");
   texture[2] = LoadTexBMP("textures/face.bmp");
   texture[3] = LoadTexBMP("textures/grass.bmp");
   texture[4] = LoadTexBMP("textures/glass.bmp");
   texture[5] = LoadTexBMP("textures/hazard.bmp");
   texture[6] = LoadTexBMP("textures/faded.bmp");
   texture[7] = LoadTexBMP("textures/rwStripe.bmp");
   texture[8] = LoadTexBMP("textures/trippy.bmp");
   texture[9] = LoadTexBMP("textures/fabric/1.bmp");
   texture[10] = LoadTexBMP("textures/wood/2.bmp");
   texture[11] = LoadTexBMP("textures/rwStripe2.bmp");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
