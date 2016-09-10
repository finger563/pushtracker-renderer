#ifndef ENGINE_H
#define ENGINE_H
  #include "main.h"
  #include <math.h>

extern "C"
{
  #include "OLED.h"
}

#include "tiles_scrap.h"  // level tile_map
//#include "renderTiles.h"

#define BG_CLR  0xFF

// defines for SPRITEs
#define SPRITE_STATE_DEAD         0    // this is a dead sprite
#define SPRITE_STATE_ALIVE        1    // this is a live sprite
#define SPRITE_STATE_DYING        2    // this sprite is dying
#define SPRITE_STATE_ANIM_DONE    1    // done animation state
#define MAX_SPRITE_FRAMES         64   // maximum number of sprite frames
#define MAX_SPRITE_ANIMATIONS     16   // maximum number of animation sequences

#define SPRITE_ATTR_SINGLE_FRAME   1   // sprite has single frame
#define SPRITE_ATTR_MULTI_FRAME    2   // sprite has multiple frames
#define SPRITE_ATTR_MULTI_ANIM     4   // sprite has multiple animations
#define SPRITE_ATTR_ANIM_ONE_SHOT  8   // sprite will perform the animation once
#define SPRITE_ATTR_VISIBLE        16  // sprite is visible
#define SPRITE_ATTR_BOUNCE         32  // sprite bounces off edges
#define SPRITE_ATTR_WRAPAROUND     64  // sprite wraps around edges
#define SPRITE_ATTR_LOADED         128 // the sprite has been loaded
#define SPRITE_ATTR_CLONE          256 // the sprite is a clone


//// these defined the general clipping rectangle
//int min_clip_x = 0,                             // clipping rectangle 
//    max_clip_x = (SCREEN_WIDTH-1),
//    min_clip_y = 0,
//    max_clip_y = (SCREEN_HEIGHT-1);
//
//// these are overwritten globally by DDraw_Init()
//int screen_width    = SCREEN_WIDTH,            // width of screen
//    screen_height   = SCREEN_HEIGHT,           // height of screen
//    screen_bpp      = SCREEN_BPP,              // bits per pixel
//    screen_windowed = 0;                       // is this a windowed app?    

typedef struct
{
        const u8* img;
        int width,height;
        int mempitch;
        char trans_char;
} SPRITE_IMG,*SPRITE_IMG_PTR;

// the blitter object structure SPRITE
typedef struct
        {
        int state;          // the state of the object (general)
        int anim_state;     // an animation state variable, up to you
        int attr;           // attributes pertaining to the object (general)
        int x,y;            // position bitmap will be displayed at
        int xv,yv;          // velocity of object
        int width, height;  // the width and height of the sprite
        int width_fill;     // internal, used to force 8*x wide surfaces
        int bpp;            // bits per pixel
        int counter_1;      // general counters
        int counter_2;
        int max_count_1;    // general threshold values;
        int max_count_2;
        int varsI[16];      // stack of 16 integers
        float varsF[16];    // stack of 16 floats
        int curr_frame;     // current animation frame
        int num_frames;     // total number of animation frames
        int curr_animation; // index of current animation
        int anim_counter;   // used to time animation transitions
        int anim_index;     // animation element index
        int anim_count_max; // number of cycles before animation
        int *animations[MAX_SPRITE_ANIMATIONS]; // animation sequences

        SPRITE_IMG* images[MAX_SPRITE_FRAMES]; // the bitmap images DD surfaces
 
        } SPRITE, *SPRITE_PTR;

        
// the simple bitmap image
typedef struct
        {
        int state;          // state of bitmap
        int attr;           // attributes of bitmap
        int x,y;            // position of bitmap
        int width, height;  // size of bitmap
        int num_bytes;      // total bytes of bitmap
        int bpp;            // bits per pixel
        u8 *buffer;      // pixels of bitmap

        } BITMAP_IMAGE, *BITMAP_IMAGE_PTR;

void Draw32x32(int x,int y,int index);
void draw_bg(int x,int y);


// SPRITE functions
int Create_SPRITE(SPRITE_PTR sprite,int x, int y,int width, int height,int num_frames,int attr,
               int mem_flags=0, u16 color_key_value=0, int bpp=8);              
int Clone_SPRITE(SPRITE_PTR source, SPRITE_PTR dest);
int Destroy_SPRITE(SPRITE_PTR sprite);
int Draw_SPRITE(SPRITE_PTR sprite, u8* dest);



struct POINT3D {
	float X, Y, Z;
	float U, V;

POINT3D& operator= (const POINT3D& rhs)
{
        X = rhs.X;
        Y = rhs.Y;
        Z = rhs.Z;
        U = rhs.U;
        V = rhs.V;
	return (*this);   
}
};

static void rotateY(POINT3D &p,float theta)
{
    POINT3D tmp = p;
    p.X = cos(theta)*tmp.X - sin(theta)*tmp.Z;
    p.Z = tmp.X*sin(theta) + tmp.Z*cos(theta);
}

static void rotateX(POINT3D &p,float theta)
{
    POINT3D tmp=p;
    p.Y = tmp.Y*cos(theta) - tmp.Z*sin(theta);
    p.Z = tmp.Y*sin(theta) + tmp.Z*cos(theta);
}

static void rotateZ(POINT3D &p,float theta)
{
    POINT3D tmp=p;
    p.X = tmp.X*cos(theta) - tmp.Y*sin(theta);
    p.Y = tmp.X*sin(theta) + tmp.Y*cos(theta);
}

void TextureMapTriangle( 
//                BITMAPINFO const *pDestInfo,
		BYTE *pDestBits, POINT3D const *pVertices,
//		BITMAPINFO const *pTextureInfo,
		SPRITE_IMG_PTR pTexture 
                );
/******** structures, inlines, and function declarations **********/
struct gradients {
	gradients( POINT3D const *pVertices );
	float aOneOverZ[3]; // 1/z for each vertex
	float aUOverZ[3]; // u/z for each vertex
	float aVOverZ[3]; // v/z for each vertex
	float dOneOverZdX, dOneOverZdY; // d(1/z)/dX, d(1/z)/dY
	float dUOverZdX, dUOverZdY; // d(u/z)/dX, d(u/z)/dY
	float dVOverZdX, dVOverZdY; // d(v/z)/dX, d(v/z)/dY
};
struct edge {
	edge(gradients const &Gradients,
			POINT3D const *pVertices,
			int Top, int Bottom );
	inline int Step( void );
	float X, XStep; // fractional x and dX/dY
	int Y, Height; // current y and vert count
	float OneOverZ, OneOverZStep; // 1/z and step
	float UOverZ, UOverZStep; // u/z and step
	float VOverZ, VOverZStep; // v/z and step
};

void DrawScanLine( 
//                BITMAPINFO const *pDestInfo,
		BYTE *pDestBits, 
                gradients const &Gradients,
		edge *pLeft, 
                edge *pRight,
//		BITMAPINFO const *pTextureInfo, 
                SPRITE_IMG_PTR pTexture );

//int Load_Frame_SPRITE(SPRITE_PTR sprite, BITMAP_FILE_PTR bitmap, int frame, int cx,int cy,int mode);   
int Animate_SPRITE(SPRITE_PTR sprite);
int Move_SPRITE(SPRITE_PTR sprite);
int Load_Animation_SPRITE(SPRITE_PTR sprite, int anim_index, int num_frames, int *sequence);
int Set_Pos_SPRITE(SPRITE_PTR sprite, int x, int y);
int Set_Vel_SPRITE(SPRITE_PTR sprite,int xv, int yv);
int Set_Anim_Speed_SPRITE(SPRITE_PTR sprite,int speed);
int Set_Animation_SPRITE(SPRITE_PTR sprite, int anim_index);
int Hide_SPRITE(SPRITE_PTR sprite);
int Show_SPRITE(SPRITE_PTR sprite);
int Collision_SPRITES(SPRITE_PTR sprite1, SPRITE_PTR sprite2);

#endif



//SPRITE_IMG mysprite = {sonic,30,48,210,TRANS_CHAR};
//
//POINT3D p1 = {0,0,1,
//             0,0};
//POINT3D p2 = {30,20,1,
//             30,20};
//POINT3D p3 = {0,40,1,
//             0,40};
//POINT3D p4 = {50,50,0,
//             0,0};
//
//POINT3D tri[4] = {p1,p2,p3,p4};
//POINT3D mytri[3];

//    UnselectAccel();
//    initSSP();
//    initAccel();
//    short acc_x,acc_y,spd_x,spd_y,spr_ind;
//    spd_y=0;
//    spr_ind=0;
//    int last_x,last_y,test_x,test_y;
//    int x=128;
//    int y=128;
//    int gnd,wall;
//
//    while (1)
//    {
//        ReadInput(&spd_x,&spd_y);
//        last_x=x;
//        last_y=y;
//        if (spd_x>=0)
//        {
//           gnd = GroundTest(50+15,90,1);  // 1 is forward
//           wall = WallTest(50+30,80,1);
//        }
//        else
//        {
//           gnd = GroundTest(50+15,90,0);  // 0 is backward
//           wall = WallTest(50,80,0);
//        }
//        if (gnd)
//        {
//             gnd -= 7;    // the ground is actually 8 pixels above the orange line
//             if (spd_y>0) spd_y = 0;
//             if (wall)
//             {
//                 x = ((x+spd_x)>=0) ? (x+spd_x):0;
//                 x = (x>(TILESET_WIDTH-1)) ? (TILESET_WIDTH-1):x;
//             }
//             y = ((y+spd_y)>=0) ? (y+spd_y):0;
//             y = (y>(TILESET_HEIGHT-1)) ? (TILESET_HEIGHT-1):y;
//             if (!spd_y) y += gnd;
//              draw_bg(x,y);  
//              test_x = abs(last_x-x);
//              test_y = abs(last_y-y);
//              if ((test_y>0 || test_x>0) && (test_y<5 && test_x<5))
//              {
//                  TileMap(50,90-40,sonicrun+((spr_ind/2)%6)*37,37,40,222,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else if (test_x>=5 || test_y>=5)
//              {
//                  TileMap(50,90-37,sonicdash+((spr_ind/2)%4)*32,32,37,128,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else
//              {
//                  TileMap(50,90-48,sonic,30,48,210,TRANS_CHAR);   // + gnd
//                  spr_ind=0;
//              }
//            
//        }
//        else
//        {
//             #define FALL_SPEED 0x2
//             #define MAX_FALL_SPEED 25
//             if (wall)
//             {
//                 x = ((x+spd_x)>=0) ? (x+spd_x):0;
//                 x = (x>(TILESET_WIDTH-1)) ? (TILESET_WIDTH-1):x;
//             }
//             spd_y = ((spd_y+FALL_SPEED)>MAX_FALL_SPEED) ? MAX_FALL_SPEED:(spd_y+FALL_SPEED);
//             y += spd_y;
//             y = (y>(TILESET_HEIGHT-1)) ? (TILESET_HEIGHT-1):y;
//              draw_bg(x,y);  
//              TileMap(50,90-33,sonicroll+((spr_ind/2)%5)*31,31,33,155,TRANS_CHAR);
//              spr_ind++;
//        }
//        switch (gnd)
//        {
//        case 0:
//             #define FALL_SPEED 0x2
//             #define MAX_FALL_SPEED 25
//             if (wall)
//             {
//                 x = ((x+spd_x)>=0) ? (x+spd_x):0;
//                 x = (x>(TILESET_WIDTH-1)) ? (TILESET_WIDTH-1):x;
//             }
//             spd_y = ((spd_y+FALL_SPEED)>MAX_FALL_SPEED) ? MAX_FALL_SPEED:(spd_y+FALL_SPEED);
//             y += spd_y;
//             y = (y>(TILESET_HEIGHT-1)) ? (TILESET_HEIGHT-1):y;
//              draw_bg(x,y);  
//              TileMap(50,90-33,sonicroll+((spr_ind/2)%5)*31,31,33,155,TRANS_CHAR);
//              spr_ind++;
//             break;
//        case 1:
//             if (spd_y>0) spd_y = 0;
//             if (wall)
//             {
//                 x = ((x+spd_x)>=0) ? (x+spd_x):0;
//                 x = (x>(TILESET_WIDTH-1)) ? (TILESET_WIDTH-1):x;
//             }
//             y = ((y+spd_y)>=0) ? (y+spd_y):0;
//             y = (y>(TILESET_HEIGHT-1)) ? (TILESET_HEIGHT-1):y;
//              draw_bg(x,y);  
//              test_x = abs(last_x-x);
//              test_y = abs(last_y-y);
//              if ((test_y>0 || test_x>0) && (test_y<5 && test_x<5))
//              {
//                  TileMap(50,90-40,sonicrun+((spr_ind/2)%6)*37,37,40,222,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else if (test_x>=5 || test_y>=5)
//              {
//                  TileMap(50,90-37,sonicdash+((spr_ind/2)%4)*32,32,37,128,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else
//              {
//                  TileMap(50,90-48,sonic,30,48,210,TRANS_CHAR);
//                  spr_ind=0;
//              }
//             break;
//        case 2:
//             if (!spd_y) spd_y = 0-abs(spd_x);
//             if (wall)
//             {
//                 x = ((x+spd_x)>=0) ? (x+spd_x):0;
//                 x = (x>(TILESET_WIDTH-1)) ? (TILESET_WIDTH-1):x;
//             }
//             y = ((y+spd_y)>=0) ? (y+spd_y):0;
//             y = (y>(TILESET_HEIGHT-1)) ? (TILESET_HEIGHT-1):y;
//              draw_bg(x,y);  
//              test_x = abs(last_x-x);
//              test_y = abs(last_y-y);
//              if ((test_y>0 || test_x>0) && (test_y<5 && test_x<5))
//              {
//                  TileMap(50,90-40,sonicrun+((spr_ind/2)%6)*37,37,40,222,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else if (test_x>=5 || test_y>=5)
//              {
//                  TileMap(50,90-37,sonicdash+((spr_ind/2)%4)*32,32,37,128,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else
//              {
//                  TileMap(50,90-48,sonic,30,48,210,TRANS_CHAR);
//                  spr_ind=0;
//              }
//             break;
//        default:
//            break;
//        }
//              draw_bg(x,y);  
//              test_x = abs(last_x-x);
//              test_y = abs(last_y-y);
//              if ((test_y>0 || test_x>0) && (test_y<5 && test_x<5))
//              {
//                  TileMap(50,90-40,sonicrun+((spr_ind/2)%6)*37,37,40,222,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else if (test_x>=5 || test_y>=5)
//              {
//                  TileMap(50,90-37,sonicdash+((spr_ind/2)%4)*32,32,37,128,TRANS_CHAR);
//                  spr_ind++;
//              }
//              else
//              {
//                  TileMap(50,90-48,sonic,30,48,210,TRANS_CHAR);
//                  spr_ind=0;
//              }
//              circle(50, 90, 5, GND_CLR,GND_CLR);
//              circle(70, 90, 5, WALL_CLR,WALL_CLR);