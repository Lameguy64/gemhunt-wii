namespace osk {
	
	bool		 Active			= false;
	bool		 FullyInactive	= true;
	
	static int	 _CursorPos		= 0;
	static float _Counter		= 0;
	static char  *_TextPtr		= NULL;
	static char  *_TempTextPtr	= NULL;
	static int	 _TextMaxLen	= 0;
	static int	 _KeyMode		= 0;
	
	
	
	void Activate(char *text, int maxLen, bool retain) {
		
		Active		= true;
		_Counter	= 1.f;
		_KeyMode	= 0;
		
		_TextPtr		= text;
		_TempTextPtr	= (char*)malloc(maxLen);
		_TextMaxLen		= maxLen;
		
		if (retain) {
			strcpy(_TempTextPtr, text);
			_CursorPos = strlen(_TempTextPtr);
		} else {
			memset(_TempTextPtr, 0x00, maxLen);
			_CursorPos = 0;
		}
		
	}
	
	void Deactivate() {
		
		Active=false;
		
	}
	
	bool _InBox(short x, short y, short x1, short y1, short x2, short y2) {
		
		if ((x >= x1) && (x <= x2)) {
			if ((y >= y1) && (y <= y2)) {
				return(true);
			}
		}
		
		return(false);
		
	}
	
	void _PutKey(short x, short y, float scale, int index, char c, u_int col) {
		
		Mtx oldMatrix,tempMatrix;
		
		guMtxCopy(gdl::wii::ModelMtx, oldMatrix);
		guMtxApplyTrans(gdl::wii::ModelMtx, gdl::wii::ModelMtx, x, y, 0);
		
		guMtxScaleApply(oldMatrix, tempMatrix, scale, scale, 1);
		guMtxConcat(gdl::wii::ModelMtx, tempMatrix, gdl::wii::ModelMtx);
		
		
		sprites.Put(
			-sprites.SpriteInfo(index)->cx, 
			-sprites.SpriteInfo(index)->cy,
			index,
			col);
		
		if (c)
			font.Printf(-2, -3, 1.f, gdl::Color::Black, "%c", c);
		
		guMtxCopy(oldMatrix, gdl::wii::ModelMtx);
		GX_LoadPosMtxImm(gdl::wii::ModelMtx, GX_PNMTX0);
		
	}
	
	void Draw() {
		
		static float blinkCount		= 0;
		static bool	 blink			= false;
		static float keyScale[54]	= {1.f};
		static int	 keyPressed		= 0;
		static int	 oldKeyPressed	= 0;
		static char	 keyPressedChar	= 0;
		static bool	 keyHeld		= false;
		static bool	 textBoxClicked	= false;
		
		static int	px,py,ps,pi,pcol;
		static char pc=0;
		
		float s;
		int i,c,p,kx,ky,oskYpos;
		
		
		char *charTable=NULL;
		
		char charTableNorm[] = {
			'1','2','3','4','5','6','7','8','9','0','-',
			'q','w','e','r','t','y','u','i','o','p',
			'a','s','d','f','g','h','j','k','l',':',
			'z','x','c','v','b','n','m',',','.','=',
			'[',']', 39,'`','/','@'
		};
		
		char charTableShift[] = {
			'!','\\','#','$','%','^','&','*','(',')','_',
			'Q','W','E','R','T','Y','U','I','O','P',
			'A','S','D','F','G','H','J','K','L',';',
			'Z','X','C','V','B','N','M','<','>','+',
			'{','}', '"','?','~','|'
		};
		
		char charTableCaps[] = {
			'1','2','3','4','5','6','7','8','9','0','-',
			'Q','W','E','R','T','Y','U','I','O','P',
			'A','S','D','F','G','H','J','K','L',':',
			'Z','X','C','V','B','N','M',',','.','=',
			'[',']', 39,'`','/','@'
		};
		
		
		FullyInactive = true;
		
		if (trunc(_Counter)) {
			
			FullyInactive = false;
			
			s = (_Counter/30);
			
			
			// Draw the keyboard background
			gdl::DrawBoxF(
				0, 0, gdl::ScreenXres, gdl::ScreenYres, 
				RGBA(0, 0, 0, 191*s));
			
			oskYpos = gdl::ScreenYres-((gdl::ScreenCenterY+67)*sin((90*s)*ROTPI));
			
			gdl::DrawBoxF(
				gdl::ScreenCenterX-130, oskYpos+9, gdl::ScreenCenterX+130, oskYpos+127,
				RGBA(255, 228, 128, 255));
			
			
			gdl::DrawBox(
				gdl::ScreenCenterX-130, oskYpos+7, gdl::ScreenCenterX+130, oskYpos+128,
				RGBA(211, 181, 111, 255));
			gdl::DrawBox(
				gdl::ScreenCenterX-130, oskYpos+8, gdl::ScreenCenterX+130, oskYpos+129,
				RGBA(211, 181, 111, 255));
			
			sprites.Put(gdl::ScreenCenterX-150, oskYpos, OSK_SIDES, gdl::Color::White);
			sprites.Put(gdl::ScreenCenterX+(150-28), oskYpos, OSK_SIDES+1, gdl::Color::White);
			
			
			switch(_KeyMode) {
			case 0:
				charTable = &charTableNorm[0];
				break;
			case 1:
				charTable = &charTableShift[0];
				break;
			case 2:
				charTable = &charTableCaps[0];
				break;
			}
			
			
			if (input::Wiimote[0].data.btns_h & WPAD_BUTTON_B) {
				
				if (_KeyMode == 0)
					charTable = &charTableShift[0];
				else 
					charTable = &charTableNorm[0];
				
			}
			
			
			if (input::Wiimote[0].data.btns_h & WPAD_BUTTON_A) {
				
				if (keyHeld == false) {
					
					// Caps lock
					if (keyPressed == 24) {
						
						if (_KeyMode == 1) 
							_KeyMode = 2;
						else if (_KeyMode == 0)
							_KeyMode = 2;
						else
							_KeyMode = 0;
						
					// Shift
					} else if (keyPressed == 35) {
						
						if (_KeyMode >= 2)
							_KeyMode = 1;
						else
							_KeyMode = (_KeyMode+1)%2;
						
					// Backspace
					} else if ((keyPressed == 12) && (strlen(_TempTextPtr)>0) && (_CursorPos > 0)) {
						
						if (_CursorPos == (int)strlen(_TempTextPtr)) {
							
							_TempTextPtr[strlen(_TempTextPtr)-1] = 0x00;
							
						} else {
							
							for(i=_CursorPos-1; i<(int)strlen(_TempTextPtr); i++) {
								_TempTextPtr[i] = _TempTextPtr[i+1];
							}
							
						}
						
						if (_CursorPos > 0)
							_CursorPos -= 1;
						
					// Spacebar
					} else if ((keyPressed == 48) && (strlen(_TempTextPtr)<(u_int)_TextMaxLen)) {
						
						if (_CursorPos == (int)strlen(_TempTextPtr)) {
							
							strncat(_TempTextPtr, " ", 1);
							
						} else {
							
							for(i=(int)strlen(_TempTextPtr)+1; i>_CursorPos; i--) {
								_TempTextPtr[i] = _TempTextPtr[i-1];
							}
							_TempTextPtr[_CursorPos] = ' ';
							
						}
						
						_CursorPos += 1;
						
					// Cancel
					} else if (keyPressed == 53) {
						
						Active = false;
						
					// Okay/Enter
					} else if ((keyPressed == 54) || (keyPressed == 23)) {
						
						Active = false;
						strcpy(_TextPtr, _TempTextPtr);
						
					// Any other key
					} else if ((keyPressed) && (strlen(_TempTextPtr)<(u_int)_TextMaxLen)) {
						
						if (keyPressed != 12) {
							
							if (_CursorPos == (int)strlen(_TempTextPtr)) {
								
								strncat(_TempTextPtr, &keyPressedChar, 1);
								
							} else {
								
								for(i=(int)strlen(_TempTextPtr)+1; i>_CursorPos; i--) {
									_TempTextPtr[i] = _TempTextPtr[i-1];
								}
								_TempTextPtr[_CursorPos] = keyPressedChar;
								
							}
							
							_CursorPos += 1;
							
							if (_KeyMode == 1)
								_KeyMode = 0;
							
						}
						
					}
					
					if (keyPressed)
						sound[SND_KEYPRESS].Play(1.f, 100);
					
				}
				
				keyHeld = true;
				
			} else {
				
				keyHeld = false;
				
			}
			
			
			pcol = RGBA(255, 255, 255, 255);
			
			for(i=0; i<54; i++) {
				
				if ((i+1) == keyPressed) {
					
					if (keyHeld) {
						keyScale[i] = 0.9f;
						pcol = RGBA(191, 191, 191, 255);
					} else {
						keyScale[i] += ((1.5f-keyScale[i])/2)*gdl::Delta;
					}
					
				} else {
					keyScale[i] += ((1.f-keyScale[i])/2)*gdl::Delta;
				}
				
			}
			
			
			// Numbers & backspace
			if (keyHeld == false) {
				keyPressed = 0;
				pi = 0;
			}
			
			kx = gdl::ScreenCenterX-117;
			ky = oskYpos+30;
			c = 0;
			p = 0;
			
			for(i=0; i<11; i++) {
				
				_PutKey(kx+8, ky+6, keyScale[p], OSK_KEY, charTable[c], gdl::Color::White);
				
				if (keyHeld == false) {
					if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
						kx, ky, 
						kx+(sprites.SpriteInfo(OSK_KEY)->w-1), 
						ky+(sprites.SpriteInfo(OSK_KEY)->h-1))
						) {
						
						px = kx+8;
						py = ky+6;
						pi = OSK_KEY;
						ps = p;
						pc = charTable[c];
						
						keyPressed = 1+p;
						keyPressedChar = charTable[c];
						
					}
				}
				
				p++; c++;
				kx += 19;
			}
			
			_PutKey(kx+12, ky+6, keyScale[p], OSK_BACKSPACE, 0, gdl::Color::White);
			
			// Backspace
			if (keyHeld == false) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					kx, ky, 
					kx+(sprites.SpriteInfo(OSK_BACKSPACE)->w-1), 
					ky+(sprites.SpriteInfo(OSK_BACKSPACE)->h-1))
					) {
					
					px = kx+12;
					py = ky+6;
					pi = OSK_BACKSPACE;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
				
			}
			
			p++;
			ky += 15;
			
			
			// First key row & enter
			kx = gdl::ScreenCenterX-113;
			for(i=0; i<10; i++) {
				
				_PutKey(kx+8, ky+6, keyScale[p], OSK_KEY, charTable[c], gdl::Color::White);
				
				if (keyHeld == false) {
					if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
						kx, ky, 
						kx+(sprites.SpriteInfo(OSK_KEY)->w-1), 
						ky+(sprites.SpriteInfo(OSK_KEY)->h-1))
						) {
						
						px = kx+8;
						py = ky+6;
						pi = OSK_KEY;
						ps = p;
						pc = charTable[c];
						
						keyPressed = 1+p;
						keyPressedChar = charTable[c];
						
					}
				}
				
				p++;
				c++;
				kx+=19;
				
			}
			
			_PutKey(kx+19, ky+6, keyScale[p], OSK_ENTER, 0, gdl::Color::White);
			
			// Enter
			if (keyHeld == false) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					kx, ky, 
					kx+(sprites.SpriteInfo(OSK_ENTER)->w-1), 
					ky+(sprites.SpriteInfo(OSK_ENTER)->h-1))
					) {
					
					px = kx+19;
					py = ky+6;
					pi = OSK_ENTER;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
				
			}
			
			p++;
			ky += 15;
			
			
			// Second key row
			if (_KeyMode == 2)
				_PutKey(gdl::ScreenCenterX-110, ky+6, keyScale[p], OSK_CAPSLOCK, 0, RGBA(191, 191, 191, 255));
			else
				_PutKey(gdl::ScreenCenterX-110, ky+6, keyScale[p], OSK_CAPSLOCK, 0, gdl::Color::White);
			
			// Caps lock
			if (keyHeld == false) {
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					(gdl::ScreenCenterX-117), ky, 
					(gdl::ScreenCenterX-117)+(sprites.SpriteInfo(OSK_CAPSLOCK)->w-1), 
					ky+(sprites.SpriteInfo(OSK_CAPSLOCK)->h-1))
					) {
					
					px = gdl::ScreenCenterX-110;
					py = ky+6;
					pi = OSK_CAPSLOCK;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
			}
			
			p++;
			
			kx = gdl::ScreenCenterX-101;
			for(i=0; i<10; i++) {
				
				_PutKey(kx+8, ky+6, keyScale[p], OSK_KEY, charTable[c], gdl::Color::White);
				
				if (keyHeld == false) {
					
					if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
						kx, ky, 
						kx+(sprites.SpriteInfo(OSK_KEY)->w-1), 
						ky+(sprites.SpriteInfo(OSK_KEY)->h-1))
						) {
						
						px = kx+8;
						py = ky+6;
						pi = OSK_KEY;
						ps = p;
						pc = charTable[c];
						
						keyPressed = 1+p;
						keyPressedChar = charTable[c];
						
					}
					
				}
				
				p++;
				c++;
				kx+=19;
				
			}
			ky += 15;
			
			
			// Third key row
			if (_KeyMode != 1) {
				_PutKey(gdl::ScreenCenterX-106, ky+6, keyScale[p], OSK_SHIFT, 0, gdl::Color::White);
			} else {
				_PutKey(gdl::ScreenCenterX-106, ky+6, keyScale[p], OSK_SHIFT, 0, RGBA(191, 191, 191, 255));
			}
			
			if (keyHeld == false) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					(gdl::ScreenCenterX-117), ky, 
					(gdl::ScreenCenterX-117)+(sprites.SpriteInfo(OSK_SHIFT)->w-1), 
					ky+(sprites.SpriteInfo(OSK_SHIFT)->h-1))
					) {
					
					px = gdl::ScreenCenterX-106;
					py = ky+6;
					pi = OSK_SHIFT;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
				
			}
			p++;
			
			kx = gdl::ScreenCenterX-93;
			for(i=0; i<10; i++) {
				
				_PutKey(kx+8, ky+6, keyScale[p], OSK_KEY, charTable[c], gdl::Color::White);
				
				if (keyHeld == false) {
					if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
						kx, ky, 
						kx+(sprites.SpriteInfo(OSK_KEY)->w-1), 
						ky+(sprites.SpriteInfo(OSK_KEY)->h-1))
						) {
						
						px = kx+8;
						py = ky+6;
						pi = OSK_KEY;
						ps = p;
						pc = charTable[c];
						
						keyPressed = 1+p;
						keyPressedChar = charTable[c];
						
					}
				}
				
				p++;
				c++;
				kx+=19;
				
			}
			
			ky += 15;
			
			
			// Forth and final row
			kx = gdl::ScreenCenterX-93;
			for(i=0; i<2; i++) {
				
				_PutKey(kx+8, ky+6, keyScale[p], OSK_KEY, charTable[c], gdl::Color::White);
				
				if (keyHeld == false) {
					if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
						kx, ky, 
						kx+(sprites.SpriteInfo(OSK_KEY)->w-1), 
						ky+(sprites.SpriteInfo(OSK_KEY)->h-1))
						) {
						
						px = kx+8;
						py = ky+6;
						pi = OSK_KEY;
						ps = p;
						pc = charTable[c];
						
						keyPressed = 1+p;
						keyPressedChar = charTable[c];
						
					}
				}
				
				p++;
				c++;
				kx+=19;
				
			}
			
			_PutKey(kx+44, ky+6, keyScale[p], OSK_SPACEBAR, 0, gdl::Color::White);
			
			if (keyHeld == false) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					kx, ky, 
					kx+(sprites.SpriteInfo(OSK_SPACEBAR)->w-1), 
					ky+(sprites.SpriteInfo(OSK_SPACEBAR)->h-1))
					) {
					
					px = kx+44;
					py = ky+6;
					pi = OSK_SPACEBAR;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
				
			}
			p++;
			
			kx += sprites.SpriteInfo(OSK_SPACEBAR)->w+1;
			for(i=0; i<4; i++) {
				
				_PutKey(kx+8, ky+6, keyScale[p], OSK_KEY, charTable[c], gdl::Color::White);
				
				if (keyHeld == false) {
					if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
						kx, ky, 
						kx+(sprites.SpriteInfo(OSK_KEY)->w-1), 
						ky+(sprites.SpriteInfo(OSK_KEY)->h-1))
						) {
						
						px = kx+8;
						py = ky+6;
						pi = OSK_KEY;
						ps = p;
						pc = charTable[c];
						
						keyPressed = 1+p;
						keyPressedChar = charTable[c];
						
					}
				}
				
				p++;
				c++;
				kx+=19;
				
			}
			
			
			// Cancel
			_PutKey(gdl::ScreenCenterX-40, oskYpos+115, keyScale[p], OSK_CANCEL, 0, gdl::Color::White);
			
			kx = (gdl::ScreenCenterX-40)-sprites.SpriteInfo(OSK_CANCEL)->cx;
			ky = (oskYpos+115)-sprites.SpriteInfo(OSK_CANCEL)->cy;
			
			if (keyHeld == false) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					kx, ky, 
					kx+(sprites.SpriteInfo(OSK_CANCEL)->w-1), 
					ky+(sprites.SpriteInfo(OSK_CANCEL)->h-1))
					) {
					
					px = gdl::ScreenCenterX-40;
					py = oskYpos+115;
					pi = OSK_CANCEL;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
				
			}
			
			p++;
			
			// Okay
			_PutKey(gdl::ScreenCenterX+40, oskYpos+115, keyScale[p], OSK_OK, 0, gdl::Color::White);
			
			kx = (gdl::ScreenCenterX+40)-sprites.SpriteInfo(OSK_CANCEL)->cx;
			ky = (oskYpos+115)-sprites.SpriteInfo(OSK_CANCEL)->cy;
			
			if (keyHeld == false) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					kx, ky, 
					kx+(sprites.SpriteInfo(OSK_OK)->w-1), 
					ky+(sprites.SpriteInfo(OSK_OK)->h-1))
					) {
					
					px = gdl::ScreenCenterX+40;
					py = oskYpos+115;
					pi = OSK_OK;
					ps = p;
					pc = 0;
					
					keyPressed = 1+p;
					
				}
				
			}
			
			p++;
			
			
			if ((keyPressed) && (keyPressed != oldKeyPressed)) {
				if (trunc(_Counter) == 30) {
					
					sound[SND_KEYPASS].Play(1.f, 100);
					
					if (input::Wiimote[0].rumbleTime == 0)
						input::Wiimote[0].rumbleTime = 5;
					
				}
			}
			
			oldKeyPressed = keyPressed;
			
			
			gdl::DrawLine(gdl::ScreenCenterX-113, oskYpos+23, gdl::ScreenCenterX+113, oskYpos+23, RGBA(211, 181, 111, 255));
			gdl::DrawLine(gdl::ScreenCenterX-101, oskYpos+24, gdl::ScreenCenterX+101, oskYpos+24, RGBA(211, 181, 111, 255));
			font.Printf(gdl::ScreenCenterX-113, oskYpos+15, 1.f, gdl::Color::Black, _TempTextPtr);
			
			
			if (input::Wiimote[0].data.btns_h & WPAD_BUTTON_A) {
				
				if (_InBox(input::Wiimote[0].data.ir.x, input::Wiimote[0].data.ir.y,
					gdl::ScreenCenterX-120, oskYpos+12, 
					gdl::ScreenCenterX+120, oskYpos+24)) {
					
					textBoxClicked = true;
					
				}
				
			} else {
				
				textBoxClicked = false;
				
			}
			
			
			font.Printf(gdl::ScreenCenterX-105, oskYpos+114, 1.f, gdl::Color::Black, "%d/%d", strlen(_TempTextPtr), _TextMaxLen);
			
			
			if (textBoxClicked) {
				
				_CursorPos = -1;
				for(c=0; c<(int)strlen(_TempTextPtr)+1; c++) {
					
					i = (gdl::ScreenCenterX-113)+font.CalcStrLen(_TempTextPtr, c+1);
					if (i >= input::Wiimote[0].data.ir.x) {
						_CursorPos = c;
						break;
					}
					
				}
				
				if (_CursorPos == -1) {
					if (input::Wiimote[0].data.ir.x >= i) {
						_CursorPos = strlen(_TempTextPtr);
					} else {
						_CursorPos = 0;
					}
				}
				
			}
			
			
			if (blink) {
				
				i = font.CalcStrLen(_TempTextPtr, _CursorPos);
				
				gdl::DrawBoxF(
					(gdl::ScreenCenterX-113)+(i+1), oskYpos+14, 
					(gdl::ScreenCenterX-113)+(i+2), oskYpos+21, 
					gdl::Color::Black);
				
			}
			
			
			if ((pi) && (Active))
				_PutKey(px, py, keyScale[ps], pi, pc, pcol);
			
			
			if (Active) {
				
				if (trunc(_Counter)<30)
					_Counter += 1.f*gdl::Delta;
				
			} else {
				
				if (trunc(_Counter)>0)
					_Counter -= 1.f*gdl::Delta;
				
			}
			
			
			if (trunc(_Counter) <= 0) {
				
				_Counter = 0;
				
				if (_TempTextPtr != NULL) {
					free(_TempTextPtr);
					_TempTextPtr = NULL;
					_TextMaxLen = 0;
				}
				
			}
			
			blinkCount += gdl::Delta;
			if (trunc(blinkCount) >= 30) {
				blink ^= 1;
				blinkCount = 0;
			}
			
			
		}
		
	}
	
	
}