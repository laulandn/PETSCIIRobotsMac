#ifndef SDL_H
#define SDL_H


#ifndef _MAC
#define _MAC
#endif


#ifdef TARGET_API_MAC_CARBON
#ifndef TARGET_API_CARBON
#define TARGET_API_CARBON 1
#endif
#endif
#if TARGET_API_CARBON
#include <Carbon/Carbon.h>
#else
#include <Quickdraw.h>
#include <QDOffscreen.h>
#include <MacWindows.h>
#include <Dialogs.h>
#endif


#define SDL_SCANCODE_I 'i'
#define SDL_SCANCODE_J 'j'
#define SDL_SCANCODE_K 'k'
#define SDL_SCANCODE_L 'l'
#define SDL_SCANCODE_W 'w'
#define SDL_SCANCODE_S 's'
#define SDL_SCANCODE_A 'a'
#define SDL_SCANCODE_C 'c'
#define SDL_SCANCODE_D 'd'
#define SDL_SCANCODE_Z 'z'
#define SDL_SCANCODE_M 'm'
#define SDL_SCANCODE_Y 'y'
#define SDL_SCANCODE_N 'n'
#define SDL_SCANCODE_SPACE ' '
#define SDL_SCANCODE_TAB '\t'
#define SDL_SCANCODE_RETURN '\n'

#define SDL_SCANCODE_F1 1
#define SDL_SCANCODE_F2 2
#define SDL_SCANCODE_F6 6
#define SDL_SCANCODE_LEFT 10
#define SDL_SCANCODE_RIGHT 11
#define SDL_SCANCODE_UP 12
#define SDL_SCANCODE_DOWN 13
#define SDL_SCANCODE_ESCAPE 100
#define SDL_SCANCODE_RSHIFT 1000
#define SDL_SCANCODE_LSHIFT 1001


#define SDL_INIT_EVERYTHING 0
#define SDL_HINT_RENDER_VSYNC 0
#define AUDIO_S16LSB 0
#define SDL_AUDIO_ALLOW_ANY_CHANGE 0
#define SDL_BLENDMODE_NONE 0


#define SDL_KEYUP 1
#define SDL_KEYDOWN 2
#define SDL_JOYBUTTONUP 10
#define SDL_JOYBUTTONDOWN 11
#define SDL_QUIT 100

#define SDL_TRUE -1
#define SDL_FALSE 0


#define SDL_Window CGrafPort
#define SDL_AudioDeviceID int
#define Uint8 char
#define SDLCALL

typedef void (SDLCALL * SDL_AudioCallback) (void *userdata, Uint8 * stream,int len);

struct SDL_Rect { int x,y,w,h; };
struct SDL_Color { int r,g,b,i; };
struct SDL_Palette { int num; SDL_Color *colors; };

struct SDL_MyFormat {
  SDL_Palette *palette;
};

struct SDL_Surface {
  SDL_MyFormat *format;
  GWorldPtr g;
};


struct SDL_AudioSpec {
  int freq;
  int format;
  int channels;
  int samples;
  void *userdata;
  SDL_AudioCallback callback;
};

struct SDL_Joystick {
  void *dummy;
};

struct SDL_MyKeySym {
  int scancode;
};

struct SDL_MyKey {
  SDL_MyKeySym keysym;
  int repeat;
};

struct SDL_MyJButton {
  int button;
};

struct SDL_Event {
  int type;
  SDL_MyKey key;
  SDL_MyJButton jbutton;
};


#define main(a,b) SDL_main(a,b)

extern int SDL_main(int argc,char **argv);


#ifdef _MAC
extern SDL_Surface *SDL_CreateRGBSurface(uint32_t flags,uint32_t w,uint32_t h,uint32_t d,uint32_t rm,uint32_t gm,uint32_t bm,uint32_t am);
extern void SDL_FreeSurface(SDL_Surface *s);
extern void SDL_BlitSurface(SDL_Surface *s,SDL_Rect *sr,SDL_Surface * d,SDL_Rect *dr);
extern void SDL_BlitScaled(SDL_Surface *s,SDL_Rect *sr,SDL_Surface * d,SDL_Rect *dr);
extern void SDL_SetClipRect(SDL_Surface *s,SDL_Rect *sr);
extern void  SDL_FillRect(SDL_Surface *s,SDL_Rect *sr,uint32_t v);
extern void  SDL_FillRects(SDL_Surface *s,SDL_Rect *rs,uint32_t n,uint32_t v);
extern void  SDL_UpdateWindowSurface(SDL_Window *w);
//
extern const char *SDL_GetError(void);
extern int SDL_Init(int val);
extern void SDL_Quit(void);
extern void SDL_zero(SDL_AudioSpec);
extern void SDL_SetHint(int what,const char *val);
extern SDL_AudioDeviceID SDL_OpenAudioDevice(void *,int val,SDL_AudioSpec *want,SDL_AudioSpec *got,int setting);
extern void SDL_PauseAudioDevice(SDL_AudioDeviceID a,int v);
extern void SDL_CloseAudioDevice(SDL_AudioDeviceID a);
extern SDL_Joystick *SDL_JoystickOpen(int val);
extern void SDL_JoystickClose(SDL_Joystick *j);
extern int SDL_JoystickGetAxis(SDL_Joystick *j,int val);
extern SDL_Surface *SDL_GetWindowSurface(SDL_Window *w);
extern SDL_Window *SDL_CreateWindow(const char *name,int x,int y,int w,int h,int val);
extern void SDL_DestroyWindow(SDL_Window *w);
extern void SDL_SetColorKey(SDL_Surface *s,int v1,int v2);
extern void SDL_SetSurfaceBlendMode(SDL_Surface *s,int v);
extern void SDL_SetSurfaceColorMod(SDL_Surface *s,int r,int g,int b);
extern int SDL_PollEvent(SDL_Event *e);
extern uint32_t SDL_MapRGB(SDL_MyFormat *s,int r,int g,int b);
#endif


#endif
