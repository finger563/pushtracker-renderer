#include "Engine.h"
#include <cstddef>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "SSP.h"
#include "ADXL345.h"
}


inline int edge::Step( void ) {
	X += XStep; Y++; Height--;
	UOverZ += UOverZStep; VOverZ += VOverZStep;
	OneOverZ += OneOverZStep;
	return Height;
}

void myTextureMapTriangle(
                BYTE *pDestBits,
                POINT3D const *pVertices,
                SPRITE_IMG_PTR pTexture
                )
{
    
}              

/********** DrawScanLine ************/
void myDrawScanLine( 
		BYTE *pDestBits, 
                gradients const &Gradients,
		edge *pLeft, edge *pRight,
		SPRITE_IMG_PTR pTexture
                )
{
	// we assume dest and texture are top-down
	int DestWidthBytes =
                (SCREEN_WIDTH+1);
	int TextureWidthBytes =
                pTexture->mempitch;

	int XStart = ceil(pLeft->X);
	float XPrestep = XStart - pLeft->X;
        const BYTE *pTextureBits = pTexture->img;

	pDestBits += pLeft->Y * DestWidthBytes + XStart;

	int Width = ceil(pRight->X) - XStart;
	float OneOverZ = pLeft->OneOverZ +
		XPrestep * Gradients.dOneOverZdX;
	float UOverZ = pLeft->UOverZ +
		XPrestep * Gradients.dUOverZdX;
	float VOverZ = pLeft->VOverZ +
		XPrestep * Gradients.dVOverZdX;
	if(Width > 0) 
	{
		while(Width--) 
		{
			float Z = 1/OneOverZ;
			int U = UOverZ * Z;
			int V = VOverZ * Z;
			*(pDestBits++) = *(pTextureBits + U +
				(V * TextureWidthBytes));
			OneOverZ += Gradients.dOneOverZdX;
			UOverZ += Gradients.dUOverZdX;
			VOverZ += Gradients.dVOverZdX;
		}
	}
}

		
/******** TextureMapTriangle **********/
void TextureMapTriangle( 
//                BITMAPINFO const *pDestInfo,
		BYTE *pDestBits, 
                POINT3D const *pVertices,
//		BITMAPINFO const *pTextureInfo,
		SPRITE_IMG_PTR pTexture 
                )
{
	int Top, Middle, Bottom;
	int MiddleCompare, BottomCompare;
	float Y0 = pVertices[0].Y;
	float Y1 = pVertices[1].Y;
	float Y2 = pVertices[2].Y;
	// sort vertices in y
	if(Y0 < Y1) 
	{
		if(Y2 < Y0) 
		{
			Top = 2; Middle = 0; Bottom = 1;
			MiddleCompare = 0; BottomCompare = 1;
		} 
		else 
		{
			Top = 0;
			if(Y1 < Y2) 
			{
				Middle = 1; Bottom = 2;
				MiddleCompare = 1; BottomCompare = 2;
			} 
			else 
			{
				Middle = 2; Bottom = 1;
				MiddleCompare = 2; BottomCompare = 1;
			}
		}
	} 
	else 
	{
		if(Y2 < Y1) 
		{
			Top = 2; Middle = 1; Bottom = 0;
			MiddleCompare = 1; BottomCompare = 0;
		} 
		else 
		{
			Top = 1;
			if(Y0 < Y2) 
			{
				Middle = 0; Bottom = 2;
				MiddleCompare = 3; BottomCompare = 2;
			} 
			else 
			{
				Middle = 2; Bottom = 0;
				MiddleCompare = 2; BottomCompare = 3;
			}
		}
	}
	gradients Gradients(pVertices);
	edge TopToBottom(Gradients,pVertices,Top,Bottom);
	edge TopToMiddle(Gradients,pVertices,Top,Middle);
	edge MiddleToBottom(Gradients,pVertices,Middle,Bottom);
	edge *pLeft, *pRight;
	int MiddleIsLeft;
	// the triangle is clockwise, so
	// if bottom > middle then middle is right
	if(BottomCompare > MiddleCompare) 
	{
		MiddleIsLeft = 0;
		pLeft = &TopToBottom; pRight = &TopToMiddle;
	} 
	else 
	{
		MiddleIsLeft = 1;
		pLeft = &TopToMiddle; pRight = &TopToBottom;
	}
	int Height = TopToMiddle.Height;
	while(Height--) 
	{
		DrawScanLine(pDestBits,Gradients,
				pLeft,pRight,pTexture);
		TopToMiddle.Step(); TopToBottom.Step();
	}
	Height = MiddleToBottom.Height;
	if(MiddleIsLeft) 
        {
               pLeft = &MiddleToBottom; pRight = &TopToBottom;
	} 
        else 
        {
               pLeft = &TopToBottom; pRight = &MiddleToBottom;
	}
	while(Height--) 
        {
//            DrawScanLine(pDestInfo,pDestBits,Gradients,
//                          pLeft,pRight,pTextureInfo,pTextureBits);
            DrawScanLine(pDestBits,Gradients,
                          pLeft,pRight,pTexture);
            MiddleToBottom.Step(); TopToBottom.Step();
	}
}

/********** gradients constructor **********/
gradients::gradients( POINT3D const *pVertices )
{
	int Counter;
	float OneOverdX = 1 /(((pVertices[1].X - pVertices[2].X) *
			(pVertices[0].Y - pVertices[2].Y)) -
			((pVertices[0].X - pVertices[2].X) *
			(pVertices[1].Y - pVertices[2].Y)));
	float OneOverdY = -OneOverdX;
	for(Counter = 0;Counter < 3;Counter++) 
	{
		float const OneOverZ = 1/pVertices[Counter].Z;
		aOneOverZ[Counter] = OneOverZ;
		aUOverZ[Counter] = pVertices[Counter].U * OneOverZ;
		aVOverZ[Counter] = pVertices[Counter].V * OneOverZ;
	}
	dOneOverZdX = OneOverdX * (((aOneOverZ[1] - aOneOverZ[2]) *
		(pVertices[0].Y - pVertices[2].Y)) -
		((aOneOverZ[0] - aOneOverZ[2]) *
		(pVertices[1].Y - pVertices[2].Y)));
	dOneOverZdY = OneOverdY * (((aOneOverZ[1] - aOneOverZ[2]) *
		(pVertices[0].X - pVertices[2].X)) -
		((aOneOverZ[0] - aOneOverZ[2]) *
		(pVertices[1].X - pVertices[2].X)));
	dUOverZdX = OneOverdX * (((aUOverZ[1] - aUOverZ[2]) *
		(pVertices[0].Y - pVertices[2].Y)) -
		((aUOverZ[0] - aUOverZ[2]) *
		(pVertices[1].Y - pVertices[2].Y)));
	dUOverZdY = OneOverdY * (((aUOverZ[1] - aUOverZ[2]) *
		(pVertices[0].X - pVertices[2].X)) -
		((aUOverZ[0] - aUOverZ[2]) *
		(pVertices[1].X - pVertices[2].X)));
	dVOverZdX = OneOverdX * (((aVOverZ[1] - aVOverZ[2]) *
		(pVertices[0].Y - pVertices[2].Y)) -
		((aVOverZ[0] - aVOverZ[2]) *
		(pVertices[1].Y - pVertices[2].Y)));
	dVOverZdY = OneOverdY * (((aVOverZ[1] - aVOverZ[2]) *
		(pVertices[0].X - pVertices[2].X)) -
		((aVOverZ[0] - aVOverZ[2]) *
		(pVertices[1].X - pVertices[2].X)));
}

/********** edge constructor ***********/
edge::edge( 
          gradients const &Gradients,
          POINT3D const *pVertices, 
          int Top, 
          int Bottom 
          )
{
	Y = ceil(pVertices[Top].Y);
	int YEnd = ceil(pVertices[Bottom].Y);
	Height = YEnd - Y;
	float YPrestep = Y - pVertices[Top].Y;
	float RealHeight = pVertices[Bottom].Y - pVertices[Top].Y;
	float RealWidth = pVertices[Bottom].X - pVertices[Top].X;
	X = ((RealWidth * YPrestep)/RealHeight) + pVertices[Top].X;
	XStep = RealWidth/RealHeight;
	float XPrestep = X - pVertices[Top].X;
	OneOverZ = Gradients.aOneOverZ[Top] +
		YPrestep * Gradients.dOneOverZdY +
		XPrestep * Gradients.dOneOverZdX;
	OneOverZStep = XStep *
		Gradients.dOneOverZdX + Gradients.dOneOverZdY;
	UOverZ = Gradients.aUOverZ[Top] +
		YPrestep * Gradients.dUOverZdY +
		XPrestep * Gradients.dUOverZdX;
	UOverZStep = XStep *
		Gradients.dUOverZdX + Gradients.dUOverZdY;
	VOverZ = Gradients.aVOverZ[Top] +
		YPrestep * Gradients.dVOverZdY +
		XPrestep * Gradients.dVOverZdX;
	VOverZStep = XStep *
		Gradients.dVOverZdX + Gradients.dVOverZdY;
}

/********** DrawScanLine ************/
void DrawScanLine( 
//                BITMAPINFO const *pDestInfo,
		BYTE *pDestBits, 
                gradients const &Gradients,
		edge *pLeft, edge *pRight,
//		BITMAPINFO const *pTextureInfo,
		SPRITE_IMG_PTR pTexture
                )
{
	// we assume dest and texture are top-down
	int DestWidthBytes =
                (SCREEN_WIDTH+1);
	int TextureWidthBytes =
                pTexture->mempitch;

	int XStart = ceil(pLeft->X);
	float XPrestep = XStart - pLeft->X;
        const BYTE *pTextureBits = pTexture->img;

	pDestBits += pLeft->Y * DestWidthBytes + XStart;

	int Width = ceil(pRight->X) - XStart;
	float OneOverZ = pLeft->OneOverZ +
		XPrestep * Gradients.dOneOverZdX;
	float UOverZ = pLeft->UOverZ +
		XPrestep * Gradients.dUOverZdX;
	float VOverZ = pLeft->VOverZ +
		XPrestep * Gradients.dVOverZdX;
	if(Width > 0) 
	{
		while(Width--) 
		{
			float Z = 1/OneOverZ;
			int U = UOverZ * Z;
			int V = VOverZ * Z;
			*(pDestBits++) = *(pTextureBits + U +
				(V * TextureWidthBytes));
			OneOverZ += Gradients.dOneOverZdX;
			UOverZ += Gradients.dUOverZdX;
			VOverZ += Gradients.dVOverZdX;
		}
	}
}




void Draw32x32(int x,int y,int index)
{

   const u8 *dsp = tiles32x32;
   int size32x32 = 4*4;
   dsp += index*size32x32;
   int size8x8 = 8*8;
   const u8 *ptr;
   for (int j = 0;j<size32x32;j++)
   {
        ptr = &tiles8x8[(*dsp)*(size8x8)];
        blit(((j%4)*8)+x,(j/4)*8+y,ptr,8,8,0x7C);
        dsp++;
   }
}

void draw_bg(int x,int y)
{
   FillScreen(BG_CLR);

   const u8 *dsp = tiles256x256; // ptr to top left tile
   const u8 *trdsp = tiles256x256;   // ptr to top right tile
   const u8 *bldsp = tiles256x256;   // ptr to bottom left tile
   const u8 *brdsp = tiles256x256;   // ptr to bottom right tile

   int size256x256 = 8*8;

   int tile_ind = x/256 + (y/256)*32;

   int x_shown = 0-(x%32);
   int y_shown = 0-(y%32);
   int i = (x/32)%8;
   int j = (y%256)/32;

   int index,indR,indB,indBR;

   index = tilesmap[tile_ind];

   dsp += index*size256x256;

   if ((tile_ind+32)<64) 
      indB = tilesmap[tile_ind + 32];  // to get the 256x256 tile directly below the current tile
      bldsp += indB*size256x256;

   if (((tile_ind + 1)%32)>(tile_ind%32))
      indR = tilesmap[tile_ind + 1];   // to get the 256x256 tile directly to the right of the current tile
      trdsp += indR*size256x256;

   if ((tile_ind +33)<64) 
      indBR = tilesmap[tile_ind+ 33];  // to get the 256x256 tile to the bottom right of the current tile
      brdsp += indBR*size256x256;

   while (y_shown<=127)
   {
      while (x_shown<=127)
      {
           if (i<=7 && j<=7)    // still in same 256x256 tile
           {
              Draw32x32(x_shown,y_shown,dsp[i + (j)*8]);
           }
           if (i<=7 && j>7 && (tile_ind+32)<64)    // now in bottom 256x256 tile
           {
              Draw32x32(x_shown,y_shown,bldsp[i + (j%8)*8]);              
           }
           if (i>7 && j<=7 && ((tile_ind + 1)%32)>(tile_ind%32))    // now in right 256x256 tile
           {
              Draw32x32(x_shown,y_shown,trdsp[i%8 + (j)*8]);
           }
           if (i>7 && j>7 && (tile_ind +33)<64)   // now in bottom right 256x256 tile
           {
              Draw32x32(x_shown,y_shown,brdsp[i%8 + (j%8)*8]);
           }
          i++;
          x_shown += 32;
      }
      j++;
      i=(x/32)%8;
      y_shown += 32;
      x_shown = 0-(x%32);
   }
}


void draw_sprite(const u8 *sprt)
{
//        blit((j*8)%128,(j/16)*8,sprt,8,8,0x2E);
//        dsp += 8*8;
}



//////////////////////////////////////////////////////////

int Create_SPRITE(SPRITE_PTR sprite,           // the sprite to create
               int x, int y,          // initial posiiton
               int width, int height, // size of sprite
               int num_frames,        // number of frames
               int attr,              // attrs
               int mem_flags,         // memory flags in DD format
               u16 color_key_value, // default color key
               int bpp)                // bits per pixel

{
// Create the SPRITE object, note that all SPRITEs 
// are created as offscreen surfaces in VRAM as the
// default, if you want to use system memory then
// set flags equal to:
// DDSCAPS_SYSTEMMEMORY 
// for video memory you can create either local VRAM surfaces or AGP 
// surfaces via the second set of constants shown below in the regular expression
// DDSCAPS_VIDEOMEMORY | (DDSCAPS_NONLOCALVIDMEM | DDSCAPS_LOCALVIDMEM ) 


//DDSURFACEDESC2 ddsd; // used to create surface
int index;           // looping var

// set state and attributes of SPRITE
sprite->state          = SPRITE_STATE_ALIVE;
sprite->attr           = attr;
sprite->anim_state     = 0;
sprite->counter_1      = 0;     
sprite->counter_2      = 0;
sprite->max_count_1    = 0;
sprite->max_count_2    = 0;

sprite->curr_frame     = 0;
sprite->num_frames     = num_frames;
sprite->bpp            = bpp;
sprite->curr_animation = 0;
sprite->anim_counter   = 0;
sprite->anim_index     = 0;
sprite->anim_count_max = 0; 
sprite->x              = x;
sprite->y              = y;
sprite->xv             = 0;
sprite->yv             = 0;

// set dimensions of the new bitmap surface
sprite->width  = width;
sprite->height = height;

// set all images to null
for (index=0; index<MAX_SPRITE_FRAMES; index++)
    sprite->images[index] = NULL;

// set all animations to null
for (index=0; index<MAX_SPRITE_ANIMATIONS; index++)
    sprite->animations[index] = NULL;

// return success
return(1);

} // end Create_SPRITE

///////////////////////////////////////////////////////////

int Draw_SPRITE(SPRITE_PTR sprite,               // sprite to draw
             u8* dest) // surface to draw the sprite on
{
// draw a sprite at the x,y defined in the SPRITE
// on the destination surface defined in dest

// is this a valid sprite
if (!sprite)
    return(0);

// is sprite visible
if (!(sprite->attr & SPRITE_ATTR_VISIBLE))
   return(1);

//// fill in the destination rect
//dest += sprite->x + sprite->y*SCREEN_WIDTH;
//dest_rect.left   = sprite->x;
//dest_rect.top    = sprite->y;
//dest_rect.right  = sprite->x+sprite->width;
//dest_rect.bottom = sprite->y+sprite->height;

//// fill in the source rect
//source_rect.left    = 0;
//source_rect.top     = 0;
//source_rect.right   = sprite->width;
//source_rect.bottom  = sprite->height;

// blt to destination surface
TileMap(sprite->x,
        sprite->y,
        sprite->images[sprite->curr_frame]->img,
        sprite->images[sprite->curr_frame]->width,
        sprite->images[sprite->curr_frame]->height,
        sprite->images[sprite->curr_frame]->mempitch,
        sprite->images[sprite->curr_frame]->trans_char);

//if (FAILED(dest->Blt(&dest_rect, sprite->images[sprite->curr_frame],
//          &source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),
//          NULL)))
//    return(0);

// return success
return(1);
} // end Draw_SPRITE

///////////////////////////////////////////////////////////

int Animate_SPRITE(SPRITE_PTR sprite)
{
// this function animates a sprite, basically it takes a look at
// the attributes of the sprite and determines if the sprite is 
// a single frame, multiframe, or multi animation, updates
// the counters and frames appropriately

// is this a valid sprite
if (!sprite)
   return(0);

// test the level of animation
if (sprite->attr & SPRITE_ATTR_SINGLE_FRAME)
    {
    // current frame always = 0
    sprite->curr_frame = 0;
    return(1);
    } // end if
else
if (sprite->attr & SPRITE_ATTR_MULTI_FRAME)
   {
   // update the counter and test if its time to increment frame
   if (++sprite->anim_counter >= sprite->anim_count_max)
      {
      // reset counter
      sprite->anim_counter = 0;

      // move to next frame
      if (++sprite->curr_frame >= sprite->num_frames)
         sprite->curr_frame = 0;

      } // end if
  
   } // end elseif
else
if (sprite->attr & SPRITE_ATTR_MULTI_ANIM)
   {
   // this is the most complex of the animations it must look up the
   // next frame in the animation sequence

   // first test if its time to animate
   if (++sprite->anim_counter >= sprite->anim_count_max)
      {
      // reset counter
      sprite->anim_counter = 0;

      // increment the animation frame index
      sprite->anim_index++;
      
      // extract the next frame from animation list 
      sprite->curr_frame = sprite->animations[sprite->curr_animation][sprite->anim_index];
     
      // is this and end sequence flag -1
      if (sprite->curr_frame == -1)
         {
         // test if this is a single shot animation
         if (sprite->attr & SPRITE_ATTR_ANIM_ONE_SHOT)
            {
            // set animation state message to done
            sprite->anim_state = SPRITE_STATE_ANIM_DONE;
            
            // reset frame back one
            sprite->anim_index--;

            // extract animation frame
            sprite->curr_frame = sprite->animations[sprite->curr_animation][sprite->anim_index];    

            } // end if
        else
           {
           // reset animation index
           sprite->anim_index = 0;

           // extract first animation frame
           sprite->curr_frame = sprite->animations[sprite->curr_animation][sprite->anim_index];
           } // end else

         }  // end if
      
      } // end if

   } // end elseif

// return success
return(1);

} // end Amimate_SPRITE

///////////////////////////////////////////////////////////

int Move_SPRITE(SPRITE_PTR sprite)
{
// this function moves the sprite based on its current velocity
// also, the function test for various motion attributes of the'
// sprite and takes the appropriate actions
   

// is this a valid sprite
if (!sprite)
   return(0);

// translate the sprite
sprite->x+=sprite->xv;
sprite->y+=sprite->yv;

//// test for wrap around
//if (sprite->attr & SPRITE_ATTR_WRAPAROUND)
//   {
//   // test x extents first
////   if (sprite->x > max_clip_x)
////       sprite->x = min_clip_x - sprite->width;
////   else
////   if (sprite->x < min_clip_x-sprite->width)
////       sprite->x = max_clip_x;
////   
////   // now y extents
////   if (sprite->x > max_clip_x)
////       sprite->x = min_clip_x - sprite->width;
////   else
////   if (sprite->x < min_clip_x-sprite->width)
////       sprite->x = max_clip_x;
////
////   } // end if
////else
////// test for bounce
////if (sprite->attr & SPRITE_ATTR_BOUNCE)
////   {
////   // test x extents first
////   if ((sprite->x > max_clip_x - sprite->width) || (sprite->x < min_clip_x) )
////       sprite->xv = -sprite->xv;
////    
////   // now y extents 
////   if ((sprite->y > max_clip_y - sprite->height) || (sprite->y < min_clip_y) )
////       sprite->yv = -sprite->yv;
//
//   } // end if

// return success
return(1);
} // end Move_SPRITE

///////////////////////////////////////////////////////////

int Load_Animation_SPRITE(SPRITE_PTR sprite, 
                       int anim_index, 
                       int num_frames, 
                       int *sequence)
{
// this function load an animation sequence for a sprite
// the sequence consists of frame indices, the function
// will append a -1 to the end of the list so the display
// software knows when to restart the animation sequence

// is this sprite valid
if (!sprite)
   return(0);

// allocate memory for sprite animation
if (!(sprite->animations[anim_index] = (int *)malloc((num_frames+1)*sizeof(int))))
   return(0);

// load data into 
int index;
for (index=0; index<num_frames; index++)
    sprite->animations[anim_index][index] = sequence[index];

// set the end of the list to a -1
sprite->animations[anim_index][index] = -1;

// return success
return(1);

} // end Load_Animation_SPRITE

///////////////////////////////////////////////////////////

int Set_Pos_SPRITE(SPRITE_PTR sprite, int x, int y)
{
// this functions sets the postion of a sprite

// is this a valid sprite
if (!sprite)
   return(0);

// set positin
sprite->x = x;
sprite->y = y;

// return success
return(1);
} // end Set_Pos_SPRITE

///////////////////////////////////////////////////////////

int Set_Anim_Speed_SPRITE(SPRITE_PTR sprite,int speed)
{
// this function simply sets the animation speed of a sprite
    
// is this a valid sprite
if (!sprite)
   return(0);

// set speed
sprite->anim_count_max = speed;

// return success
return(1);

} // end Set_Anim_Speed

///////////////////////////////////////////////////////////

int Set_Animation_SPRITE(SPRITE_PTR sprite, int anim_index)
{
// this function sets the animation to play

// is this a valid sprite
if (!sprite)
   return(0);

// set the animation index
sprite->curr_animation = anim_index;

// reset animation 
sprite->anim_index = 0;

// return success
return(1);

} // end Set_Animation_SPRITE

///////////////////////////////////////////////////////////

int Set_Vel_SPRITE(SPRITE_PTR sprite,int xv, int yv)
{
// this function sets the velocity of a sprite

// is this a valid sprite
if (!sprite)
   return(0);

// set velocity
sprite->xv = xv;
sprite->yv = yv;

// return success
return(1);
} // end Set_Vel_SPRITE

///////////////////////////////////////////////////////////

int Hide_SPRITE(SPRITE_PTR sprite)
{
// this functions hides sprite 

// is this a valid sprite
if (!sprite)
   return(0);

//// reset the visibility bit
//RESET_BIT(sprite->attr, SPRITE_ATTR_VISIBLE);

// return success
return(1);
} // end Hide_SPRITE

///////////////////////////////////////////////////////////

int Show_SPRITE(SPRITE_PTR sprite)
{
// this function shows a sprite

// is this a valid sprite
if (!sprite)
   return(0);

//// set the visibility bit
//SET_BIT(sprite->attr, SPRITE_ATTR_VISIBLE);

// return success
return(1);
} // end Show_SPRITE

///////////////////////////////////////////////////////////

int Collision_SPRITES(SPRITE_PTR sprite1, SPRITE_PTR sprite2)
{
// are these a valid sprites
if (!sprite1 || !sprite2)
   return(0);

// get the radi of each rect
int width1  = (sprite1->width>>1) - (sprite1->width>>3);
int height1 = (sprite1->height>>1) - (sprite1->height>>3);

int width2  = (sprite2->width>>1) - (sprite2->width>>3);
int height2 = (sprite2->height>>1) - (sprite2->height>>3);

// compute center of each rect
int cx1 = sprite1->x + width1;
int cy1 = sprite1->y + height1;

int cx2 = sprite2->x + width2;
int cy2 = sprite2->y + height2;

// compute deltas
int dx = abs(cx2 - cx1);
int dy = abs(cy2 - cy1);

// test if rects overlap
if (dx < (width1+width2) && dy < (height1+height2))
   return(1);
else
// else no collision
return(0);

} // end Collision_SPRITES
