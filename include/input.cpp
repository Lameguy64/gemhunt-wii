#include <wiiuse/wpad.h>


// Offscreen padding size for IR pointer
#define INPUT_WIIMOTE_OFFSCREEN	64


namespace input {
	
	
	typedef struct {
		
		bool	rumble;
		float	rumbleTime;
		int		status;
		
		WPADData data;
		
	} wiimoteType;
	
	
	int			_initialized=false;
	wiimoteType	Wiimote[WPAD_MAX_WIIMOTES];
	
	
	int Init() {
		
		
		// Initialize crappy Wiimote subsystem, WPAD
		WPAD_Init();
		
		memset(Wiimote, 0x00, sizeof(wiimoteType)*WPAD_MAX_WIIMOTES);
		
		for(int i=0; i<WPAD_MAX_WIIMOTES; i++) {
			
			WPAD_SetDataFormat(i, WPAD_FMT_BTNS_ACC_IR);
			WPAD_SetVRes(i, gdl::ScreenXres+INPUT_WIIMOTE_OFFSCREEN, gdl::ScreenYres+INPUT_WIIMOTE_OFFSCREEN);
			
		}
		
		
		// Finish initialization
		_initialized=true;
		
		
		return(0);
		
	}
	
	
	void UpdateInput() {
		
		for(int i=0; i<WPAD_MAX_WIIMOTES; i++) {
			
			if (trunc(Wiimote[i].rumbleTime) > 1) {
				
				Wiimote[i].rumble = true;
				Wiimote[i].rumbleTime -= 1.f*gdl::Delta;
				
			} else if ((trunc(Wiimote[i].rumbleTime) < 0) || (trunc(Wiimote[i].rumbleTime) == 1) ) {
				
				Wiimote[i].rumble = false;
				Wiimote[i].rumbleTime = 0;
				
			}
			
			int ret;
			
			while(1) {
				
				ret = WPAD_ReadEvent(i, &Wiimote[i].data);
				if (ret < WPAD_ERR_NONE) break;
				
				WPAD_Rumble(i, Wiimote[i].rumble);
				
				Wiimote[i].data.ir.x -= (INPUT_WIIMOTE_OFFSCREEN/2);
				Wiimote[i].data.ir.y -= (INPUT_WIIMOTE_OFFSCREEN/2);
				
			}
			
			if (ret != WPAD_ERR_QUEUE_EMPTY) {
				Wiimote[i].status = ret;
			} else {
				Wiimote[i].status = WPAD_ERR_NONE;
			}
			
		}
		
	}
	
	
};