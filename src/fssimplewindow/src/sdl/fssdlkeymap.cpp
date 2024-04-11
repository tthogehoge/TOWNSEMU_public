/* ////////////////////////////////////////////////////////////

File Name: fsglxkeymap.cpp
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
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <SDL2/SDL_events.h>

#include "fssimplewindow.h"

#define FS_NUM_XK 65536

class XKtoFSKEYMap
{
public:
	int fskeyForInkey=FSKEY_NULL;
	int fskeyForKeyState=FSKEY_NULL;
};

static XKtoFSKEYMap mapXKtoFSKEY[FS_NUM_XK];
static char mapXKtoChar[FS_NUM_XK];
static int mapFSKEYtoXK[FSKEY_NUM_KEYCODE];


static void FsXAddKeyMapping(int fskey1,int fskey2,int keysym)
{
	if(fskey1<0 || FSKEY_NUM_KEYCODE<=fskey1)
	{
		fprintf(stderr,"FSKEY is out of range\n");
		return;
	}
	if(keysym<0 || FS_NUM_XK<=keysym)
	{
		fprintf(stderr,"XK is out of range\n");
		return;
	}

	mapXKtoFSKEY[keysym].fskeyForInkey=fskey1;
	mapXKtoFSKEY[keysym].fskeyForKeyState=fskey2;
	mapFSKEYtoXK[fskey1]=keysym;
	mapFSKEYtoXK[fskey2]=keysym;
}

static void FsXAddKeyMapping(int fskey,int keysym)
{
	if(fskey<0 || FSKEY_NUM_KEYCODE<=fskey)
	{
		fprintf(stderr,"FSKEY is out of range\n");
		return;
	}
	if(keysym<0 || FS_NUM_XK<=keysym)
	{
		fprintf(stderr,"XK is out of range\n");
		return;
	}

	mapXKtoFSKEY[keysym].fskeyForInkey=fskey;
	mapXKtoFSKEY[keysym].fskeyForKeyState=FSKEY_NULL;
	mapFSKEYtoXK[fskey]=keysym;
}

static void FsXAddKeysymToCharMapping(int keysym,char c)
{
	if(0<keysym && keysym<FS_NUM_XK)
	{
		mapXKtoChar[keysym]=c;
	}
}

void FsXCreateKeyMapping(void)
{
	int i;
	for(i=0; i<FS_NUM_XK; i++)
	{
		mapXKtoFSKEY[i].fskeyForInkey=0;
		mapXKtoFSKEY[i].fskeyForKeyState=0;
		mapXKtoChar[i]=0;
	}
	for(i=0; i<FSKEY_NUM_KEYCODE; i++)
	{
		mapFSKEYtoXK[i]=0;
	}

	FsXAddKeyMapping(FSKEY_SPACE,               SDLK_SPACE);
	FsXAddKeyMapping(FSKEY_0,                   SDLK_0);
	FsXAddKeyMapping(FSKEY_1,                   SDLK_1);
	FsXAddKeyMapping(FSKEY_2,                   SDLK_2);
	FsXAddKeyMapping(FSKEY_3,                   SDLK_3);
	FsXAddKeyMapping(FSKEY_4,                   SDLK_4);
	FsXAddKeyMapping(FSKEY_5,                   SDLK_5);
	FsXAddKeyMapping(FSKEY_6,                   SDLK_6);
	FsXAddKeyMapping(FSKEY_7,                   SDLK_7);
	FsXAddKeyMapping(FSKEY_8,                   SDLK_8);
	FsXAddKeyMapping(FSKEY_9,                   SDLK_9);
	FsXAddKeyMapping(FSKEY_A,                   SDLK_a);
	FsXAddKeyMapping(FSKEY_B,                   SDLK_b);
	FsXAddKeyMapping(FSKEY_C,                   SDLK_c);
	FsXAddKeyMapping(FSKEY_D,                   SDLK_d);
	FsXAddKeyMapping(FSKEY_E,                   SDLK_e);
	FsXAddKeyMapping(FSKEY_F,                   SDLK_f);
	FsXAddKeyMapping(FSKEY_G,                   SDLK_g);
	FsXAddKeyMapping(FSKEY_H,                   SDLK_h);
	FsXAddKeyMapping(FSKEY_I,                   SDLK_i);
	FsXAddKeyMapping(FSKEY_J,                   SDLK_j);
	FsXAddKeyMapping(FSKEY_K,                   SDLK_k);
	FsXAddKeyMapping(FSKEY_L,                   SDLK_l);
	FsXAddKeyMapping(FSKEY_M,                   SDLK_m);
	FsXAddKeyMapping(FSKEY_N,                   SDLK_n);
	FsXAddKeyMapping(FSKEY_O,                   SDLK_o);
	FsXAddKeyMapping(FSKEY_P,                   SDLK_p);
	FsXAddKeyMapping(FSKEY_Q,                   SDLK_q);
	FsXAddKeyMapping(FSKEY_R,                   SDLK_r);
	FsXAddKeyMapping(FSKEY_S,                   SDLK_s);
	FsXAddKeyMapping(FSKEY_T,                   SDLK_t);
	FsXAddKeyMapping(FSKEY_U,                   SDLK_u);
	FsXAddKeyMapping(FSKEY_V,                   SDLK_v);
	FsXAddKeyMapping(FSKEY_W,                   SDLK_w);
	FsXAddKeyMapping(FSKEY_X,                   SDLK_x);
	FsXAddKeyMapping(FSKEY_Y,                   SDLK_y);
	FsXAddKeyMapping(FSKEY_Z,                   SDLK_z);
	FsXAddKeyMapping(FSKEY_ESC,                 SDLK_ESCAPE);
	FsXAddKeyMapping(FSKEY_F1,                  SDLK_F1);
	FsXAddKeyMapping(FSKEY_F2,                  SDLK_F2);
	FsXAddKeyMapping(FSKEY_F3,                  SDLK_F3);
	FsXAddKeyMapping(FSKEY_F4,                  SDLK_F4);
	FsXAddKeyMapping(FSKEY_F5,                  SDLK_F5);
	FsXAddKeyMapping(FSKEY_F6,                  SDLK_F6);
	FsXAddKeyMapping(FSKEY_F7,                  SDLK_F7);
	FsXAddKeyMapping(FSKEY_F8,                  SDLK_F8);
	FsXAddKeyMapping(FSKEY_F9,                  SDLK_F9);
	FsXAddKeyMapping(FSKEY_F10,                 SDLK_F10);
	FsXAddKeyMapping(FSKEY_F11,                 SDLK_F11);
	FsXAddKeyMapping(FSKEY_F12,                 SDLK_F12);
	FsXAddKeyMapping(FSKEY_PRINTSCRN,           0);
	FsXAddKeyMapping(FSKEY_SCROLLLOCK,          SDLK_SCROLLLOCK);
	FsXAddKeyMapping(FSKEY_PAUSEBREAK,          SDLK_PAUSE);
	FsXAddKeyMapping(FSKEY_TILDA,               SDLK_BACKQUOTE);
	FsXAddKeyMapping(FSKEY_MINUS,               SDLK_MINUS);
	FsXAddKeyMapping(FSKEY_PLUS,                SDLK_EQUALS); // There was a mix up.
	FsXAddKeyMapping(FSKEY_BS,                  SDLK_BACKSPACE);
	FsXAddKeyMapping(FSKEY_TAB,                 SDLK_TAB);
	FsXAddKeyMapping(FSKEY_LBRACKET,            SDLK_LEFTBRACKET);
	FsXAddKeyMapping(FSKEY_RBRACKET,            SDLK_RIGHTBRACKET);
	FsXAddKeyMapping(FSKEY_BACKSLASH,           SDLK_BACKSLASH);
	FsXAddKeyMapping(FSKEY_CAPSLOCK,            0);
	FsXAddKeyMapping(FSKEY_SEMICOLON,           ';');
	FsXAddKeyMapping(FSKEY_COLON,               ':');
	FsXAddKeyMapping(FSKEY_SINGLEQUOTE,         '\'');
	FsXAddKeyMapping(FSKEY_ENTER,               SDLK_RETURN);
	FsXAddKeyMapping(FSKEY_SHIFT,               FSKEY_LEFT_SHIFT,   SDLK_LSHIFT);
	FsXAddKeyMapping(FSKEY_SHIFT,               FSKEY_RIGHT_SHIFT,  SDLK_RSHIFT);
	FsXAddKeyMapping(FSKEY_COMMA,               SDLK_COMMA);
	FsXAddKeyMapping(FSKEY_DOT,                 SDLK_PERIOD);
	FsXAddKeyMapping(FSKEY_SLASH,               SDLK_SLASH);
	FsXAddKeyMapping(FSKEY_CTRL,                FSKEY_LEFT_CTRL,    SDLK_LCTRL);
	FsXAddKeyMapping(FSKEY_CTRL,                FSKEY_RIGHT_CTRL,   SDLK_RCTRL);
	FsXAddKeyMapping(FSKEY_ALT,                 FSKEY_LEFT_ALT,     SDLK_LALT);
	FsXAddKeyMapping(FSKEY_ALT,                 FSKEY_RIGHT_ALT,    SDLK_RALT);
	FsXAddKeyMapping(FSKEY_INS,                 SDLK_INSERT);
	FsXAddKeyMapping(FSKEY_DEL,                 SDLK_DELETE);
	FsXAddKeyMapping(FSKEY_HOME,                SDLK_HOME);
	FsXAddKeyMapping(FSKEY_END,                 SDLK_END);
	FsXAddKeyMapping(FSKEY_PAGEUP,              SDLK_PAGEUP);
	FsXAddKeyMapping(FSKEY_PAGEDOWN,            SDLK_PAGEDOWN);
	FsXAddKeyMapping(FSKEY_UP,                  SDLK_UP);
	FsXAddKeyMapping(FSKEY_DOWN,                SDLK_DOWN);
	FsXAddKeyMapping(FSKEY_LEFT,                SDLK_LEFT);
	FsXAddKeyMapping(FSKEY_RIGHT,               SDLK_RIGHT);
	FsXAddKeyMapping(FSKEY_NUMLOCK,             SDLK_NUMLOCKCLEAR);
	FsXAddKeyMapping(FSKEY_TEN0,                SDLK_KP_0);
	FsXAddKeyMapping(FSKEY_TEN1,                SDLK_KP_1);
	FsXAddKeyMapping(FSKEY_TEN2,                SDLK_KP_2);
	FsXAddKeyMapping(FSKEY_TEN3,                SDLK_KP_3);
	FsXAddKeyMapping(FSKEY_TEN4,                SDLK_KP_4);
	FsXAddKeyMapping(FSKEY_TEN5,                SDLK_KP_5);
	FsXAddKeyMapping(FSKEY_TEN6,                SDLK_KP_6);
	FsXAddKeyMapping(FSKEY_TEN7,                SDLK_KP_7);
	FsXAddKeyMapping(FSKEY_TEN8,                SDLK_KP_8);
	FsXAddKeyMapping(FSKEY_TEN9,                SDLK_KP_9);
	FsXAddKeyMapping(FSKEY_TENDOT,              SDLK_KP_DECIMAL);
	FsXAddKeyMapping(FSKEY_TENSLASH,            SDLK_KP_DIVIDE);
	FsXAddKeyMapping(FSKEY_TENSTAR,             SDLK_KP_MULTIPLY);
	FsXAddKeyMapping(FSKEY_TENMINUS,            SDLK_KP_MINUS);
	FsXAddKeyMapping(FSKEY_TENPLUS,             SDLK_KP_PLUS);
	FsXAddKeyMapping(FSKEY_TENENTER,            SDLK_KP_ENTER);

	/*
	FsXAddKeyMapping(FSKEY_TEN0,                SDLK_KP_INSERT);
	FsXAddKeyMapping(FSKEY_TEN1,                SDLK_KP_END);
	FsXAddKeyMapping(FSKEY_TEN2,                SDLK_KP_DOWN);
	FsXAddKeyMapping(FSKEY_TEN3,                SDLK_KP_PAGEDOWN);
	FsXAddKeyMapping(FSKEY_TEN4,                SDLK_KP_LEFT);
	FsXAddKeyMapping(FSKEY_TEN5,                SDLK_KP_BEGIN);
	FsXAddKeyMapping(FSKEY_TEN6,                SDLK_KP_RIGHT);
	FsXAddKeyMapping(FSKEY_TEN7,                SDLK_KP_HOME);
	FsXAddKeyMapping(FSKEY_TEN8,                SDLK_KP_UP);
	FsXAddKeyMapping(FSKEY_TEN9,                SDLK_KP_PAGEUP);
	FsXAddKeyMapping(FSKEY_TENDOT,              SDLK_KP_DELETE);
	*/
	// FsXAddKeyMapping(FSKEY_TENSLASH,            SDLK_KP_Divide);
	// FsXAddKeyMapping(FSKEY_TENSTAR,             SDLK_KP_Multiply);
	// FsXAddKeyMapping(FSKEY_TENMINUS,            SDLK_KP_Subtract);
	// FsXAddKeyMapping(FSKEY_TENPLUS,             SDLK_KP_Add);
	// FsXAddKeyMapping(FSKEY_TENENTER,            SDLK_KP_Enter);

	//FsXAddKeyMapping(FSKEY_CONVERT,             SDL_SCANCODE_INTERNATIONAL4); 
	//FsXAddKeyMapping(FSKEY_NONCONVERT,          SDL_SCANCODE_INTERNATIONAL5); 
	//FsXAddKeyMapping(FSKEY_KANA,                SDL_SCANCODE_INTERNATIONAL2); 
	FsXAddKeyMapping(FSKEY_CONTEXT,             SDLK_APPLICATION);


	FsXAddKeysymToCharMapping(SDLK_SPACE,         ' ');
	FsXAddKeysymToCharMapping(SDLK_0,             '0');
	FsXAddKeysymToCharMapping(SDLK_1,             '1');
	FsXAddKeysymToCharMapping(SDLK_2,             '2');
	FsXAddKeysymToCharMapping(SDLK_3,             '3');
	FsXAddKeysymToCharMapping(SDLK_4,             '4');
	FsXAddKeysymToCharMapping(SDLK_5,             '5');
	FsXAddKeysymToCharMapping(SDLK_6,             '6');
	FsXAddKeysymToCharMapping(SDLK_7,             '7');
	FsXAddKeysymToCharMapping(SDLK_8,             '8');
	FsXAddKeysymToCharMapping(SDLK_9,             '9');
	FsXAddKeysymToCharMapping(SDLK_a,             'a');
	FsXAddKeysymToCharMapping(SDLK_b,             'b');
	FsXAddKeysymToCharMapping(SDLK_c,             'c');
	FsXAddKeysymToCharMapping(SDLK_d,             'd');
	FsXAddKeysymToCharMapping(SDLK_e,             'e');
	FsXAddKeysymToCharMapping(SDLK_f,             'f');
	FsXAddKeysymToCharMapping(SDLK_g,             'g');
	FsXAddKeysymToCharMapping(SDLK_h,             'h');
	FsXAddKeysymToCharMapping(SDLK_i,             'i');
	FsXAddKeysymToCharMapping(SDLK_j,             'j');
	FsXAddKeysymToCharMapping(SDLK_k,             'k');
	FsXAddKeysymToCharMapping(SDLK_l,             'l');
	FsXAddKeysymToCharMapping(SDLK_m,             'm');
	FsXAddKeysymToCharMapping(SDLK_n,             'n');
	FsXAddKeysymToCharMapping(SDLK_o,             'o');
	FsXAddKeysymToCharMapping(SDLK_p,             'p');
	FsXAddKeysymToCharMapping(SDLK_q,             'q');
	FsXAddKeysymToCharMapping(SDLK_r,             'r');
	FsXAddKeysymToCharMapping(SDLK_s,             's');
	FsXAddKeysymToCharMapping(SDLK_t,             't');
	FsXAddKeysymToCharMapping(SDLK_u,             'u');
	FsXAddKeysymToCharMapping(SDLK_v,             'v');
	FsXAddKeysymToCharMapping(SDLK_w,             'w');
	FsXAddKeysymToCharMapping(SDLK_x,             'x');
	FsXAddKeysymToCharMapping(SDLK_y,             'y');
	FsXAddKeysymToCharMapping(SDLK_z,             'z');
	FsXAddKeysymToCharMapping(SDLK_ESCAPE,        0x1b);
	FsXAddKeysymToCharMapping(SDLK_BACKSPACE,     0x08);
	FsXAddKeysymToCharMapping(SDLK_TAB,           '\t');
	FsXAddKeysymToCharMapping(SDLK_RETURN,        '\n');

	
	FsXAddKeysymToCharMapping(SDLK_BACKQUOTE,     '`');
	FsXAddKeysymToCharMapping(SDLK_CARET,         '~');
	FsXAddKeysymToCharMapping(SDLK_EXCLAIM,       '!');
	FsXAddKeysymToCharMapping(SDLK_AT,            '@');
	FsXAddKeysymToCharMapping(SDLK_HASH,          '#');
	FsXAddKeysymToCharMapping(SDLK_DOLLAR,        '$');
	FsXAddKeysymToCharMapping(SDLK_PERCENT,       '%');
	FsXAddKeysymToCharMapping(SDLK_CARET,         '^');
	FsXAddKeysymToCharMapping(SDLK_AMPERSAND,     '&');
	FsXAddKeysymToCharMapping(SDLK_ASTERISK,      '*');
	FsXAddKeysymToCharMapping(SDLK_LEFTPAREN,     '(');
	FsXAddKeysymToCharMapping(SDLK_RIGHTPAREN,    ')');
	FsXAddKeysymToCharMapping(SDLK_MINUS,         '-');
	FsXAddKeysymToCharMapping(SDLK_UNDERSCORE,    '_');
	FsXAddKeysymToCharMapping(SDLK_EQUALS,        '=');
	FsXAddKeysymToCharMapping(SDLK_PLUS,          '+');
	FsXAddKeysymToCharMapping(SDLK_LEFTBRACKET,   '[');
	FsXAddKeysymToCharMapping(SDLK_LEFTBRACKET,   '{');
	FsXAddKeysymToCharMapping(SDLK_RIGHTBRACKET,  ']');
	FsXAddKeysymToCharMapping(SDLK_RIGHTBRACKET,  '}');
	FsXAddKeysymToCharMapping(SDLK_BACKSLASH,     '\\');
	FsXAddKeysymToCharMapping(SDLK_BACKSLASH,     '|');
	FsXAddKeysymToCharMapping(SDLK_SEMICOLON,     ';');
	FsXAddKeysymToCharMapping(SDLK_COLON,         ':');
	FsXAddKeysymToCharMapping(SDLK_QUOTE,         '\'');
	FsXAddKeysymToCharMapping(SDLK_QUOTEDBL,      '\"');
	FsXAddKeysymToCharMapping(SDLK_COMMA,         ',');
	FsXAddKeysymToCharMapping(SDLK_LESS,          '<');
	FsXAddKeysymToCharMapping(SDLK_PERIOD,        '.');
	FsXAddKeysymToCharMapping(SDLK_GREATER,       '>');
	FsXAddKeysymToCharMapping(SDLK_SLASH,         '/');
	FsXAddKeysymToCharMapping(SDLK_QUESTION,      '?');

	FsXAddKeysymToCharMapping(SDLK_KP_0,          '0');
	FsXAddKeysymToCharMapping(SDLK_KP_1,          '1');
	FsXAddKeysymToCharMapping(SDLK_KP_2,          '2');
	FsXAddKeysymToCharMapping(SDLK_KP_3,          '3');
	FsXAddKeysymToCharMapping(SDLK_KP_4,          '4');
	FsXAddKeysymToCharMapping(SDLK_KP_5,          '5');
	FsXAddKeysymToCharMapping(SDLK_KP_6,          '6');
	FsXAddKeysymToCharMapping(SDLK_KP_7,          '7');
	FsXAddKeysymToCharMapping(SDLK_KP_8,          '8');
	FsXAddKeysymToCharMapping(SDLK_KP_9,          '9');
	FsXAddKeysymToCharMapping(SDLK_KP_DECIMAL,    '.');
	FsXAddKeysymToCharMapping(SDLK_KP_DIVIDE,     '/');
	FsXAddKeysymToCharMapping(SDLK_KP_MULTIPLY,   '*');
	FsXAddKeysymToCharMapping(SDLK_KP_MINUS,      '-');
	FsXAddKeysymToCharMapping(SDLK_KP_PLUS,       '+');
	FsXAddKeysymToCharMapping(SDLK_KP_ENTER,      '\n');
}

int FsXKeySymToFsInkey(int keysym)
{
	if(0<=keysym && keysym<FS_NUM_XK)
	{
		return mapXKtoFSKEY[keysym].fskeyForInkey;
	}
	return 0;
}

int FsXKeySymToFsGetKeyState(int keysym)
{
	if(0<=keysym && keysym<FS_NUM_XK)
	{
		return mapXKtoFSKEY[keysym].fskeyForKeyState;
	}
	return 0;
}

char FsXKeySymToChar(int keysym)
{
	if(0<=keysym && keysym<FS_NUM_XK)
	{
		return mapXKtoChar[keysym];
	}
	return 0;
}

int FsXFskeyToKeySym(int fskey)
{
	if(0<=fskey && fskey<FSKEY_NUM_KEYCODE)
	{
		return mapFSKEYtoXK[fskey];
	}
	return 0;
}
