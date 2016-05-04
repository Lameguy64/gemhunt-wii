#define TR_ERROR_OK	0

namespace tr {
	
	typedef struct {
		float u0,v0;
		float u1,v1;
		float u2,v2;
		float u3,v3;
	} _tileCoords;
	
	class TileSet {
	public:
		
		GXTexObj	*texObj;				// Texture object to tile sheet
		
		void		*tcoordList;			// Texture coordinate list
		short		tileXsize,tileYsize;	// Size of tiles
		int			numTiles;				// Number of tiles in tile sheet
		
		TileSet() {
			
			texObj=NULL;
			
			tcoordList=NULL;
			tileXsize=0;
			tileYsize=0;
			numTiles=0;
			
		}
		
		virtual ~TileSet() {
			
			if (tcoordList != NULL) free(tcoordList);
			
		}
		
		void Free() {
			
			if (tcoordList != NULL) free(tcoordList);
			TileSet();
			
		}
		
		void Create(GXTexObj *texObj, short sheetXsize, short sheetYsize, short tileXsize, short tileYsize) {
			
			short txSize = sheetXsize/tileXsize;
			short tySize = sheetYsize/tileYsize;
			
			TileSet::numTiles = txSize*tySize;
			TileSet::tcoordList = memalign(32, sizeof(_tileCoords)*numTiles);
			
			_tileCoords *tCoords = (_tileCoords*)tcoordList;
			
			for(short ty=0; ty<tySize; ty++) {
				for(short tx=0; tx<txSize; tx++) {
					
					int i=tx+(txSize*ty);
					
					tCoords[i].u0 = ((float)(tileXsize*tx)+0.01f)     /sheetXsize;
					tCoords[i].v0 = ((float)(tileYsize*ty)+0.01f)     /sheetYsize;
					tCoords[i].u1 = (float)((tileXsize*tx)+tileXsize) /sheetXsize;
					tCoords[i].v1 = (float)((tileYsize*ty)+0.01f)     /sheetYsize;
					tCoords[i].u2 = (float)((tileXsize*tx)+tileXsize) /sheetXsize;
					tCoords[i].v2 = (float)((tileYsize*ty)+tileYsize) /sheetYsize;
					tCoords[i].u3 = ((float)(tileXsize*tx)+0.01f)     /sheetXsize;
					tCoords[i].v3 = (float)((tileYsize*ty)+tileYsize) /sheetYsize;
					
				}
			}
			
			DCFlushRange(tcoordList, sizeof(_tileCoords)*numTiles);
			
			TileSet::texObj = texObj;
			TileSet::tileXsize = tileXsize;
			TileSet::tileYsize = tileYsize;
			
		}
		
	};
	
	
	// Tile render class
	class TileRender {
		
		short	tileXsize,tileYsize;	// Tile size in pixels
		
		short	mapXsize,mapYsize;		// Tile map size in tile units
		short	mapFmt;					// Tile map format (0 - 8-bit, 1 - 16-bit)
		void	*mapPtr;				// Tile map data pointer (will not be freed when class is deconstructed)
		
		short	drawXpos,drawYpos;
		short	drawXsize,drawYsize;
		short	drawXrange,drawYrange;
		void	*vertList;
		
		TileSet	*tileSet;
		
	public:
		
		
		// Constructor
		TileRender() {
			
			tileXsize=0;
			tileYsize=0;
			
			mapXsize=0;
			mapYsize=0;
			mapFmt=0;
			mapPtr=NULL;
			
			drawXsize=0;
			drawYsize=0;
			vertList=NULL;
			
		}
		
		// Secondary constructor overload
		TileRender(short renderXsize, short renderYsize, short tileXsize, short tileYsize) {
			
			Create(renderXsize, renderYsize, tileXsize, tileYsize);
			
		}
		
		// Deconstructor
		virtual ~TileRender() {
			
			if (vertList != NULL) free(vertList);
			
		}
		
		void Free() {
			
			if (vertList != NULL) free(vertList);
			TileRender();
			
		}
		
		// Create a tile render instance
		int Create(short renderXsize, short renderYsize, short tileXsize, short tileYsize) {
			
			// Set necessary parameters for the rendering class
			TileRender::tileXsize = tileXsize;
			TileRender::tileYsize = tileYsize;
			
			drawXsize = renderXsize;
			drawYsize = renderYsize;
			drawXrange = ((renderXsize+(tileXsize-1))/tileXsize)+2;
			drawYrange = ((renderYsize+(tileYsize-1))/tileYsize)+2;
			
			
			// Generate vertex array of a grid pattern for drawing the tiles
			vertList = memalign(32, sizeof(gdl::wii::VERT2s16)*(drawXrange*drawYrange));
			gdl::wii::VERT2s16 *vertexList = (gdl::wii::VERT2s16*)TileRender::vertList;
			
			for(short iy=0; iy<drawYrange; iy++) {
				for(short ix=0; ix<drawXrange; ix++) {
					
					short i=ix+(drawXrange*iy);
					vertexList[i].x = tileXsize*ix;
					vertexList[i].y = tileYsize*iy;
					
				}
			}
			
			DCFlushRange(vertList, sizeof(gdl::wii::VERT2s16)*(drawXrange*drawYrange));
			
			return(TR_ERROR_OK);
			
		}
		
		void SetMap(short mapXsize, short mapYsize, void *mapPtr, short mapFmt) {
			
			TileRender::mapXsize	= mapXsize;
			TileRender::mapYsize	= mapYsize;
			TileRender::mapPtr		= mapPtr;
			TileRender::mapFmt		= mapFmt;
			
		}
		
		void SetTileSet(TileSet &tileSet) {
			
			TileRender::tileSet = &tileSet;
			
		}
		
		// Draw tile map
		void Render(int scrollX, int scrollY) {
			
			if (vertList == NULL)	return;
			if (tileSet == NULL)	return;
			
			
			GX_LoadTexObj(tileSet->texObj, GX_TEXMAP0);
			GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
			
			GX_ClearVtxDesc();
			GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
			GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
			
			GX_SetArray(GX_VA_POS, vertList, 2*sizeof(s16));
			GX_SetArray(GX_VA_TEX0, tileSet->tcoordList, 2*sizeof(f32));
			
			
			short ttx=scrollX/tileXsize;
			short tty=scrollY/tileYsize;
			
			
			Mtx tempMatrix;
			guMtxApplyTrans(gdl::wii::ModelMtx, tempMatrix, -(scrollX%tileXsize), -(scrollY%tileYsize), 0);
			GX_LoadPosMtxImm(tempMatrix, GX_PNMTX0);
			
			
			GX_Begin(GX_QUADS, GX_VTXFMT0, 4*((drawXrange-1)*(drawYrange-1)));
			
			if (mapPtr != NULL) {
				
				if (mapFmt == 0) {
					
					for(short ty=0; ty<drawYrange-1; ty++) {
						for(short tx=0; tx<drawXrange-1; tx++) {
							
							u_short	vi = tx+(drawXrange*ty);
							u_int	toffs = (ttx+tx)+(mapXsize*(tty+ty));
							
							if (((ttx+tx>=0) && (tty+ty>=0)) && 
								((ttx+tx<mapXsize) && (tty+ty<mapYsize))) {
								
								if (((u_char*)mapPtr)[toffs]) {
									
									u_short ti = 4*(((u_char*)mapPtr)[toffs]-1);
									
									GX_Position1x16(vi);
									GX_TexCoord1x16(ti);
									
									GX_Position1x16(vi+1);
									GX_TexCoord1x16(ti+1);
									
									GX_Position1x16(vi+(drawXrange+1));
									GX_TexCoord1x16(ti+2);
									
									GX_Position1x16(vi+drawXrange);
									GX_TexCoord1x16(ti+3);
									
								} else {
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
								}
								
							} else {
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
							}
							
						}
					}
					
				} else {
					
					for(short ty=0; ty<drawYrange-1; ty++) {
						for(short tx=0; tx<drawXrange-1; tx++) {
							
							u_short	vi = tx+(drawXrange*ty);
							u_int	toffs = (ttx+tx)+(mapXsize*(tty+ty));
							
							if (((ttx+tx>=0) && (tty+ty>=0)) && 
								((ttx+tx<mapXsize) && (tty+ty<mapYsize))) {
								
								if (((u_short*)mapPtr)[toffs]) {
									
									u_short ti = 4*(((u_short*)mapPtr)[toffs]-1);
									
									GX_Position1x16(vi);
									GX_TexCoord1x16(ti);
									
									GX_Position1x16(vi+1);
									GX_TexCoord1x16(ti+1);
									
									GX_Position1x16(vi+(drawXrange+1));
									GX_TexCoord1x16(ti+2);
									
									GX_Position1x16(vi+drawXrange);
									GX_TexCoord1x16(ti+3);
									
								} else {
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
									GX_Position1x16(0);
									GX_TexCoord1x16(0);
									
								}
								
							} else {
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
								GX_Position1x16(0);
								GX_TexCoord1x16(0);
								
							}
							
						}
					}
					
				}
				
			} else {
				
				for(short ty=0; ty<drawYrange-1; ty++) {
					for(short tx=0; tx<drawXrange-1; tx++) {
						
						u_short	vi = tx+(drawXrange*ty);
						
						if (((ttx+tx>=0) && (tty+ty>=0)) && 
							((ttx+tx<mapXsize) && (tty+ty<mapYsize))) {
							
							GX_Position1x16(vi);
							GX_TexCoord1x16(0);
							
							GX_Position1x16(vi+1);
							GX_TexCoord1x16(1);
							
							GX_Position1x16(vi+(drawXrange+1));
							GX_TexCoord1x16(2);
							
							GX_Position1x16(vi+drawXrange);
							GX_TexCoord1x16(3);
							
						} else {
							
							GX_Position1x16(0);
							GX_TexCoord1x16(0);
							
							GX_Position1x16(0);
							GX_TexCoord1x16(0);
							
							GX_Position1x16(0);
							GX_TexCoord1x16(0);
							
							GX_Position1x16(0);
							GX_TexCoord1x16(0);
							
						}
						
					}
				}
				
			}
			
			GX_End();
			
			GX_LoadPosMtxImm(gdl::wii::ModelMtx, GX_PNMTX0);
			
		}
		
		
		u_short	GetTile(short x, short y) {
			
			if ((x < 0) || (y < 0)) return(0);
			if ((x > mapXsize) || (y > mapYsize)) return(0);
			
			int toffs=x+(mapXsize*y);
			
			if (mapFmt == 0)
				return(((u_char*)mapPtr)[toffs]);
			else
				return(((u_short*)mapPtr)[toffs]);
			
		}
		
		void SetTile(short x, short y, u_short value) {
			
			if ((x < 0) || (y < 0)) return;
			if ((x > mapXsize) || (y > mapYsize)) return;
			
			int toffs=x+(mapXsize*y);
			
			if (mapFmt == 0)
				((u_char*)mapPtr)[toffs] = value;
			else
				((u_short*)mapPtr)[toffs] = value;
			
		}
		
		u_short CollideL(short x, short y, short mx, short my, short *cx) {
			
			if (((x+mx) < 0) || ((y+my) < 0))
				return(0);
			if ((((x+mx)+tileXsize)>=(tileXsize*(mapXsize+1))) || (((y+my)+tileYsize)>=(tileYsize*(mapYsize+1))))
				return(0);
			
			short	tx=(x+mx)/tileXsize,ty=(y+my)/tileYsize;
			u_short	tile;
			
			if ((tile = GetTile(tx, ty))) {
				*cx = (tileXsize*(tx+1))-mx;
				return(tile);
			}
			
			return(0);
			
		}
		
		u_short CollideR(short x, short y, short mx, short my, short *cx) {
			
			if (((x+mx) < 0) || ((y+my) < 0))
				return(0);
			if ((((x+mx)+tileXsize)>=(tileXsize*(mapXsize+1))) || (((y+my)+tileYsize)>=(tileYsize*(mapYsize+1))))
				return(0);
			
			short	tx=(x+mx)/tileXsize,ty=(y+my)/tileYsize;
			u_short	tile;
			
			if ((tile = GetTile(tx, ty))) {
				*cx = (tileXsize*tx)-mx;
				return(tile);
			}
			
			return(0);
			
		}
		
		u_short CollideT(short x, short y, short mx, short my, short *cy) {
			
			if (((x+mx) < 0) || ((y+my) < 0))
				return(0);
			if ((((x+mx)+tileXsize)>=(tileXsize*(mapXsize+1))) || (((y+my)+tileYsize)>=(tileYsize*(mapYsize+1))))
				return(0);
			
			short	tx=(x+mx)/tileXsize,ty=(y+my)/tileYsize;
			u_short	tile;
			
			if ((tile = GetTile(tx, ty))) {
				*cy = (tileYsize*(ty+1))-my;
				return(tile);
			}
			
			return(0);
			
		}
		
		u_short CollideB(short x, short y, short mx, short my, short *cy) {
			
			if (((x+mx) < 0) || ((y+my) < 0))
				return(0);
			if ((((x+mx)+tileXsize)>=(tileXsize*(mapXsize+1))) || (((y+my)+tileYsize)>=(tileYsize*(mapYsize+1))))
				return(0);
			
			short	tx=(x+mx)/tileXsize,ty=(y+my)/tileYsize;
			int		tile;
			
			if ((tile = GetTile(tx, ty))) {
				*cy = (tileYsize*ty)-my;
				return(tile);
			}
			
			return(0);
			
		}
		
		
		short MapXsize() {
			
			return(mapXsize);
			
		}
		
		short MapYsize() {
			
			return(mapYsize);
			
		}
		
	};
	
};