/* ////////////////////////////////////////////////////////////

File Name: fsglxwrapper.cpp
Copyright (c) 2017 Soji Yamakawa.  All rights reserved.
http://www.ysflight.com

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>


#include "fssimplewindow.h"

#define FS_NUM_XK 65536

extern void FsXCreateKeyMapping(void);
extern int FsXKeySymToFsInkey(int keysym);
extern int FsXKeySymToFsGetKeyState(int keysym);
extern char FsXKeySymToChar(int keysym);
extern int FsXFskeyToKeySym(int fskey);

class FsMouseEventLog
{
public:
	int eventType;
	int lb,mb,rb;
	int mx,my;
	unsigned int shift,ctrl;
};

#define NKEYBUF 256
static int nKeyBufUsed=0;
static int keyBuffer[NKEYBUF];
static int nCharBufUsed=0;
static int charBuffer[NKEYBUF];
static int nMosBufUsed=0;
static FsMouseEventLog mosBuffer[NKEYBUF];
static SDL_Event ev_key;


static SDL_Window* ysWindow;
static SDL_GLContext ysContext;
static Colormap ysXCMap;
static XVisualInfo *ysXVis;
static const int ysXEventMask=(KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ExposureMask|StructureNotifyMask);

static GLXContext ysGlRC;
static int ysGlxCfgSingle[]={GLX_RGBA,GLX_DEPTH_SIZE,16,None};
static int ysGlxCfgDouble[]={GLX_DOUBLEBUFFER,GLX_RGBA,GLX_DEPTH_SIZE,16,None};

static int ysXWid,ysXHei,ysXlupX,ysXlupY;



static int fsKeyPress[FSKEY_NUM_KEYCODE];
static int exposure=0;
static int lastKnownLb=0,lastKnownMb=0,lastKnownRb=0;



// For FsGui library tunnel >>
static long long int pastedContentLength=0;
static char *pastedContent=NULL;
static void ProcessSelectionRequest(XEvent &evt);
// For FsGui library tunnel <<


void FsOpenWindow(const FsOpenWindowOption &opt)
{
	int x0=opt.x0;
	int y0=opt.y0;
	int wid=opt.wid;
	int hei=opt.hei;
	int useDoubleBuffer=(int)opt.useDoubleBuffer;
	// int useMultiSampleBuffer=(int)opt.useMultiSampleBuffer;
	const char *title=(NULL!=opt.windowTitle ? opt.windowTitle : "Main Window");

	int n;
	char **m,*def;

	int lupX,lupY,sizX,sizY;
	lupX=x0;
	lupY=y0;
	sizX=wid;
	sizY=hei;

	FsXCreateKeyMapping();
	for(n=0; n<FSKEY_NUM_KEYCODE; n++)
	{
		fsKeyPress[n]=0;
	}

	// Apparently XInitThread is requierd even if only one thread is accessing X-Window system, unless that thread is the main thread.
	// Weird.
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) < 0) {
		printf("SDL_Init failed.\n");
		exit(1);
	}
	printf("SDL_Init OK.\n");
	if(useDoubleBuffer){
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	}
	ysWindow = SDL_CreateWindow(title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			wid,hei,
			SDL_WINDOW_OPENGL);
	if(ysWindow==NULL){
		fprintf(stderr,"Cannot Open Display.\n");
		exit(1);
	}
	printf("Opened display.\n");
	ysContext = SDL_GL_CreateContext(ysWindow);
	if(ysContext==NULL){
		fprintf(stderr,"Cannot create OpenGL context.\n");
		exit(1);
	}
	printf("Created OpenGL context.\n");

	// These lines are needed, or window will not appear >>
	glClearColor(1.0F,1.0F,1.0F,0.0F);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glFlush();
	// glXSwapBuffers(ysXDsp,ysXWnd);
	// These lines are needed, or window will not appear <<

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);

	GLfloat dif[]={0.8F,0.8F,0.8F,1.0F};
	GLfloat amb[]={0.4F,0.4F,0.4F,1.0F};
	GLfloat spc[]={0.9F,0.9F,0.9F,1.0F};
	GLfloat shininess[]={50.0,50.0,50.0,0.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);
	glLightfv(GL_LIGHT0,GL_SPECULAR,spc);
	glMaterialfv(GL_FRONT|GL_BACK,GL_SHININESS,shininess);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	if(0!=tryAlternativeSingleBuffer)
	{
		glDrawBuffer(GL_FRONT);
	}

	glClearColor(1.0F,1.0F,1.0F,0.0F);
	glClearDepth(1.0F);
	glDisable(GL_DEPTH_TEST);

	glViewport(0,0,sizX,sizY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,(float)sizX-1,(float)sizY-1,0,-1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glPointSize(1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3ub(0,0,0);

	if(NULL!=fsOpenGLInitializationCallBack)
	{
		(*fsOpenGLInitializationCallBack)(fsOpenGLInitializationCallBackParam);
	}

	if(NULL!=fsAfterWindowCreationCallBack)
	{
		(*fsAfterWindowCreationCallBack)(fsAfterWindowCreationCallBackParam);
	}

	return;
}

void FsResizeWindow(int newWid,int newHei)
{
}

int FsCheckWindowOpen(void)
{
	if(ysWindow!=NULL)
	{
		return 1;
	}
	return 0;
}

void FsGetWindowSize(int &wid,int &hei)
{
	SDL_GetWindowSize(ysWindow, &wid, &hei);
}

void FsGetWindowPosition(int &x0,int &y0)
{
	SDL_GetWindowPosition(ysWindow, &x0, &y0);
}

void FsSetWindowTitle(const char windowTitle[])
{
	SDL_SetWindowTitle(ysWindow, windowTitle);
}

static void execKeyEvent(SDL_Event ev)
{
	int i,fsKey,fsKey2;
	char chr;
	SDL_Keycode ks;
	ev_key = ev;

	fsKey=FSKEY_NULL;

	int i;
	// SDL_Scancode ev.key.keysym.scancode;
	// SDL_Keycode ev.key.keysym.sym;
	// SDL_Keymod ev.key.keysym.mod;

	if(0!=(ev.key.keysym.mod&KMOD_CTRL))
	{
		chr=0;
	}
	else if((ev.key.keysym.mod&KMOD_CAPS)==0 && (ev.key.keysym.mod&KMOD_SHIFT)==0)
	{
		chr=FsXKeySymToChar(keySymMap[0]); // mapXKtoChar[keySymMap[0]];
	}
	else if((ev.key.keysym.mod&KMOD_CAPS)==0 && (ev.key.keysym.mod&KMOD_SHIFT)!=0)
	{
		chr=FsXKeySymToChar(keySymMap[1]); // mapXKtoChar[keySymMap[1]];
	}
	else if((ev.key.keysym.mod&KMOD_SHIFT)==0 && (ev.key.keysym.mod&KMOD_CAPS)!=0)
	{
		chr=FsXKeySymToChar(keySymMap[0]); // mapXKtoChar[keySymMap[0]];
		if('a'<=chr && chr<='z')
		{
			chr=chr+('A'-'a');
		}
	}
	else if((ev.key.keysym.mod&KMOD_SHIFT)!=0 && (ev.key.keysym.mod&KMOD_CAPS)!=0)
	{
		chr=FsXKeySymToChar(keySymMap[1]); // mapXKtoChar[keySymMap[1]];
		if('a'<=chr && chr<='z')
		{
			chr=chr+('A'-'a');
		}
	}

	// Memo:
	// XK code is so badly designed.  XK_KP_Divide, XK_KP_Multiply,
	// XK_KP_Subtract, XK_KP_Add, should not be altered to
	// XK_XF86_Next_VMode or like that.  Other XK_KP_ code
	// can be altered by Mod2Mask.


	// following keys should be flipped based on Num Lock mask.  Apparently mod2mask is num lock by standard.
	// XK_KP_Space
	// XK_KP_Tab
	// XK_KP_Enter
	// XK_KP_F1
	// XK_KP_F2
	// XK_KP_F3
	// XK_KP_F4
	// XK_KP_Home
	// XK_KP_Left
	// XK_KP_Up
	// XK_KP_Right
	// XK_KP_Down
	// XK_KP_Prior
	// XK_KP_Page_Up
	// XK_KP_Next
	// XK_KP_Page_Down
	// XK_KP_End
	// XK_KP_Begin
	// XK_KP_Insert
	// XK_KP_Delete
	// XK_KP_Equal
	// XK_KP_Multiply
	// XK_KP_Add
	// XK_KP_Separator
	// XK_KP_Subtract
	// XK_KP_Decimal
	// XK_KP_Divide

	// XK_KP_0
	// XK_KP_1
	// XK_KP_2
	// XK_KP_3
	// XK_KP_4
	// XK_KP_5
	// XK_KP_6
	// XK_KP_7
	// XK_KP_8
	// XK_KP_9

	ks=ev.key.keysym.sym;
	if(SDLK_a<=ks && ks<=SDLK_z)
	{
		ks=ks+SDLK_A-SDLK_a;
	}

	if(0<=ks && ks<FS_NUM_XK)
	{
		fsKey=FsXKeySymToFsInkey(ks); // mapXKtoFSKEY[ks];
		fsKey2=FsXKeySymToFsGetKeyState(ks);

		// 2005/03/29 >>
		if(fsKey==0)
		{
			SDL_Scancode kcode;
			kcode=SDL_GetScancodeFromKey(ks);
			if(kcode!=0)
			{
				ks=XKeycodeToKeysym(ysXDsp,kcode,0);
				if(SDLK_a<=ks && ks<=SDLK_z)
				{
					ks=ks+SDLK_A-SDLK_a;
				}

				if(0<=ks && ks<FS_NUM_XK)
				{
					fsKey=FsXKeySymToFsInkey(ks); // mapXKtoFSKEY[ks];
					fsKey2=FsXKeySymToFsGetKeyState(ks);
				}
			}
		}
		// 2005/03/29 <<

		if(ev.type==KeyPress && fsKey!=0)
		{
			fsKeyPress[fsKey]=1;
			if(FSKEY_NULL!=fsKey2)
			{
				fsKeyPress[fsKey2]=1;
			}
			if(ev.xkey.window==ysXWnd) // 2005/04/08
			{
				if(nKeyBufUsed<NKEYBUF)
				{
					keyBuffer[nKeyBufUsed++]=fsKey;
				}
				if(chr!=0 && nCharBufUsed<NKEYBUF)
				{
					charBuffer[nCharBufUsed++]=chr;
				}
			}
		}
		else
		{
			fsKeyPress[fsKey]=0;
			if(FSKEY_NULL!=fsKey2)
			{
				fsKeyPress[fsKey2]=0;
			}
		}
	}
}

static void execMouseButtonEvent(SDL_Event ev)
{
	int fsKey=FSKEY_NULL;

	if(SDL_MOUSEBUTTONDOWN==ev.type || SDL_MOUSEBUTTONUP==ev.type)
	{
		fsKey=FSKEY_NULL;
		if(ev.button.button==SDL_BUTTON_X1)
		{
			fsKey=FSKEY_WHEELUP;
		}
		else if(ev.button.button==SDL_BUTTON_X2)
		{
			fsKey=FSKEY_WHEELDOWN;
		}

		if(FSKEY_NULL!=fsKey)
		{
			if(ev.type==SDL_MOUSEBUTTONDOWN)
			{
				fsKeyPress[fsKey]=1;
				if(ev.button.window==ysXWnd)
				{
					if(nKeyBufUsed<NKEYBUF)
					{
						keyBuffer[nKeyBufUsed++]=fsKey;
					}
				}

			}
			else if(ev.type==SDL_MOUSEBUTTONUP)
			{
				fsKeyPress[fsKey]=0;
			}
		}
		else if(NKEYBUF>nMosBufUsed)
		{
			mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_NONE;
			if(ev.type==SDL_MOUSEBUTTONDOWN)
			{
				switch(ev.button.button)
				{
					case SDL_BUTTON_LEFT:
						mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_LBUTTONDOWN;
						lastKnownLb=1;
						break;
					case SDL_BUTTON_MIDDLE:
						mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_MBUTTONDOWN;
						lastKnownMb=1;
						break;
					case SDL_BUTTON_RIGHT:
						mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_RBUTTONDOWN;
						lastKnownRb=1;
						break;
				}
			}
			else if(ev.type==SDL_MOUSEBUTTONUP)
			{
				switch(ev.button.button)
				{
					case SDL_BUTTON_LEFT:
						mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_LBUTTONUP;
						lastKnownLb=0;
						break;
					case SDL_BUTTON_MIDDLE:
						mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_MBUTTONUP;
						lastKnownMb=0;
						break;
					case SDL_BUTTON_RIGHT:
						mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_RBUTTONUP;
						lastKnownRb=0;
						break;
				}
			}

			mosBuffer[nMosBufUsed].mx=ev.button.x;
			mosBuffer[nMosBufUsed].my=ev.button.y;
			// Turned out these button states are highly unreliable.
			// It may come with (state&Button1Mask)==0 on ButtonPress event of Button 1.
			// Confirmed this problem in VirtualBox.  This silly flaw may not occur in 
			// real environment.
			// mosBuffer[nMosBufUsed].lb=(0!=(ev.button.state & Button1Mask));
			// mosBuffer[nMosBufUsed].mb=(0!=(ev.button.state & Button2Mask));
			// mosBuffer[nMosBufUsed].rb=(0!=(ev.button.state & Button3Mask));
			mosBuffer[nMosBufUsed].lb=lastKnownLb;
			mosBuffer[nMosBufUsed].mb=lastKnownMb;
			mosBuffer[nMosBufUsed].rb=lastKnownRb;
			mosBuffer[nMosBufUsed].shift=(0!=ev_key.key.keysym.mod & KMOD_SHIFT);
			mosBuffer[nMosBufUsed].ctrl=(0!=ev_key.key.keysym.mod & KMOD_CTRL);

			nMosBufUsed++;
		}
	}
}

static void execMouseMotionEvent(SDL_Event ev)
{
	int mx=ev.button.x;
	int my=ev.button.y;
	int lb=lastKnownLb; // XButtonEvent.state turns out to be highly unreliable  (0!=(ev.button.state & Button1Mask));
	int mb=lastKnownMb; // (0!=(ev.button.state & Button2Mask));
	int rb=lastKnownRb; // (0!=(ev.button.state & Button3Mask));
	int shift=(0!=ev_key.key.keysym.mod & KMOD_SHIFT);
	int ctrl=(0!=ev_key.key.keysym.mod & KMOD_CTRL);

	if(0<nMosBufUsed &&
			mosBuffer[nMosBufUsed-1].eventType==FSMOUSEEVENT_MOVE &&
			mosBuffer[nMosBufUsed-1].lb==lb &&
			mosBuffer[nMosBufUsed-1].mb==mb &&
			mosBuffer[nMosBufUsed-1].rb==rb &&
			mosBuffer[nMosBufUsed-1].shift==shift &&
			mosBuffer[nMosBufUsed-1].ctrl==ctrl)
	{
		mosBuffer[nMosBufUsed-1].mx=mx;
		mosBuffer[nMosBufUsed-1].my=my;
	}

	if(NKEYBUF>nMosBufUsed)
	{
		mosBuffer[nMosBufUsed].mx=mx;
		mosBuffer[nMosBufUsed].my=my;
		mosBuffer[nMosBufUsed].lb=lb;
		mosBuffer[nMosBufUsed].mb=mb;
		mosBuffer[nMosBufUsed].rb=rb;
		mosBuffer[nMosBufUsed].shift=shift;
		mosBuffer[nMosBufUsed].ctrl=ctrl;

		mosBuffer[nMosBufUsed].eventType=FSMOUSEEVENT_MOVE;

		nMosBufUsed++;
	}
}

void FsPollDevice(void)
{
	if(NULL==ysXWnd)
	{
		return;
	}

	SDL_Event e;

	if(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				execKeyEvent(e);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				execMouseButtonEvent(e);
				break;
			case SDL_MOUSEMOTION:
				execMouseMotionEvent(e);
				break;
			case SDL_WINDOWEVENT_RESIZED:
				ysXWid = event.window.data1;
				ysXHei = event.window.data2;
				break;
			case SDL_WINDOWEVENT_CLOSE:
				exit(1);
				break;
		}
	}

	/*
	// Clipboard Tunnel for FsGuiLib >>
	if(True==XCheckTypedWindowEvent(ysXDsp,ysXWnd,SelectionRequest,&ev) ||
	   True==XCheckTypedWindowEvent(ysXDsp,ysXWnd,SelectionClear,&ev))
	{
		if(ev.type==SelectionRequest)
		{
			ProcessSelectionRequest(ev);
		}
		else if(ev.type==SelectionClear)
		{
			printf("Lost selection ownership.\n");
		}
	}
	// Clipboard Tunnel for FsGuiLib <<
	*/

	return;
}

void FsPushOnPaintEvent(void)
{
	XExposeEvent evt;
	evt.type=Expose;
	evt.serial=0;
	evt.send_event=true;
	evt.display=ysXDsp;
	evt.window=ysXWnd;
	evt.x=0;
	evt.y=0;
	evt.width=ysXWid;
	evt.height=ysXHei;
	evt.count=0;
	XSendEvent(ysXDsp,ysXWnd,true,ExposureMask,(XEvent *)&evt);
}

void FsCloseWindow(void)
{
	SDL_Quit();
}

void FsMaximizeWindow(void)
{
	SDL_MaximizeWindow(ysWindow);
}

void FsUnmaximizeWindow(void)
{
	SDL_RestoreWindow(ysWindow);
}
void FsMakeFullScreen(void)
{
	//SDL_SetWindowFullscreen(ywWindow, SDL_WINDOW_FULLSCREEN);
	SDL_SetWindowFullscreen(ywWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void FsSleep(int ms)
{
	SDL_Delay(ms);
}

long long int FsPassedTime(void)
{
	static long long int lastTick;
	long long int tick;

	static int first=1;
	if(1==first)
	{
		lastTick=FsSubSecondTimer();
		first=0;
	}

	tick=FsSubSecondTimer();
	long long passed=tick-lastTick;
	lastTick=tick;

	return passed;
}

long long int FsSubSecondTimer(void)
{
	timeval tm;
	gettimeofday(&tm,NULL);
	long long int sec=tm.tv_sec;
	long long int usec=tm.tv_usec;

	long long int clk=sec*(long long int)1000+usec/(long long int)1000;


	static long long int lastValue=0;
	static long long int base=0;

	if(clk<lastValue) // Underflow.  It's tomorrow now.
	{
		base+=1000*3600*24;
	}
	lastValue=clk;

	static int first=1;
	static long long int t0=0;
	if(1==first)
	{
		t0=base+clk;
		first=0;
	}

	return base+clk-t0;
}

void FsGetMouseState(int &lb,int &mb,int &rb,int &mx,int &my)
{
	unsigned int mask;
	
	mask = SDL_GetMouseState(&mx, &my);

	/* These masks are seriouly unreliable.  It could still report zero after ButtonPress event 
	   is issued.  Therefore, it causes inconsistency and unusable.  Flaw confirmed in VirtualBox.
	lb=((mask & Button1Mask) ? 1 : 0);
	mb=((mask & Button2Mask) ? 1 : 0);
	rb=((mask & Button3Mask) ? 1 : 0); */

	lb=lastKnownLb;
	mb=lastKnownMb;
	rb=lastKnownRb;

	/*
	lb = (mask & SDL_BUTTON_LMASK) ? 1 : 0;
	mb = (mask & SDL_BUTTON_MMASK) ? 1 : 0;
	rb = (mask & SDL_BUTTON_RMASK) ? 1 : 0;
	*/
}

void FsSetMousePosition(int mx,int my)
{
	// This should move the mouse cursor to the given location.
	// However, it doesn't in VirtualBox.
	// This function may no longer implemented.
	SDL_WarpMouseGlobal(mx,my);
}

int FsGetMouseEvent(int &lb,int &mb,int &rb,int &mx,int &my)
{
	if(0<nMosBufUsed)
	{
		int eventType=mosBuffer[0].eventType;
		mx=mosBuffer[0].mx;
		my=mosBuffer[0].my;
		lb=mosBuffer[0].lb;
		mb=mosBuffer[0].mb;
		rb=mosBuffer[0].rb;

		int i;
		for(i=0; i<nMosBufUsed-1; i++)
		{
			mosBuffer[i]=mosBuffer[i+1];
		}
		nMosBufUsed--;

		return eventType;
	}
	else
	{
		FsGetMouseState(lb,mb,rb,mx,my);
		return FSMOUSEEVENT_NONE;
	}
}

void FsSwapBuffers(void)
{
	glFlush();
	glXSwapBuffers(ysXDsp,ysXWnd);
}

int FsInkey(void)
{
	if(nKeyBufUsed>0)
	{
		int i,keyCode;
		keyCode=keyBuffer[0];
		nKeyBufUsed--;
		for(i=0; i<nKeyBufUsed; i++)
		{
			keyBuffer[i]=keyBuffer[i+1];
		}
		return keyCode;
	}
	return 0;
}

int FsInkeyChar(void)
{
	if(nCharBufUsed>0)
	{
		int i,asciiCode;
		asciiCode=charBuffer[0];
		nCharBufUsed--;
		for(i=0; i<nCharBufUsed; i++)
		{
			charBuffer[i]=charBuffer[i+1];
		}
		return asciiCode;
	}
	return 0;
}

int FsGetKeyState(int fsKeyCode)
{
	if(0<fsKeyCode && fsKeyCode<FSKEY_NUM_KEYCODE)
	{
		return fsKeyPress[fsKeyCode];
	}
	return 0;
}

int FsCheckWindowExposure(void)
{
	int ret;
	ret=exposure;
	exposure=0;
	return ret;
}



#if 0
////////////////////////////////////////////////////////////
// Clipboard support
static void ProcessSelectionRequest(XEvent &evt)
{
	XEvent reply;

	Atom targetsAtom=XInternAtom(ysXDsp,"TARGETS",0);

	reply.xselection.type=SelectionNotify;
	reply.xselection.serial=evt.xselectionrequest.serial;
	reply.xselection.send_event=True;
	reply.xselection.display=ysXDsp;
	reply.xselection.requestor=evt.xselectionrequest.requestor;
	reply.xselection.selection=evt.xselectionrequest.selection;
	reply.xselection.target=evt.xselectionrequest.target;
	reply.xselection.property=evt.xselectionrequest.property; // None for nothing.
	reply.xselection.time=evt.xselectionrequest.time; // None for nothing.

	if(evt.xselectionrequest.target==targetsAtom)
	{
		Atom dataType[]={XA_STRING,targetsAtom};
		XChangeProperty(ysXDsp,evt.xselectionrequest.requestor,evt.xselectionrequest.property,XA_ATOM,32,PropModeReplace,(unsigned char *)dataType,2);
		printf("Answered supported data type.\n");
	}
	else if(evt.xselectionrequest.target==XA_STRING) // Since it declares it only accepts XA_TARGETS and XA_STRING, target should be a XA_STRING
	{
		XChangeProperty(ysXDsp,evt.xselectionrequest.requestor,evt.xselectionrequest.property,evt.xselectionrequest.target,8,PropModeReplace,(unsigned char *)clipBoardContent,clipBoardContentLength);
	}
	else // Just in case.
	{
		// Apparently, in this case, the requestor is supposed to try a different target type.
		reply.xselection.property=None;
	}

	XSendEvent(ysXDsp,evt.xselectionrequest.requestor,True,NoEventMask,&reply);
}
#endif

void FsX11GetClipBoardString(long long int &returnLength,char *&returnStr)
{
	if(NULL!=pastedContent)
	{
		delete [] pastedContent;
	}
	pastedContent=NULL;
	pastedContentLength=0;

	char *str = SDL_GetPrimarySelectionText();
	if(str){
		pastedContentLength = strlen(str);
		if(pastedContentLength){
			pastedContent=new char [pastedContentLength];
			memcpy(pastedContent, str, pastedContentLength);
		}
	}

	returnLength=pastedContentLength;
	returnStr=pastedContent;
}

void FsX11SetClipBoardString(long long int length,const char str[])
{
	if(length){
		SDL_SetClipboardText(str);
	}
}

void FsChangeToProgramDir(void)
{
	char buf[4096];
	auto len=readlink("/proc/self/exe",buf,4095);
	if(len<4095)
	{
		buf[len]=0;
		for(len=strlen(buf); 0<len; --len) // Probably it is safe to say for(len=len; 0<len; --len)
		{
			if(buf[len]=='/')
			{
				buf[len]=0;
				break;
			}
		}

		if(0!=chdir(buf))
		{
			switch(errno)
			{
			case EACCES:
				printf("EACCES\n");
				break;
			case EFAULT:
				printf("EFAULT\n");
				break;
			case EIO:
				printf("EIO\n");
				break;
			case ELOOP:
				printf("ELOOP\n");
				break;
			case ENAMETOOLONG:
				printf("ENAMETOOLONG\n");
				break;
			case ENOENT:
				printf("ENOENT\n");
				break;
			case ENOMEM:
				printf("ENOMEM\n");
				break;
			case ENOTDIR:
				printf("ENOTDIR\n");
				break;
			case EBADF:
				printf("EBADF\n");
				break;
			}
		}

		// 2016/1/1 chdir ignored?
		getcwd(buf,4095);
		printf("Changed to %s\n",buf);
	}
	else
	{
		printf("Current process file name too long.\n");
	}
}

void FsPushKey(int fskey)
{
	if(nKeyBufUsed<NKEYBUF)
	{
		keyBuffer[nKeyBufUsed++]=fskey;
	}
}

void FsPushChar(int c)
{
	if(nCharBufUsed<NKEYBUF)
	{
		charBuffer[nCharBufUsed++]=c;
	}
}

int FsGetNumCurrentTouch(void)
{
	return 0;
}

const FsVec2i *FsGetCurrentTouch(void)
{
	return nullptr;
}

////////////////////////////////////////////////////////////

// How can I enable IME in Linux?

int FsEnableIME(void)
{
	return 0;
}

void FsDisableIME(void)
{
}

////////////////////////////////////////////////////////////

int FsIsNativeTextInputAvailable(void)
{
	return 0;
}

int FsOpenNativeTextInput(int x1,int y1,int wid,int hei)
{
	return 0;
}

void FsCloseNativeTextInput(void)
{
}

void FsSetNativeTextInputText(const wchar_t [])
{
}

int FsGetNativeTextInputTextLength(void)
{
	return 0;
}

void FsGetNativeTextInputText(wchar_t str[],int bufLen)
{
	if(0<bufLen)
	{
		str[0]=0;
	}
}

int FsGetNativeTextInputEvent(void)
{
	return FSNATIVETEXTEVENT_NONE;
}

void FsShowMouseCursor(int showFlag)
{
}
int FsIsMouseCursorVisible(void)
{
	return 1;
}
