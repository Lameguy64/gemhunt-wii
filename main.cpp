#include <mgdl-wii.h>
#include "include/names.h"
#include "include/tilerender.cpp"
#include "include/input.cpp"

#define	ONE		4096
#define GRAVITY	0.12f

#define MAX_PARTICLES	512
#define MAX_PROJECTILES	256

#define SURFACE_LEVEL 16
#define ROCK_LEVEL 24

typedef struct {
	int x,y;
	float fx,fy;
} cameraStruct;


typedef struct {

	float pointerX,pointerY;
	float pointerAngle;
	bool  pointerOn;

	bool  jump;
	bool  dash;

	float analog_xMove;
	float analog_yMove;

} inputStruct;


typedef struct {

    short   x,y;

    int     fx,fy;
    int     xMove,yMove;

    short   dir;
    float   anim;

    int     Jump;
    bool    JumpPressed;

    bool    OnFloor;

    short   gems,hp;
    float   damage;

    short   MineX,MineY;
    short   MineSide;
    short   MineStage;
    float   MineCount;

    float   ReticuleX;
    float   ReticuleY;
    float   ReticuleDist;
	float   ReticuleDistClip;
    float   ReticuleAngle;
    bool    OnMineableTile;

    short   dead;
    float   deadRot;
    float   deadScale;

	cameraStruct camera;
	inputStruct	 input;

} playerStruct;


// Screen stuff
int ScreenSidePad=8;


// Graphics objects
gdl::Font		font;
gdl::SpriteSet	sprites;
gdl::Image		mapImage;
gdl::Image		tileSheet;
gdl::Image		tileSheetBg;

tr::TileRender	tileRender;
tr::TileRender	tileRenderBg;


// Sound effects
#define SND_MAXSOUNDS	4
#define	SND_KEYPASS		0
#define SND_KEYPRESS	1
#define SND_JUMP		2
#define SND_EXPLODE		3

gdl::Sound		sound[SND_MAXSOUNDS];


// Declarations
void DrawMiniMap(short x, short y, playerStruct *player);
void DestroyTile(short x, short y, tr::TileRender *tileRender, void *particles);

void DoInput(int chan, playerStruct *player);
void DoPlayerLogic(playerStruct *player);
void DrawPlayerSprite(playerStruct *player);

void PrepDisplay();
void Display();



// Find the angle from x1/y1 to x2/y2 in radians
float FindAngleRad(float x1, float y1, float x2, float y2) {

	return(atan2(x2-x1, y1-y2));

}

// Find the length of a line from x1/y1 to x2/y2
float FindLineLength(float x1, float y1, float x2, float y2) {

	return(sqrt(pow(x2-x1, 2) + pow(y2-y1, 2)));

}

bool inbox(int x, int y, int x1, int y1, int x2, int y2) {

	if ((x >= x1) && (x <= x2)) {
		if ((y >= y1) && (y <= y2)) {
			return(true);
		}
	}

	return(false);

}

bool isSolid(u_short index) {

	switch(index) {
	case 1:
	case 2:
	case 5:
		return(true);
	default:
		return(false);
	}

}


// Additional includes
#include "include/osk.cpp"
#include "include/particles.cpp"
#include "include/projectiles.cpp"


void loadGlobalResources() {


	// Graphics
	if (!font.LoadFont("data/normfont.fnt"))
		exit(0);
	if (!sprites.LoadSprites("data/sprites/sprites.tsm", "data/sprites/", gdl::Nearest, gdl::RGBA8))
		exit(0);
	if (!tileSheet.LoadImage("data/tiles/tiles_default.png", gdl::Nearest, gdl::RGBA8))
		exit(0);
	if (!tileSheetBg.LoadImage("data/tiles/tiles_bg_default.png", gdl::Nearest, gdl::RGBA8))
		exit(0);

	// Sound effects
	if (!sound[SND_KEYPASS].LoadSound("data/sounds/sfx_keypass.wav"))
		exit(0);
	if (!sound[SND_KEYPRESS].LoadSound("data/sounds/sfx_keypress.wav"))
		exit(0);
	if (!sound[SND_JUMP].LoadSound("data/sounds/sfx_jump.wav"))
		exit(0);
	if (!sound[SND_EXPLODE].LoadSound("data/sounds/sfx_explosion.wav"))
		exit(0);


}


void generateWorld(tr::TileRender &tileMap, u_int seed) {


	u_char	heightMap[2][tileMap.MapXsize()];


	for(int i=0; i<tileMap.MapXsize(); i++) {
		heightMap[0][i] = 8-((8*(((float)rand()/RAND_MAX)))-4);
		heightMap[1][i] = 16-((8*(((float)rand()/RAND_MAX)))-4);
	}


	for(int i=0; i<4; i++) {
		for(int tx=1; tx<tileMap.MapXsize()-1; tx++) {
			heightMap[0][tx] = (heightMap[0][tx]+heightMap[0][tx+1])/2;
			heightMap[1][tx] = (heightMap[1][tx]+heightMap[1][tx+1])/2;
		}
	}


	u_char	tempMap[tileMap.MapYsize()][tileMap.MapXsize()];

	for(int i=0; i<2; i++) {

		for(int ty=0; ty<tileMap.MapYsize(); ty++) {
			for(int tx=0; tx<tileMap.MapXsize(); tx++) {
				tempMap[ty][tx] = 256*((float)rand()/RAND_MAX);
			}
		}

	}

	for(int i=0; i<4; i++) {

		for(int ty=0; ty<tileMap.MapYsize(); ty++) {
			for(int tx=0; tx<tileMap.MapXsize(); tx++) {

				int avgVal = 0;
				int numDivs = 0;

				for(int py=ty-1; py<=ty+1; py++) {
					for(int px=tx-1; px<=tx+1; px++) {

						if ((px >= 0) && (px < tileMap.MapXsize())) {
							if ((py >= 0) && (py < tileMap.MapYsize())) {
								avgVal += tempMap[py][px];
								numDivs++;
							}
						}

					}
				}

				if (numDivs)
					tempMap[ty][tx] = avgVal/numDivs;

			}
		}

	}

	for(int ty=0; ty<tileMap.MapYsize(); ty++) {
		for(int tx=0; tx<tileMap.MapXsize(); tx++) {

			if (tempMap[ty][tx] < 120) {
				tileMap.SetTile(tx, ty, 0);
			} else {
				tileMap.SetTile(tx, ty, 1);
			}

		}
	}

	for(int tx=0; tx<tileMap.MapXsize(); tx++) {

		for(int i=heightMap[0][tx]; i<=heightMap[1][tx]; i++) {
			tileMap.SetTile(tx, i, 0);
		}

	}


	/*for(int tx=0; tx<tileMap.MapXsize(); tx++) {

		for(int i=0; i<heightMap[0][tx]; i++) {
			tileMap.SetTile(tx, i, 1);
		}

		for(int i=heightMap[1][tx]; i<tileMap.MapYsize(); i++) {
			tileMap.SetTile(tx, i, 1);
		}

	}*/


}

void init(int argc, char *argv[]) {

	// Init file system
	fatInitDefault();


	gdl::InitAspectMode aspectMode=gdl::AspectAuto;

	// Scan program arguments
	for(int i=0; i<argc; i++) {

		if (strcmp(argv[i], "-widescreen") == 0)
			aspectMode = gdl::Aspect16x9;

	}

	gdl::InitSystem(gdl::ModeAuto, aspectMode, gdl::LowRes, 0);


	// Init input
	input::Init();

	// Load game resources
	gdl::ConsoleMode();
	loadGlobalResources();

}

int main(int argc, char *argv[]) {


	init(argc, argv);


	playerStruct player={0};

	BulletClass	  Bullets;
	ParticleClass Particles;

	tr::TileSet	tiles;
	tiles.Create(tileSheet.Texture.TexObj(), tileSheet.Xsize(), tileSheet.Ysize(), 16, 16);

	tr::TileSet	bgtiles;
	bgtiles.Create(tileSheetBg.Texture.TexObj(), tileSheetBg.Xsize(), tileSheetBg.Ysize(), 16, 16);


	// Create and generate world
	void *mapBuff = valloc(256*128);
	memset(mapBuff, 0x00, (256*128));

	tileRender.Create(gdl::ScreenXres, gdl::ScreenYres, 16, 16);
	tileRender.SetMap(256, 128, mapBuff, 0);
	tileRender.SetTileSet(tiles);

	generateWorld(tileRender, 0);


	tileRenderBg.Create(gdl::ScreenXres, gdl::ScreenYres, 16, 16);
	tileRenderBg.SetMap(256, 128, NULL, 0);
	tileRenderBg.SetTileSet(bgtiles);


	GXColor		renderCol;

	renderCol.r = 0xff;
	renderCol.g = 0xff;
	renderCol.b = 0xff;
	renderCol.a = 0xff;


	mapImage.Create(64, 64, gdl::Nearest, gdl::RGB565);


	//gdl::PlayMusic("data/music/gh_music_01.ogg", true);
	gdl::SetMasterVolumes(75, 100);

	player.OnFloor = true;
	player.input.pointerX = gdl::ScreenCenterX;
	player.input.pointerY = gdl::ScreenCenterY;

	player.fx = ONE*32;
	player.fy = ONE*(16*12)-1;



	bool firePressed=false;

	while(1) {


		input::UpdateInput();
		DoInput(0, &player);


		if (input::Wiimote[0].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_FULL_R) {

			tileRender.SetTile(
				(player.camera.x+player.input.pointerX)/16,
				(player.camera.y+player.input.pointerY)/16,
				0);

		}

		if (input::Wiimote[0].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_FULL_L) {
			if (firePressed == false) {

				Bullets.PutBullet(player.camera.x+player.ReticuleX, player.camera.y+player.ReticuleY,
					(player.ReticuleDist/32.f)*sin(player.ReticuleAngle),
					-((player.ReticuleDist/32.f)*cos(player.ReticuleAngle)),
					0, BulletClass::Dynamite);

				/*Bullets.PutBullet(player.x, player.y,
					(player.input.pointerX-gdl::ScreenCenterX)/32.f,
					(player.input.pointerY-gdl::ScreenCenterY)/16.f,
					0, BulletClass::Dynamite);
				*/

			}
			firePressed = true;
		} else {
			firePressed = false;
		}

		if (input::Wiimote[0].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_ZL) {

			tileRender.SetTile(
				(player.camera.x+player.input.pointerX)/16,
				(player.camera.y+player.input.pointerY)/16,
				1);

		}


		if (input::Wiimote[0].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_HOME)
			break;


		DoPlayerLogic(&player);



		PrepDisplay();


		// Draw main tilemap
		GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
		GX_SetChanMatColor(GX_COLOR0A0, renderCol);

		tileRenderBg.Render(player.camera.x, player.camera.y);
		tileRender.Render(player.camera.x, player.camera.y);

		GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);


		// Draw player
		DrawPlayerSprite(&player);


		Bullets.DoBullets(&player, &Particles);
		Particles.DoParticles(&player);


		// Draw HUD graphics

		// HP bar
		sprites.Put(ScreenSidePad, gdl::ScreenYres-34, HUD_HP_ICON, gdl::Color::White);
		sprites.PutS(ScreenSidePad+11, gdl::ScreenYres-32, (ScreenSidePad+11)+100, gdl::ScreenYres-27, HUD_HP_ICON+1, gdl::Color::White);
		font.Printf(ScreenSidePad+14, gdl::ScreenYres-34, 1.f, gdl::Color::Black, "100/100");
		font.Printf(ScreenSidePad+13, gdl::ScreenYres-34, 1.f, gdl::Color::White, "100/100");

		// MP bar
		sprites.Put(ScreenSidePad, gdl::ScreenYres-24, HUD_MP_ICON, gdl::Color::White);
		sprites.PutS(ScreenSidePad+11, gdl::ScreenYres-22, (ScreenSidePad+11)+100, gdl::ScreenYres-17, HUD_MP_ICON+1, gdl::Color::White);
		font.Printf(ScreenSidePad+14, gdl::ScreenYres-24, 1.f, gdl::Color::Black, "75/75");
		font.Printf(ScreenSidePad+13, gdl::ScreenYres-24, 1.f, gdl::Color::White, "75/75");


		// Mini-map
		DrawMiniMap((gdl::ScreenXres-ScreenSidePad)-64, gdl::ScreenYres-82, &player);


		if (player.input.pointerOn) {

			sprites.Put(player.input.pointerX, player.input.pointerY, CROSSHAIR, gdl::Color::White,
				gdl::Pivot, gdl::Pivot, 1.f, 0);

			sprites.Put(player.ReticuleX, player.ReticuleY, CROSSHAIR, gdl::Color::Red,
				gdl::Pivot, gdl::Pivot, 1.f, 0);

		}


		if (input::Wiimote[0].status == WPAD_ERR_NONE) {
			if (input::Wiimote[0].data.btns_h & WPAD_BUTTON_2) {
				gdl::SaveScreen("gemhunt_screencap.png");
			}
		}


		Display();

		if (input::Wiimote[0].status == WPAD_ERR_NONE)
			if (input::Wiimote[0].data.btns_h & WPAD_BUTTON_2)
				gdl::Delta = 1.0f;

	}

	gdl::ConsoleMode();
	printf("Quitting...\n");

}

void DrawMiniMap(short x, short y, playerStruct *player) {

	int px=player->x/16;
	int py=player->y/16;
	int sx=px-32;
	int sy=py-32;

	if (sx < 0)
		sx = 0;
	if (sy < 0)
		sy = 0;
	if (sx > (tileRender.MapXsize()-64))
		sx = tileRender.MapXsize()-64;
	if (sy > (tileRender.MapYsize()-64))
		sy = tileRender.MapYsize()-64;

	gdl::wii::SetAutoFlush(false);

	for(int ty=0; ty<64; ty++) {
		for(int tx=0; tx<64; tx++) {

			if (tileRender.GetTile(sx+tx, sy+ty))
				mapImage.Texture.PokePixel(tx, ty, RGBA(0, 127, 191, 255));
			else
				mapImage.Texture.PokePixel(tx, ty, RGBA(0, 0, 191, 255));

		}
	}

	mapImage.Texture.Flush();


	gdl::DrawBox(x, y, x+65, y+65, gdl::Color::White);
	mapImage.Put(x+1, y+1, RGBA(255, 255, 255, 127));

	gdl::DrawPoint((x+1)+(px-sx), (y+1)+(py-sy), gdl::Color::White);

}

void DestroyTile(short x, short y, tr::TileRender *tileRender, void *particles) {

	if (isSolid(tileRender->GetTile(x, y))) {

		tileRender->SetTile(x, y, 0);

		for(int i=0; i<4+(4*((float)rand()/RAND_MAX)); i++) {

			int tx=((16*x)+8)+(( 8*((float)rand()/RAND_MAX) )-4);
			int ty=((16*y)+8)+(( 8*((float)rand()/RAND_MAX) )-4);

			((ParticleClass*)particles)->PutParticle(tx, ty, (6*((float)rand()/RAND_MAX))-3, (6*((float)rand()/RAND_MAX))-3, 360*((float)rand()/RAND_MAX), ParticleClass::Rubble);

		}

	}

}

void DoInput(int chan, playerStruct *player) {

	player->input.pointerOn	= false;
	player->input.jump		= false;
	player->input.dash		= false;

	if (input::Wiimote[chan].status == WPAD_ERR_NONE) {

		switch(input::Wiimote[chan].data.exp.type) {
		case EXP_NUNCHUK:

			// Get Nunchuk input
			player->input.analog_xMove = (float)(input::Wiimote[chan].data.exp.nunchuk.js.pos.x-129)/100;
			player->input.analog_yMove = (float)(input::Wiimote[chan].data.exp.nunchuk.js.pos.x-129)/100;

			// Get pointer input
			if (input::Wiimote[chan].data.ir.valid) {

				player->input.pointerX = input::Wiimote[chan].data.ir.x;
				player->input.pointerY = input::Wiimote[chan].data.ir.y;
				player->input.pointerOn = true;

			}

			if (input::Wiimote[chan].data.btns_h & WPAD_BUTTON_A)
				player->input.jump = true;

			break;

		case EXP_CLASSIC:

			player->input.analog_xMove = (float)(input::Wiimote[chan].data.exp.classic.ljs.pos.x-32)/32;
			player->input.analog_yMove = (float)(input::Wiimote[chan].data.exp.classic.ljs.pos.y-32)/32;

			player->input.pointerX += ((float)(input::Wiimote[chan].data.exp.classic.rjs.pos.x-15)/15)*8;
			player->input.pointerY -= ((float)(input::Wiimote[chan].data.exp.classic.rjs.pos.y-15)/15)*8;

			if (player->input.pointerX < ScreenSidePad)
				player->input.pointerX = ScreenSidePad;
			if (player->input.pointerX > gdl::ScreenXres-ScreenSidePad)
				player->input.pointerX = gdl::ScreenXres-ScreenSidePad;
			if (player->input.pointerY < 20)
				player->input.pointerY = 20;
			if (player->input.pointerY > gdl::ScreenYres-20)
				player->input.pointerY = gdl::ScreenYres-20;

			player->input.pointerOn = true;


			// Hori Battlepad (GC style Classic Controller) button map
			if (input::Wiimote[chan].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_B)
				player->input.dash = true;

			if (input::Wiimote[chan].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_A)
				player->input.jump = true;



			break;

		}
	}

}

short LineScan(float x0, float y0, float x1, float y1, short *cx, short *cy, short *lx, short *ly, tr::TileRender *tileMap) {

	enum {PREC=2^16};

	int ix = floor(x0),ix1 = floor(x1)-ix,ixi = (ix1>0)*2-1;
	int iy = floor(y0),iy1 = floor(y1)-iy,iyi = (iy1>0)*2-1;

	float fx = x0-ix; if (ixi > 0) fx = 1-fx;
	float fy = y0-iy; if (iyi > 0) fy = 1-fy;

	float gx = fabs(x1-x0)*PREC; int idx = int(gx);
	float gy = fabs(y1-y0)*PREC; int idy = int(gy);

	float id;

	if (ix1 == 0) {
		id = -1; idx = 0;
	} else if (iy1 == 0) {
		id =  0; idy = 0;
	} else {
		id = int(fy*gx - fx*gy);
	}

	*lx = ix; *ly = iy;
	for(int c=abs(ix1)+abs(iy1);c>=0;c--) {

		if (isSolid(tileMap->GetTile(ix/16, iy/16))) {
			*cx = ix; *cy = iy;
			return(tileMap->GetTile(ix/16, iy/16));
		}

		*lx = ix; *ly = iy;
		if (id >= 0) {
			ix += ixi; id -= idy;
		} else {
			iy += iyi; id += idx;
		}

	}

	return(0);

}

void DoPlayerLogic(playerStruct *player) {

	int i=0;
	short cx,cy;
	float lx,ly;

	// Jumping
	if (player->input.jump) {

		if (player->JumpPressed) {

			if ((player->Jump) && (player->Jump < ONE*6)) {
				player->yMove -= ((ONE/2)+abs(player->xMove/32))*gdl::Delta;
				player->Jump += ONE*gdl::Delta;
			} else {
				player->Jump = 0;
			}

		}

		if ((player->JumpPressed == false) && (player->OnFloor) && (isSolid(tileRender.CollideT(player->x, player->y, i, -32, &cy)) == false)) {

			//player->yMove = -((ONE+(abs(player->xMove/16)))*2.4f);
			player->yMove = -(ONE*1.2f);
			player->Jump = ONE;
			sound[SND_JUMP].Play2D(1.f, 100.f, (player->x-player->camera.x)-gdl::ScreenCenterX, (player->y-player->camera.y)-gdl::ScreenCenterY);

		}

		player->JumpPressed = true;

	} else {

		player->JumpPressed = false;
		player->Jump = 0;

	}


	// Movement
	if (abs(ONE*player->input.analog_xMove) > 1024) {

		if ((ONE*player->input.analog_xMove) < 1024)
			player->dir = -1;

		if ((ONE*player->input.analog_xMove) > 1024)
			player->dir = 1;

		if (player->input.dash == false) {

			if ((player->xMove < (ONE*2)) && (player->xMove > -(ONE*2))) {

				player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

			} else if ((player->xMove > (ONE*2)) || (player->xMove < -(ONE*2))) {

				if ((player->xMove > ONE) && ((ONE*player->input.analog_xMove)<0)) {

					player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

				} else if ((player->xMove < -ONE) && ((ONE*player->input.analog_xMove)>0)) {

					player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

				}
			}

		} else {

			if (player->OnFloor) {

				if ((player->xMove > -(ONE*5)) && (player->xMove < (ONE*5))) {

					player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

				}

			} else {

				if ((player->xMove > -(ONE*2)) && (player->xMove < (ONE*2))) {

					player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

				} else if ((player->xMove > (ONE*2)) || (player->xMove < -(ONE*2))) {

					if ((player->xMove > ONE) && ((ONE*player->input.analog_xMove)<0)) {

						player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

					} else if ((player->xMove < -ONE) && ((ONE*player->input.analog_xMove)>0)) {

						player->xMove += (ONE*(player->input.analog_xMove/4))*gdl::Delta;

					}
				}

			}

		}

	}


	if (player->input.pointerOn) {

		if ((player->input.pointerX+player->camera.x) < player->x) {

			if ((player->OnFloor) && (player->xMove > (ONE*1.5f)))
				player->xMove = (ONE*1.5f);

			player->dir = -1;

		}

		if ((player->input.pointerX+player->camera.x) > player->x) {

			if ((player->OnFloor) && (player->xMove < -(ONE*1.5f)))
				player->xMove = -(ONE*1.5f);

			player->dir = 1;

		}

	}


	// Momentum control
	if (player->OnFloor) {

		i=abs(player->xMove);

		if (i > 320)
			i=320;

		if (player->xMove > 0)
			player->xMove -= (i*gdl::Delta);
		else if (player->xMove < 0)
			player->xMove += (i*gdl::Delta);

	} else {

		i = abs(player->xMove);

		if (i > 128)
			i=128;

		if (player->xMove > 0)
			player->xMove -= (i*gdl::Delta);
		else if (player->xMove < 0)
			player->xMove += (i*gdl::Delta);


	}


	// Player physics
	player->yMove += (ONE*GRAVITY)*gdl::Delta;
	player->fx += player->xMove*gdl::Delta;
	player->fy += player->yMove*gdl::Delta;
	player->x = (short)(player->fx/ONE);
	player->y = (short)(player->fy/ONE);

	int destX=0;
	int destY=0;

	if (input::Wiimote[0].data.exp.classic.btns_held & CLASSIC_CTRL_BUTTON_ZR) {

		destX =(player->x+(player->input.pointerX-gdl::ScreenCenterX))-gdl::ScreenCenterX;
		destY =((player->y-18)+(player->input.pointerY-gdl::ScreenCenterY))-gdl::ScreenCenterY;

	} else {

		destX = player->x-gdl::ScreenCenterX;
		destY = (player->y-18)-gdl::ScreenCenterY;

	}

	if (destX < 0)
		destX = 0;
	if (destY < 0)
		destY = 0;
	if (destX > ((16*tileRender.MapXsize())-gdl::ScreenXres))
		destX = (16*tileRender.MapXsize())-gdl::ScreenXres;
	if (destY > ((16*tileRender.MapYsize())-gdl::ScreenYres))
		destY = (16*tileRender.MapYsize())-gdl::ScreenYres;

    player->camera.fx += ((destX-player->camera.fx)/12)*gdl::Delta;
    player->camera.fy += ((destY-player->camera.fy)/12)*gdl::Delta;
    player->camera.x = trunc(player->camera.fx);
    player->camera.y = trunc(player->camera.fy);


	player->OnFloor = false;


	// Collision detection
    for(i=-4; i<4; i+=2) {

        if (isSolid(tileRender.CollideT(player->x, player->y, i, -32, &cy))) {

			player->y = cy;
			player->fy = ONE*player->y;

            if (player->yMove < 0) {
                player->yMove = ONE;
                player->Jump = 0;
            }

        }

        if (isSolid(tileRender.CollideB(player->x, player->y, i, 1, &cy))) {

            player->y = cy;
			player->fy = ONE*player->y;

            /*if (((float)player->yMove/ONE) > 16.f) {

                player->hp		-= ((float)player->yMove/ONE)-16.f;
                player->damage	= 20*(((float)player->yMove/ONE)-5);
                //sound[3].Play(1.f, 100.f);

            }*/

            player->yMove	= 0;
            player->OnFloor	= true;
            player->Jump	= 0;

        }

    }

	for(i=0; i<25; i+=5) {

		if (isSolid(tileRender.CollideL(player->x, player->y, -6, -i-4, &cx))) {
            player->x		= cx;
			player->fx		= ONE*player->x;
            player->xMove	= 0;
        }

        if (isSolid(tileRender.CollideR(player->x, player->y, 6, -i-4, &cx))) {
            player->x		= cx;
			player->fx		= ONE*player->x;
            player->xMove	= 0;
        }

    }


	// Item aim reticule
	if (player->input.pointerOn) {

		player->ReticuleAngle = FindAngleRad(
			player->x-player->camera.x, (player->y-player->camera.y)-10,
			player->input.pointerX, player->input.pointerY);

		player->ReticuleDist = FindLineLength(
			player->x-player->camera.x, (player->y-player->camera.y)-10,
			player->input.pointerX, player->input.pointerY);

		player->ReticuleDistClip = player->ReticuleDist;
		if (player->ReticuleDistClip > 50)
			player->ReticuleDistClip = 50;


		player->ReticuleX = (player->x-player->camera.x)+(player->ReticuleDistClip*sin(player->ReticuleAngle));
		player->ReticuleY = ((player->y-player->camera.y)-(player->ReticuleDistClip*cos(player->ReticuleAngle)))-10;


		// Scan to make sure that reticule is not in the way
		lx = player->x;
		ly = player->y-10;

		for(i=0; i<player->ReticuleDistClip; i++) {

			cx = lx;
			cy = ly;
			lx += 1*sin(player->ReticuleAngle);
			ly -= 1*cos(player->ReticuleAngle);

			if (isSolid(tileRender.GetTile(lx/16, ly/16))) {
				player->ReticuleX = lx-player->camera.x;
				player->ReticuleY = ly-player->camera.y;
				break;
			}

		}

		/*gdl::DrawLine(
			-player->camera.x, cy-player->camera.y,
			lx-player->camera.x, ly-player->camera.y,
			gdl::Color::Yellow);
		*/

	}


}

void DrawPlayerSprite(playerStruct *player) {

	int i;

	// Draw character sprite
	if (player->dir < 0)
		i = RARITY_LEFT;
	if (player->dir >= 0)
		i = RARITY_RIGHT;

	if (player->dead == 0) {

		if (player->OnFloor) {

			if (abs(player->xMove) > 64) {

				if (abs(player->xMove) < (ONE*4)) {

					player->anim += (((float)abs(player->xMove)/ONE)/8)*gdl::Delta;

					if ((short)player->anim <= -1)
						player->anim = 7;
					if ((short)player->anim >= 8)
						player->anim = 0;

					i += RARITY_RIGHT_WALK+(short)player->anim;

				} else {

					player->anim += (((float)abs(player->xMove)/ONE)/16)*gdl::Delta;

					if ((short)player->anim <= -1)
						player->anim = 4;
					if ((short)player->anim >= 5)
						player->anim = 0;

					i += RARITY_RIGHT_DASH+(short)player->anim;

				}

			} else {

				if (player->input.pointerOn) {

					if ((player->y-player->camera.y) > (player->input.pointerY+30))
						i += RARITY_RIGHT_LOOK;
					else if ((player->y-player->camera.y) < (player->input.pointerY-50))
						i += RARITY_RIGHT_LOOK+1;

				}

			}

		} else {

			if (player->yMove < 0)
				i += 1;
			else
				i += 2;

		}

	} else {

		i += RARITY_RIGHT_DEAD;

	}

	sprites.Put(
		player->x-player->camera.x, player->y-player->camera.y, i, gdl::Color::White,
		gdl::Pivot, gdl::Pivot, 1.f, 0);

}

void PrepDisplay() {

	if (gdl::wii::IsWidescreen())
		ScreenSidePad = 24;
	else
		ScreenSidePad = 8;

	gdl::PrepDisplay();

}

void Display() {

	if (input::Wiimote[0].status != WPAD_ERR_NONE) {
		gdl::DrawBoxF(0, 0, gdl::ScreenXres, gdl::ScreenYres, RGBA(0, 0, 0, 91));
		font.Printf(gdl::Centered, gdl::ScreenCenterY-4, 1.f, gdl::Color::White, "Communication with the Wii Remote has been lost.");
		font.Printf(gdl::Centered, gdl::ScreenCenterY+4, 1.f, gdl::Color::White, "Make sure your Wii Remote has been paired to your console");
		font.Printf(gdl::Centered, gdl::ScreenCenterY+12, 1.f, gdl::Color::White, "and that it has batteries with enough power to make it work.");
	} else if ((input::Wiimote[0].data.exp.type != EXP_NUNCHUK) && (input::Wiimote[0].data.exp.type != EXP_CLASSIC)) {
		gdl::DrawBoxF(0, 0, gdl::ScreenXres, gdl::ScreenYres, RGBA(0, 0, 0, 91));
		font.Printf(gdl::Centered, gdl::ScreenCenterY-4, 1.f, gdl::Color::White, "You will need a Nunchuk or Classic Controller.");
	}


	gdl::Display();


	if (input::Wiimote[0].status != WPAD_ERR_NONE) {
		while(input::Wiimote[0].status != WPAD_ERR_NONE) {
			input::UpdateInput();
			VIDEO_WaitVSync();
		}
	} else {
		while((input::Wiimote[0].data.exp.type != EXP_NUNCHUK) && (input::Wiimote[0].data.exp.type != EXP_CLASSIC)) {
			input::UpdateInput();
			VIDEO_WaitVSync();
		}
	}


}
