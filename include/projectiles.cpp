// Bullet system struct and class
typedef struct {
	float	x,y;
	float	xMove,yMove;
	float	angle;
	int		type;
	float	counter;
	int		animFrame;
	float	animCount;
} bulletStruct;

class BulletClass {
public:

	int				BulletActive;
	bulletStruct	Bullet[MAX_PROJECTILES];

	enum BulletType {
		Dynamite = 1,
	};

	// Constructor
	BulletClass() {

		BulletActive = 0;
		memset(Bullet, 0x00, sizeof(BulletType)*MAX_PROJECTILES);

	}

	// Deconstructor
	virtual ~BulletClass() {

	}

	// Place a bullet
	void PutBullet(float x, float y, float xMove, float yMove, float angle, int type) {

		if (BulletActive >= MAX_PROJECTILES)
			return;

		Bullet[BulletActive].x = x;
		Bullet[BulletActive].y = y;
		Bullet[BulletActive].xMove = xMove;
		Bullet[BulletActive].yMove = yMove;
		Bullet[BulletActive].angle = angle;
		Bullet[BulletActive].type = type;
		Bullet[BulletActive].counter = 0;
		Bullet[BulletActive].animFrame = 0;
		Bullet[BulletActive].animCount = 0;
		BulletActive++;

	}

	// Process all bullets
	void DoBullets(playerStruct *player, ParticleClass *particles) {

		short cx,cy;
		short tx,ty;
		bool delEntry;

		for(int i=0; i<BulletActive; i++) {

			delEntry = false;

			switch(Bullet[i].type) {
			case BulletClass::Dynamite:

				if (!inbox(Bullet[i].x-player->camera.x, Bullet[i].y-player->camera.y, -256, -256, gdl::ScreenXres+256, gdl::ScreenYres+256)) {
					delEntry = true;
					break;
				}

				sprites.Put(
					Bullet[i].x-player->camera.x, Bullet[i].y-player->camera.y, DYNAMITE_LIT+Bullet[i].animFrame,
					gdl::Color::White, gdl::Pivot, gdl::Pivot,
					1.f, Bullet[i].angle);

				Bullet[i].x += Bullet[i].xMove;
				Bullet[i].y += Bullet[i].yMove;
				Bullet[i].angle += Bullet[i].xMove*4;
				Bullet[i].yMove += GRAVITY;


				if (isSolid(tileRender.CollideB(ceil(Bullet[i].x), ceil(Bullet[i].y), 0, 2, &cy))) {

					Bullet[i].y = cy;
					Bullet[i].xMove *= 0.8f;

					if ((Bullet[i].yMove*128) > 0)
						Bullet[i].yMove -= fabs(Bullet[i].yMove)*1.6f;

				}

				if (isSolid(tileRender.CollideT(Bullet[i].x, Bullet[i].y, 0, -4, &cy))) {

					Bullet[i].y = cy;
					Bullet[i].xMove *= 0.8f;

					Bullet[i].yMove = fabs(Bullet[i].yMove);

				}


				if (isSolid(tileRender.CollideL(Bullet[i].x, Bullet[i].y, -4, 0, &cx))) {
					Bullet[i].x = cx;
					Bullet[i].xMove -= Bullet[i].xMove*1.8f;
				}

				if (isSolid(tileRender.CollideR(Bullet[i].x, Bullet[i].y, 4, 0, &cx))) {
					Bullet[i].x = cx;
					Bullet[i].xMove -= Bullet[i].xMove*1.8f;
				}


				Bullet[i].animCount += 1*gdl::Delta;
				if (Bullet[i].animCount > 2) {

					Bullet[i].animFrame += 1;
					if (Bullet[i].animFrame >= 3)
						Bullet[i].animFrame = 0;

					Bullet[i].animCount = 0;

				}

				Bullet[i].counter += 1*gdl::Delta;
				if (Bullet[i].counter > 240) {

					sound[SND_EXPLODE].Play2D(
						1.f, 100.f,
						(Bullet[i].x-player->camera.x)-gdl::ScreenCenterX, (Bullet[i].y-player->camera.y)-gdl::ScreenCenterY);

					for(int c=0; c<16; c++) {

						float speed=1+(5*((float)rand()/RAND_MAX));
						float angle=360*((float)rand()/RAND_MAX);

						particles->PutParticle(Bullet[i].x, Bullet[i].y, speed*sin(angle*ROTPI), speed*cos(angle*ROTPI), 0, ParticleClass::ExplodeCloud);

					}

					tx = ceil(Bullet[i].x)/16;
					ty = ceil(Bullet[i].y)/16;

					for(cy=ty-2; cy<=ty+2; cy++) {
						for(cx=tx-2; cx<=tx+2; cx++) {

							DestroyTile(cx, cy, &tileRender, particles);

						}
					}

					delEntry=true;

				}


				break;

			}

			if (delEntry) {

				for(int c=i; c<BulletActive; c++) {
					Bullet[c] = Bullet[c+1];
				}
				BulletActive--;
				i--;

			}

		}

	}

};
