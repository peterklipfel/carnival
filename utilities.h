#ifndef utilities
#define utilities
#include "CSCIx229.h"

typedef struct {int lamp; int one; int distance; int inc; int smooth; int local; 
       int emission; int ambient; int diffuse; int specular; int shininess;
       float shinyvec[1]; int zh; float ylight;} LightingStruct;

typedef struct {int axes; int th; int ph; int fov; int num_lights; int spokes;
        double asp; double dim; double rotation; double vel_division; int earthquake;} Globals;


static void ground(double x,  double y,  double z,
                   double dx, double dy, double dz,
                   double th1,double th2,double ph, 
                   int quads, LightingStruct lighting_struct, unsigned int texture)
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
   glRotated(th1,ph,1,th2);
   glScaled(dx,dy,dz);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture);

   //  passenger_box
   glNormal3f( 0,+1, 0);
   glBegin(GL_QUADS);
   int i, j;
   double mul = 2.0/quads;

   for (i=0;i<quads;i++)
   {
      for (j=0;j<quads;j++)
      {
         // printf("x: %f, y: %f ---- i: %d, j: %d \n", 5*mul*(i+0), 5*mul*(j+0)-5, i, j);
         glTexCoord2f(mul*(i+0),mul*(j+0)); glVertex3d(5*mul*(i+0)-5,0, 5*mul*(j+0)-5);
         glTexCoord2f(mul*(i+1),mul*(j+0)); glVertex3d(5*mul*(i+1)-5,0, 5*mul*(j+0)-5);
         glTexCoord2f(mul*(i+1),mul*(j+1)); glVertex3d(5*mul*(i+1)-5,0, 5*mul*(j+1)-5);
         glTexCoord2f(mul*(i+0),mul*(j+1)); glVertex3d(5*mul*(i+0)-5,0, 5*mul*(j+1)-5);
      }
   }

   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);

};

static void Vertex(double th,double ph)
{
   // glVertex3d(Sin(th)*Cos(ph) , Sin(ph) , Cos(th)*Cos(ph));
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glTexCoord2d((90-th)/360.0+90,ph/360.0+0.5);
   glVertex3d(x,y,z);
};

static void light(double x,double y,double z,double r, double start_angle, double end_angle)
{
   const int d=5;
   int th,ph;

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,y,z);
   glScaled(r,r,r);
   glColor3f(sin(10*x), sin(7*x+3*y), cos(y*y+x*4));
   //  Latitude bands
   for (ph=start_angle;ph<end_angle;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=90;th<=270;th+=d)
      {
         Vertex(th,ph);
         Vertex(th,ph+d);
      }
      glEnd();
   }
   //  Undo transformations
   glPopMatrix();
};

static void sky(double x,double y,double z,double r, double start_angle, 
                double end_angle)
{
   const int d=5;
   int th,ph;

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,y,z);
   glScaled(r,r,r);
   glColor3f(0, .75, 1);
   //  Latitude bands
   for (ph=start_angle;ph<end_angle;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=90;th<=270;th+=d)
      {
         Vertex(th,ph);
         Vertex(th,ph+d);
      }
      glEnd();
   }
   //  Undo transformations
   glPopMatrix();
};

#endif

