#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./Engine/object.h"
//#include "../Engine/World.h"
//#include "../Network/Player.h"

extern "C"
{
  //#include <stdio.h>
  //#include <stdlib.h>
#include "./lib/board.h"
#include "./lib/SSP.h"
#include "./lib/ADXL345.h"
#include "./lib/OLED.h"
}

Matrix worldToCamera=Matrix(),
	   perspectiveProjection=Matrix(),
	   projectionToPixel=Matrix();

//Player_c player;

std::list<Object> objectlist;		// used for the static world objects
std::list<Object> dynamiclist;		// used for dynamic objects received from server
std::list<Poly> renderlist;			// aggregate polygon list to be rendered


Poly testpoly = Poly(Vertex(-6.666,6.666,0,1,0,0),
                                 Vertex(13.333,6.666,0,1,1,0),
                                 Vertex(-6.666,-13.333,0,1,0,1),
                                 Vertex(),3,Vector3D(0,0,-1),TEXTURED);

Object testobj = Object(testpoly,floortexsmall,floortexsmallwidth,floortexsmallheight,Vector3D(),Point3D(0,0,15));

const double size = 5;
Poly mypoly = Poly( Vertex(size, size, -size,1,1,0), Vertex(size, -size, -size,1,1,1),Vertex(-size, size, -size,1,0,0),
						   Vertex(),3,Vector3D(0,0,-1),TEXTURED);

void bootup(void);
void PllFeed(void);

extern "C" int    main( void );

extern "C" void abort(void)
{
  while (1 == 1)
  {}
}

void ReadInput(short *x, short *y)
{
      #define MAX_SPEED   10
      #define JUMP_SPEED  25
      int read_val = (~FIO0PIN)&(0x7<<SWITCH_UP);
      switch (read_val)
      {
      case (1<<SWITCH_DOWN):
           *x = (abs(*x)>MAX_SPEED) ? -MAX_SPEED:(*x-1);
           break;
      case (1<<SWITCH_UP):
           *x = (abs(*x)>MAX_SPEED) ? MAX_SPEED:(*x+1);
           break;
      case (1<<SWITCH_ENTER):
           break;
      default:
           *x = 0;
      }
      if (accelZ()>((GRAVITY)*8/10)) *y = -JUMP_SPEED;
}

int main( void )
{
    bootup();
    OLED_init();
    
    // Structure of a transformation matrix:
    // ( r=rotation, p=projection, t=translation )
    // [ x y z w ]	| r r r p | = [ x' y' z' w']
    //				| r r r p |
    //				| r r r p |
    //				| t t t s |
    //				or 
    // | r r r t | | x | = | x' |
    // | r r r t | | y |   | y' |
    // | r r r t | | z |   | z' |
    // | p p p s | | w |   | w' |

    // We use ROW vector notation
            
    perspectiveProjection.data[3][2] = -1;	// translate z
    perspectiveProjection.data[2][3] = 1;	// project z

    projectionToPixel.data[3][0] = (float)SIZE_X*0.5;	// translate x
    projectionToPixel.data[3][1] = (float)SIZE_Y*0.5;	// translate y
    projectionToPixel.data[0][0] = (float)SIZE_X*0.5;	// scale x
    projectionToPixel.data[1][1] = (float)SIZE_Y*0.5;	// scale y

//    player.Level(-1);
//    objectlist = player.Level().GetRenderList();

    mypoly.SetTexture(boxtexture, boxtexturewidth, boxtextureheight);
    mypoly.SetColor(rand()/(float)RAND_MAX,rand()/(float)RAND_MAX,rand()/(float)RAND_MAX);
    mypoly.SetVertexColors(0.9,0,0,
            0,0.9,0,
            0,0,0.9);

//    testobj.GenerateFloor(50,-5);
//    testobj.GenerateShot(Vector3D(-2,0,5),3.141/2,0);
//    testobj.add(mypoly);

    while (1)
    {
        renderlist.clear();

//	for (std::list<Object>::iterator it = objectlist.begin(); it != objectlist.end(); it++) {
//		it->updateList();
//		Vector3D tmppos = it->GetPosition() - player.Eye().GetPosition();
//		it->TranslateTemp(tmppos);
//		worldToCamera.SetIdentity();
//		worldToCamera = worldToCamera*player.Eye().GetWorldToCamera();
//		it->TransformToCamera( worldToCamera );
//		it->TransformToPerspective( perspectiveProjection );
//		std::list<Poly> templist = it->GetRenderList();
//		renderlist.splice(renderlist.end(), templist);
//	}

        testobj.updateList();
        testobj.TranslateTemp(testobj.GetPosition());
//        worldToCamera.SetIdentity();
//        testobj.TransformToCamera( worldToCamera );
        testobj.TransformToPerspective( perspectiveProjection );
        std::list<Poly> templist = testobj.GetRenderList();
        renderlist.splice(renderlist.end(), templist);

	for (std::list<Poly>::iterator it = renderlist.begin(); it != renderlist.end(); it++) {
		it->Clip();
		it->HomogeneousDivide();
		it->TransformToPixel( projectionToPixel );
		it->SetupRasterization( );		// for speed optimization
	}

        for (int y=SIZE_Y-1;y>=0;y--) {
                memset(z_buff,DEFAULT_Z_BUFFER,SIZE_X*sizeof(float));
                memset(disp_buff,RGB_BLACK,SIZE_X*sizeof(char));
                for (std::list<Poly>::iterator it = renderlist.begin(); it != renderlist.end(); it++) {
                        it->RasterizeFast(y,z_buff,disp_buff);
                }
                BufferToScreen(y);
        }

        Matrix rot;
        rot.SetRotation(3.141f/12.0f,Vector3D(0,0,1));
        testobj.Rotate(rot);
    }
    return 0;
}

void bootup(void)
{
    SCS = 3;
    
    // Setting Multiplier and Divider values
    PLLCFG=0x25; // M = 6
    PllFeed();
    
    // Enabling the PLL */
    PLLCON=0x1;
    PllFeed();
    
#define PLOCK 0x400
    // Wait for the PLL to lock to set frequency
    while(!(PLLSTAT & PLOCK)) ;
    
    // Connect the PLL as the clock source
    PLLCON=0x3;
    PllFeed();
    
    // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
    //MAMTIM=0x3; //VCOM?
    MAMCR=0x2;
    MAMTIM=0x4; //Original
    
    // Setting peripheral Clock (pclk) to System Clock (cclk)
    VPBDIV=0x1;
    
    FIO0DIR |= (1<<STAT0)|(1<<STAT1);
    FIO0SET = (1<<STAT0)|(1<<STAT1);
} // bootup

void PllFeed(void)
{
   PLLFEED=0xAA;
   PLLFEED=0x55;
} // PllFeed
