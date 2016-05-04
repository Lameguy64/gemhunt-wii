// Particle system struct and class
typedef struct {
	float x,y;
	float xMove,yMove;
	float angle;
	int	  type;
	float counter;
	int	  animFrame;
	float animCount;
} particleStruct;

class ParticleClass {
public:

	int				ParticleActive;
	particleStruct	Particle[MAX_PARTICLES];


	enum ParticleType {
		ExplodeCloud = 1,
		Rubble = 2,
		Next = 6,
	};


	ParticleClass() {

		ParticleActive = 0;
		memset(Particle, 0x00, sizeof(particleStruct)*MAX_PARTICLES);

	}

	virtual ~ParticleClass() {

	}

	void PutParticle(float x, float y, float xMove, float yMove, float angle, int type) {

		if (ParticleActive >= MAX_PARTICLES)
			return;

		Particle[ParticleActive].x = x;
		Particle[ParticleActive].y = y;
		Particle[ParticleActive].xMove = xMove;
		Particle[ParticleActive].yMove = yMove;
		Particle[ParticleActive].angle = angle;
		Particle[ParticleActive].type = type;
		Particle[ParticleActive].counter = 0;
		Particle[ParticleActive].animFrame = 0;
		Particle[ParticleActive].animCount = 0;
		ParticleActive++;

	}

	void DoParticles(playerStruct *player) {

		short cx,cy;
		bool delEntry;

		for(int i=0; i<ParticleActive; i++) {

			delEntry = false;

			switch(Particle[i].type) {
			case ParticleClass::ExplodeCloud:

				sprites.Put(Particle[i].x-player->camera.x, Particle[i].y-player->camera.y, EXPLODE_CLOUD+Particle[i].animFrame,
					gdl::Color::White, gdl::Pivot, gdl::Pivot, 1.f, Particle[i].angle);

				Particle[i].x += Particle[i].xMove;
				Particle[i].y += Particle[i].yMove;

				Particle[i].animCount += 1*gdl::Delta;

				if (Particle[i].animCount > 8) {

					Particle[i].animFrame += 1;
					if (Particle[i].animFrame >= 4)
						delEntry = true;

					Particle[i].animCount = 0;

				}

				break;

			case ParticleClass::Rubble:
			case ParticleClass::Rubble+1:
			case ParticleClass::Rubble+2:
			case ParticleClass::Rubble+3:

				if (!inbox(Particle[i].x-player->camera.x, Particle[i].y-player->camera.y, -128, -128, gdl::ScreenXres+128, gdl::ScreenYres+128)) {
					delEntry = true;
					break;
				}

				sprites.Put(
					Particle[i].x-player->camera.x, Particle[i].y-player->camera.y, RUBBLE_SPRITES+(Particle[i].type-ParticleClass::Rubble),
					gdl::Color::White, gdl::Pivot, gdl::Pivot, 1.f, Particle[i].angle);


				Particle[i].x += Particle[i].xMove;
				Particle[i].y += Particle[i].yMove;
				Particle[i].angle += Particle[i].xMove*4;
				Particle[i].yMove += GRAVITY;


				if (isSolid(tileRender.CollideB(ceil(Particle[i].x), ceil(Particle[i].y), 0, 2, &cy))) {

					Particle[i].y = cy;
					Particle[i].xMove *= 0.8f;

					if ((Particle[i].yMove*128) > 0)
						Particle[i].yMove -= fabs(Particle[i].yMove)*1.6f;

				}

				if (isSolid(tileRender.CollideT(Particle[i].x, Particle[i].y, 0, -4, &cy))) {

					Particle[i].y = cy;
					Particle[i].xMove *= 0.8f;

					Particle[i].yMove = fabs(Particle[i].yMove);

				}

				if (isSolid(tileRender.CollideL(Particle[i].x, Particle[i].y, -4, 0, &cx))) {
					Particle[i].x = cx;
					Particle[i].xMove -= Particle[i].xMove*1.8f;
				}

				if (isSolid(tileRender.CollideR(Particle[i].x, Particle[i].y, 4, 0, &cx))) {
					Particle[i].x = cx;
					Particle[i].xMove -= Particle[i].xMove*1.8f;
				}

				Particle[i].counter += 1*gdl::Delta;
				if (Particle[i].counter > 240)
					delEntry = true;


				break;

			}

			if (delEntry) {

				for(int c=i; c<ParticleActive; c++) {
					Particle[c] = Particle[c+1];
				}
				ParticleActive--;
				i--;

			}

		}

	}

};
