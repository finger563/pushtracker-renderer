
typedef unsigned char u8;
typedef unsigned long u32;
typedef unsigned char byte;

#define BUSY LPC_GPIO3->MASKED_ACCESS[1<<1]

typedef struct
{
	int width;
	int height;
	const u8* map;
	const u8* t256x256;
	const u8* t32x32;
	const u8* t8x8;
} Tileset;

//#include "tiles.h"
#include "tiles_scrap.h"

Tileset gTiles;
void InitTiles()
{
	gTiles.width = TILESET_WIDTH;
	gTiles.height = TILESET_HEIGHT;
	gTiles.map = tilesmap;
	gTiles.t256x256 = tiles256x256;
	gTiles.t32x32 = tiles32x32;
	gTiles.t8x8 = tiles8x8;
}

static int Range(int x, int m)
{
     while (x < 0)
         x += m;
     while (x >= m)
         x -= m;
     return x;
}

void Get32x32(int index, int y, u8* dst, int d);
//void Get256x256(int index, int y, u8* dst, int d);
void Get256x256(int index, int y, u8* dst, u8* end);

////  buffer must be 128 bytes long
//void GetLine(int x, int y, byte* dst)
//{
//    const u8* tm = gTiles.map;
//    int width = gTiles.width >> 3;    // in blocks  = 8192>>3 = 1024
//    int height = gTiles.height >> 3;     // =512>>3 = 64
//
//    y = Range(y, height);
//    x &= ~31;                     // Always align to a 256 block
//
//    int d = 0;
//    int v = (y >> 5) * (width >> 5); //vertical block index
//    int i = 4;    // 4, 3*32 = 96, 4*32 = 128, so new line buffer is 128
//    while (i--)
//    {
//        x = Range(x, width);
//        byte i256 = tm[(x >> 5) + v];  // index of 256x256 block
//        Get256x256(i256, y & 31, dst, d);
//        d += 32;
//        x += 32;
//    }
//}

//void Get256x256(int index, int y, u8* dst, int d)
//{
//    const u8* tm = gTiles.t256x256;
//    int v = (index << 6) + ((y >> 2) << 3);
//    int x;
//    y &= 3;
//    for (x = 0; x < 8; x++)
//    {
//        byte i32 = tm[x + v];          // index of 32x32 block
//        Get32x32(i32, y & 3, dst, d);
//        d += 4;
//    }
//}


void Get256x256(int index, int y, u8* dst, u8* end)
{
    const u8* tm256 = tiles256x256;
    const u8* tm32 = tiles32x32;
	tm256 += (index << 6) + ((y >> 2) << 3);	// 8x8 blocks
	tm32 += ((y & 3) << 2);						// 4x4 blocks

	if (end > (dst+32))
		end = dst+32;
    u32* d32 = (u32*)dst;
    while (d32 < (u32*)end)
    {
		int index = *tm256++;
		u32* s32 = (u32*)(tm32 + (index << 4));
		*d32++ = *s32;
    }
}

//  buffer must be 96 bytes long (256xblocks)
//	Will return 1,2 or 3 256x256 pixel chunks
void GetCellLine(int x, int y, u8* dst, int count)
{
    const u8* tm = tilesmap;
    int width = TILESET_WIDTH >> 3;		// in blocks
    int height = TILESET_HEIGHT >> 3;
    y = Range(y, height);
	width >>= 5;
	x >>= 5;
	tm += (y >> 5) * width;				// vertical block index
	y &= 31;

	u8* end = dst+count;	// may exceed count by 31
    while (dst < end)
    {
        x = Range(x, width);			// read up to 32 cells from 256x256 block
        Get256x256(tm[x++], y, dst, end);
        dst += 32;
    }
}

void Get32x32(int index, int y, u8* dst, int d)
{
    const u8* tm = gTiles.t32x32;
    int v = (index << 4) + (y << 2);
    u32* d32 = (u32*)(dst+d);
    u32* s32 = (u32*)(tm+v);
    *d32 = *s32;
}

void EnqueueLine(volatile u8** lineHandle);
#define LINE_BUFFER_COUNT 4

typedef struct {
	int LineIndex;
	volatile u8* volatile Lines[LINE_BUFFER_COUNT];
	u8	Buffer[LINE_BUFFER_COUNT*320];
	u8	renderprocbuf[128];
} RenderState;
RenderState gRenderState;

typedef void (*RenderProc)(u8* dst, const u8* cells, const u8* base, volatile uint32_t* state);

void RenderLine2(u8* dst, const u8* cells, const u8* base, volatile uint32_t* state)
{
	asm volatile (
		".syntax unified\n"
		"	mov 	r0,%[dst]\n"
		"	mov 	r1,%[cells]\n"
		"	mov 	r2,%[base]\n"
		"	mov		r6,%[state]\n"

		"	mov		r3,r8\n"		// save r8
		"	mov		r4,r9\n"		// save r9
		"	mov		r5,r7\n"		// save r7
		"	push	{r3,r4,r5}\n"
		"	mov		r8,r2\n"		// base in r8
		"	movs	r7,#36\n"
		"	adds	r7,r7,r1\n"		// End ptr

		"0:\n"
		//	"	movs	r2,#0x7\n"
		//	"	strb	r2,[r6,#0]\n"

			"	ldrb	r2,[r1,#0]\n"	//b0
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r3,r4}\n"	//

			"	ldrb	r2,[r1,#1]\n"	//b1
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r5,r6}\n"	//
			"	stm		r0!,{r3,r4,r5,r6}\n"	//

			"	ldrb	r2,[r1,#2]\n"	//b2
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r3,r4}\n"	//

			"	ldrb	r2,[r1,#3]\n"	//b3
			"	lsls	r2,r2,#6\n"		// *64
			"	add		r2,r2,r8\n"		// base
			"	ldm		r2!,{r5,r6}\n"	//
			"	stm		r0!,{r3,r4,r5,r6}\n"	//

		//	"	movs	r2,#0x0\n"
		//	"	strb	r2,[r6,#0]\n"

			"	adds	r1,r1,#4\n"
			"	cmp		r1,r7\n"
			"	bne		0b\n"

		"	pop		{r3,r4,r5}\n"
		"	mov		r7,r5\n"
		"	mov		r8,r3\n"
		"	mov		r9,r4\n"
			".syntax divided\n"
			:
			: [dst] "r" (dst),[cells] "r" (cells),[base] "r" (base),[state] "r" (state)
			: "r0","r1","r2","r3","r4","r5","r6"
		);
}
