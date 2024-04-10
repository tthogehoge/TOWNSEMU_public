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

#include <SDL_events.h>

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

	FsXAddKeyMapping(FSKEY_SPACE,               SDLK_space);
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
	FsXAddKeyMapping(FSKEY_A,                   SDLK_A);
	FsXAddKeyMapping(FSKEY_B,                   SDLK_B);
	FsXAddKeyMapping(FSKEY_C,                   SDLK_C);
	FsXAddKeyMapping(FSKEY_D,                   SDLK_D);
	FsXAddKeyMapping(FSKEY_E,                   SDLK_E);
	FsXAddKeyMapping(FSKEY_F,                   SDLK_F);
	FsXAddKeyMapping(FSKEY_G,                   SDLK_G);
	FsXAddKeyMapping(FSKEY_H,                   SDLK_H);
	FsXAddKeyMapping(FSKEY_I,                   SDLK_I);
	FsXAddKeyMapping(FSKEY_J,                   SDLK_J);
	FsXAddKeyMapping(FSKEY_K,                   SDLK_K);
	FsXAddKeyMapping(FSKEY_L,                   SDLK_L);
	FsXAddKeyMapping(FSKEY_M,                   SDLK_M);
	FsXAddKeyMapping(FSKEY_N,                   SDLK_N);
	FsXAddKeyMapping(FSKEY_O,                   SDLK_O);
	FsXAddKeyMapping(FSKEY_P,                   SDLK_P);
	FsXAddKeyMapping(FSKEY_Q,                   SDLK_Q);
	FsXAddKeyMapping(FSKEY_R,                   SDLK_R);
	FsXAddKeyMapping(FSKEY_S,                   SDLK_S);
	FsXAddKeyMapping(FSKEY_T,                   SDLK_T);
	FsXAddKeyMapping(FSKEY_U,                   SDLK_U);
	FsXAddKeyMapping(FSKEY_V,                   SDLK_V);
	FsXAddKeyMapping(FSKEY_W,                   SDLK_W);
	FsXAddKeyMapping(FSKEY_X,                   SDLK_X);
	FsXAddKeyMapping(FSKEY_Y,                   SDLK_Y);
	FsXAddKeyMapping(FSKEY_Z,                   SDLK_Z);
	FsXAddKeyMapping(FSKEY_ESC,                 SDLK_Escape);
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
	FsXAddKeyMapping(FSKEY_SCROLLLOCK,          SDLK_Scroll_Lock);
	FsXAddKeyMapping(FSKEY_PAUSEBREAK,          SDLK_Cancel);
	FsXAddKeyMapping(FSKEY_TILDA,               SDLK_grave);
	FsXAddKeyMapping(FSKEY_MINUS,               SDLK_minus);
	FsXAddKeyMapping(FSKEY_PLUS,                SDLK_equal); // There was a mix up.
	FsXAddKeyMapping(FSKEY_BS,                  SDLK_BackSpace);
	FsXAddKeyMapping(FSKEY_TAB,                 SDLK_Tab);
	FsXAddKeyMapping(FSKEY_LBRACKET,            SDLK_bracketleft);
	FsXAddKeyMapping(FSKEY_RBRACKET,            SDLK_bracketright);
	FsXAddKeyMapping(FSKEY_BACKSLASH,           SDLK_backslash);
	FsXAddKeyMapping(FSKEY_CAPSLOCK,            0);
	FsXAddKeyMapping(FSKEY_SEMICOLON,           ';');
	FsXAddKeyMapping(FSKEY_COLON,               ':');
	FsXAddKeyMapping(FSKEY_SINGLEQUOTE,         '\'');
	FsXAddKeyMapping(FSKEY_ENTER,               SDLK_Return);
	FsXAddKeyMapping(FSKEY_SHIFT,               FSKEY_LEFT_SHIFT,   SDLK_Shift_L);
	FsXAddKeyMapping(FSKEY_SHIFT,               FSKEY_RIGHT_SHIFT,  SDLK_Shift_R);
	FsXAddKeyMapping(FSKEY_COMMA,               SDLK_comma);
	FsXAddKeyMapping(FSKEY_DOT,                 SDLK_period);
	FsXAddKeyMapping(FSKEY_SLASH,               SDLK_slash);
	FsXAddKeyMapping(FSKEY_CTRL,                FSKEY_LEFT_CTRL,    SDLK_Control_L);
	FsXAddKeyMapping(FSKEY_CTRL,                FSKEY_RIGHT_CTRL,   SDLK_Control_R);
	FsXAddKeyMapping(FSKEY_ALT,                 FSKEY_LEFT_ALT,     SDLK_Alt_L);
	FsXAddKeyMapping(FSKEY_ALT,                 FSKEY_RIGHT_ALT,    SDLK_Alt_R);
	FsXAddKeyMapping(FSKEY_INS,                 SDLK_Insert);
	FsXAddKeyMapping(FSKEY_DEL,                 SDLK_Delete);
	FsXAddKeyMapping(FSKEY_HOME,                SDLK_Home);
	FsXAddKeyMapping(FSKEY_END,                 SDLK_End);
	FsXAddKeyMapping(FSKEY_PAGEUP,              SDLK_Page_Up);
	FsXAddKeyMapping(FSKEY_PAGEDOWN,            SDLK_Page_Down);
	FsXAddKeyMapping(FSKEY_UP,                  SDLK_Up);
	FsXAddKeyMapping(FSKEY_DOWN,                SDLK_Down);
	FsXAddKeyMapping(FSKEY_LEFT,                SDLK_Left);
	FsXAddKeyMapping(FSKEY_RIGHT,               SDLK_Right);
	FsXAddKeyMapping(FSKEY_NUMLOCK,             SDLK_Num_Lock);
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
	FsXAddKeyMapping(FSKEY_TENDOT,              SDLK_KP_Decimal);
	FsXAddKeyMapping(FSKEY_TENSLASH,            SDLK_KP_Divide);
	FsXAddKeyMapping(FSKEY_TENSTAR,             SDLK_KP_Multiply);
	FsXAddKeyMapping(FSKEY_TENMINUS,            SDLK_KP_Subtract);
	FsXAddKeyMapping(FSKEY_TENPLUS,             SDLK_KP_Add);
	FsXAddKeyMapping(FSKEY_TENENTER,            SDLK_KP_Enter);

	FsXAddKeyMapping(FSKEY_TEN0,                SDLK_KP_Insert);
	FsXAddKeyMapping(FSKEY_TEN1,                SDLK_KP_End);
	FsXAddKeyMapping(FSKEY_TEN2,                SDLK_KP_Down);
	FsXAddKeyMapping(FSKEY_TEN3,                SDLK_KP_Page_Down);
	FsXAddKeyMapping(FSKEY_TEN4,                SDLK_KP_Left);
	FsXAddKeyMapping(FSKEY_TEN5,                SDLK_KP_Begin);
	FsXAddKeyMapping(FSKEY_TEN6,                SDLK_KP_Right);
	FsXAddKeyMapping(FSKEY_TEN7,                SDLK_KP_Home);
	FsXAddKeyMapping(FSKEY_TEN8,                SDLK_KP_Up);
	FsXAddKeyMapping(FSKEY_TEN9,                SDLK_KP_Page_Up);
	FsXAddKeyMapping(FSKEY_TENDOT,              SDLK_KP_Delete);
	// FsXAddKeyMapping(FSKEY_TENSLASH,            SDLK_KP_Divide);
	// FsXAddKeyMapping(FSKEY_TENSTAR,             SDLK_KP_Multiply);
	// FsXAddKeyMapping(FSKEY_TENMINUS,            SDLK_KP_Subtract);
	// FsXAddKeyMapping(FSKEY_TENPLUS,             SDLK_KP_Add);
	// FsXAddKeyMapping(FSKEY_TENENTER,            SDLK_KP_Enter);

	FsXAddKeyMapping(FSKEY_CONVERT,             SDLK_Henkan);
	FsXAddKeyMapping(FSKEY_NONCONVERT,          SDLK_Muhenkan);
	FsXAddKeyMapping(FSKEY_KANA,                SDLK_Kana_Lock);
	FsXAddKeyMapping(FSKEY_CONTEXT,             SDLK_Menu);


	FsXAddKeysymToCharMapping(SDLK_space,         ' ');
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
	FsXAddKeysymToCharMapping(SDLK_A,             'A');
	FsXAddKeysymToCharMapping(SDLK_B,             'B');
	FsXAddKeysymToCharMapping(SDLK_C,             'C');
	FsXAddKeysymToCharMapping(SDLK_D,             'D');
	FsXAddKeysymToCharMapping(SDLK_E,             'E');
	FsXAddKeysymToCharMapping(SDLK_F,             'F');
	FsXAddKeysymToCharMapping(SDLK_G,             'G');
	FsXAddKeysymToCharMapping(SDLK_H,             'H');
	FsXAddKeysymToCharMapping(SDLK_I,             'I');
	FsXAddKeysymToCharMapping(SDLK_J,             'J');
	FsXAddKeysymToCharMapping(SDLK_K,             'K');
	FsXAddKeysymToCharMapping(SDLK_L,             'L');
	FsXAddKeysymToCharMapping(SDLK_M,             'M');
	FsXAddKeysymToCharMapping(SDLK_N,             'N');
	FsXAddKeysymToCharMapping(SDLK_O,             'O');
	FsXAddKeysymToCharMapping(SDLK_P,             'P');
	FsXAddKeysymToCharMapping(SDLK_Q,             'Q');
	FsXAddKeysymToCharMapping(SDLK_R,             'R');
	FsXAddKeysymToCharMapping(SDLK_S,             'S');
	FsXAddKeysymToCharMapping(SDLK_T,             'T');
	FsXAddKeysymToCharMapping(SDLK_U,             'U');
	FsXAddKeysymToCharMapping(SDLK_V,             'V');
	FsXAddKeysymToCharMapping(SDLK_W,             'W');
	FsXAddKeysymToCharMapping(SDLK_X,             'X');
	FsXAddKeysymToCharMapping(SDLK_Y,             'Y');
	FsXAddKeysymToCharMapping(SDLK_Z,             'Z');
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
	FsXAddKeysymToCharMapping(SDLK_Escape,        0x1b);
	FsXAddKeysymToCharMapping(SDLK_BackSpace,     0x08);
	FsXAddKeysymToCharMapping(SDLK_Tab,           '\t');
	FsXAddKeysymToCharMapping(SDLK_Return,        '\n');

	
	FsXAddKeysymToCharMapping(SDLK_grave,         '`');
	FsXAddKeysymToCharMapping(SDLK_asciitilde,    '~');
	FsXAddKeysymToCharMapping(SDLK_exclam,        '!');
	FsXAddKeysymToCharMapping(SDLK_at,            '@');
	FsXAddKeysymToCharMapping(SDLK_numbersign,    '#');
	FsXAddKeysymToCharMapping(SDLK_dollar,        '$');
	FsXAddKeysymToCharMapping(SDLK_percent,       '%');
	FsXAddKeysymToCharMapping(SDLK_asciicircum,   '^');
	FsXAddKeysymToCharMapping(SDLK_ampersand,     '&');
	FsXAddKeysymToCharMapping(SDLK_asterisk,      '*');
	FsXAddKeysymToCharMapping(SDLK_parenleft,     '(');
	FsXAddKeysymToCharMapping(SDLK_parenright,    ')');
	FsXAddKeysymToCharMapping(SDLK_minus,         '-');
	FsXAddKeysymToCharMapping(SDLK_underscore,    '_');
	FsXAddKeysymToCharMapping(SDLK_equal,         '=');
	FsXAddKeysymToCharMapping(SDLK_plus,          '+');
	FsXAddKeysymToCharMapping(SDLK_bracketleft,   '[');
	FsXAddKeysymToCharMapping(SDLK_braceleft,     '{');
	FsXAddKeysymToCharMapping(SDLK_bracketright,  ']');
	FsXAddKeysymToCharMapping(SDLK_braceright,    '}');
	FsXAddKeysymToCharMapping(SDLK_backslash,     '\\');
	FsXAddKeysymToCharMapping(SDLK_bar,           '|');
	FsXAddKeysymToCharMapping(SDLK_semicolon,     ';');
	FsXAddKeysymToCharMapping(SDLK_colon,         ':');
	FsXAddKeysymToCharMapping(SDLK_apostrophe,    '\'');
	FsXAddKeysymToCharMapping(SDLK_quotedbl,      '\"');
	FsXAddKeysymToCharMapping(SDLK_comma,         ',');
	FsXAddKeysymToCharMapping(SDLK_less,          '<');
	FsXAddKeysymToCharMapping(SDLK_period,        '.');
	FsXAddKeysymToCharMapping(SDLK_greater,       '>');
	FsXAddKeysymToCharMapping(SDLK_slash,         '/');
	FsXAddKeysymToCharMapping(SDLK_question,      '?');

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
	FsXAddKeysymToCharMapping(SDLK_KP_Decimal,    '.');
	FsXAddKeysymToCharMapping(SDLK_KP_Divide,     '/');
	FsXAddKeysymToCharMapping(SDLK_KP_Multiply,   '*');
	FsXAddKeysymToCharMapping(SDLK_KP_Subtract,   '-');
	FsXAddKeysymToCharMapping(SDLK_KP_Add,        '+');
	FsXAddKeysymToCharMapping(SDLK_KP_Enter,      '\n');
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
