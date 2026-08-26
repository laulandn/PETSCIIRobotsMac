#ifndef _MAC
#define _MAC
#endif


#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>

#include <string.h>


char *strdup(const char *s)
{
  if(!s) return NULL;
  char *d=(char *)malloc(strlen(s));
  strcpy(d,s);
  return d;
}


#ifdef _MAC
SDL_Surface *SDL_CreateRGBSurface(uint32_t flags,uint32_t w,uint32_t h,uint32_t d,uint32_t rm,uint32_t gm,uint32_t bm,uint32_t am)
{
  fprintf(stderr,"SDL_CreateRGBSurface...\n"); fflush(stderr);
  SDL_Surface *s=(SDL_Surface *)malloc(sizeof(SDL_Surface));
  if(!s) { fprintf(stderr,"No memory...\n"); fflush(stderr); return NULL; }
  GWorldPtr gw;
  Rect r;  r.left=0; r.top=0;
  r.bottom=h; r.right=w;
  fprintf(stderr,"r is %d %d %d %d\n",r.left,r.top,r.right,r.bottom); fflush(stderr);
  QDErr err=NewGWorld(&gw,8,&r,NULL,NULL,0);
  if(err!=noErr) {
    fprintf(stderr,"QDErr was %d!\n",err); fflush(stderr);
	return NULL;
  }
  fprintf(stderr,"new gworld at %lx\n",(long)gw); fflush(stderr);
  s->g=gw;
  return s;
}
#endif


#ifdef _MAC
void SDL_FreeSurface(SDL_Surface *s)
{
  fprintf(stderr,"SDL_FreeSurface...\n"); fflush(stderr);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return; }
  if(s->g) DisposeGWorld(s->g);
  free(s);
}
#endif


#ifdef _MAC
void SDL_BlitSurface(SDL_Surface *s,SDL_Rect *sr,SDL_Surface *d,SDL_Rect *dr)
{
  //fprintf(stderr,"SDL_BlitSurface...\n"); fflush(stderr);
  if(!s) { fprintf(stderr,"SDL_BlitSurface s was NULL!\n"); fflush(stderr); return; }
  if(!s->g) { fprintf(stderr,"SDL_BlitSurface s->g was NULL!\n"); fflush(stderr); return; }
  if(!sr) { fprintf(stderr,"SDL_BlitSurface sr was NULL!\n"); fflush(stderr); return; }
  if(!d) { fprintf(stderr,"SDL_BlitSurface d was NULL!\n"); fflush(stderr); return; }
  if(!d->g) { fprintf(stderr,"SDL_BlitSurface d->g was NULL!\n"); fflush(stderr); return; }
  if(!dr) { fprintf(stderr,"SDL_BlitSurface dr was NULL!\n"); fflush(stderr); return; }
  //fprintf(stderr,"s at %lx\n",(long)s); fprintf(stderr,"d at %lx\n",(long)d);
  Rect msr;  msr.top=sr->y; msr.left=sr->x; 
  msr.bottom=sr->y+sr->h;  msr.right=sr->x+sr->w;
  Rect mdr;  mdr.top=dr->y; mdr.left=dr->x; 
  mdr.bottom=dr->y+dr->h;  mdr.right=dr->x+dr->w;
  //fprintf(stderr,"SDL_BlitSurface msr is %d %d %d %d\n",msr.left,msr.top,SDL_BlitSurface msr.right,msr.bottom);
  //fprintf(stderr,"SDL_BlitSurface mdr is %d %d %d %d\n",mdr.left,mdr.top,mdr.right,mdr.bottom);
  const BitMap *srcBits=NULL;  
  const BitMap *dstBits=NULL;
#if TARGET_API_CARBON
  srcBits=GetPortBitMapForCopyBits(s->g);
  //srcBits=(BitMap *)*GetGWorldPixMap(s->g);
  //dstBits=GetPortBitMapForCopyBits(d->g);
  dstBits=(BitMap *)*GetGWorldPixMap(d->g);
  //GrafPtr p;
  //GetPort(&p);
  //dstBits=GetPortBitMapForCopyBits(p);
  SetGWorld(d,NULL);
#else
  srcBits=(BitMap *)&((GrafPtr)s)->portBits;
  dstBits=(BitMap *)&((GrafPtr)d)->portBits;
#endif
  //
  CopyBits(srcBits,dstBits,&msr,&mdr,srcCopy,NULL);
}
#endif


#ifdef _MAC
void SDL_BlitScaled(SDL_Surface *s,SDL_Rect *sr,SDL_Surface *d,SDL_Rect *dr)
{
  //fprintf(stderr,"SDL_BlitScaled...\n"); fflush(stderr);
  SDL_BlitSurface(s,sr,d,dr);
}
#endif


#ifdef _MAC
void SDL_SetClipRect(SDL_Surface *s,SDL_Rect *sr)
{
  //fprintf(stderr,"SDL_SetClipRect...\n"); fflush(stderr);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return; }
  if(!sr) { fprintf(stderr,"sr was NULL!\n"); fflush(stderr); return; }
  Rect msr;  msr.top=sr->y; msr.left=sr->x; 
  msr.bottom=sr->y+sr->h;  msr.right=sr->x+sr->w;
  fprintf(stderr,"SDL_SetClipRect msr is %d %d %d %d\n",msr.left,msr.top,msr.right,msr.bottom); fflush(stderr);
  // TODO
}
#endif


#ifdef _MAC
void  SDL_FillRect(SDL_Surface *s,SDL_Rect *sr,uint32_t v)
{
  //fprintf(stderr,"SDL_FillRect...\n"); fflush(stderr);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return; }
  if(!sr) { fprintf(stderr,"sr was NULL!\n"); fflush(stderr); return; }
  Rect msr;  msr.top=sr->y; msr.left=sr->x; 
  msr.bottom=sr->y+sr->h;  msr.right=sr->x+sr->w;
  fprintf(stderr,"SDL_FillRect msr is %d %d %d %d\n",msr.left,msr.top,msr.right,msr.bottom); fflush(stderr);
  // TODO
}
#endif


#ifdef _MAC
void  SDL_FillRects(SDL_Surface *s,SDL_Rect *rs,uint32_t n,uint32_t v)
{
  fprintf(stderr,"SDL_FillRects...\n"); fflush(stderr);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return; }
  if(!rs) { fprintf(stderr,"rs was NULL!\n"); fflush(stderr); return; }
  if(n) { fprintf(stderr,"rs was NULL!\n"); fflush(stderr); return; }
  for(unsigned int t=0;t<n;t++) {
    SDL_Rect *sr=&(rs[t]);
    SDL_FillRect(s,sr,v);
  }
}
#endif


#ifdef _MAC
void  SDL_UpdateWindowSurface(SDL_Window *w)
{
  //fprintf(stderr,"SDL_UpdateWindowSurface...\n"); fflush(stderr);
  if(!w) { fprintf(stderr,"w was NULL!\n"); fflush(stderr); return; }
  // TODO?
}
#endif


const char *SDL_GetError(void)
{
  //fprintf(stderr,"SDL_GetError...not implemented\n"); fflush(stderr);
  return "Who knows?";
}


int SDL_Init(int val)
{
  //fprintf(stderr,"SDL_Init...not implemented\n"); fflush(stderr);
#if !TARGET_API_CARBON
  MaxApplZone();
  InitGraf(&(qd.thePort));
#endif
  FlushEvents(everyEvent,0);
#if !TARGET_API_CARBON
  InitWindows();
  InitMenus();
  InitDialogs(NULL);
  InitCursor();
#endif
  return 0;
}


void SDL_Quit(void)
{
  //fprintf(stderr,"SDL_Quit...not implemented\n"); fflush(stderr);
  exit(0);
}


void SDL_zero(SDL_AudioSpec)
{
  fprintf(stderr,"SDL_zero...not implemented\n"); fflush(stderr);
}


void SDL_SetHint(int what,const char *val)
{
  fprintf(stderr,"SDL_SetHint...not implemented\n"); fflush(stderr);
}


SDL_AudioDeviceID SDL_OpenAudioDevice(void *,int val,SDL_AudioSpec *want,SDL_AudioSpec *got,int setting)
{
  //fprintf(stderr,"SDL_OpenAudioDevice...not implemented\n"); fflush(stderr);
  return 0;
}


void SDL_PauseAudioDevice(SDL_AudioDeviceID a,int v)
{
  //fprintf(stderr,"SDL_PauseAudioDevice...not implemented\n"); fflush(stderr);
}


void SDL_CloseAudioDevice(SDL_AudioDeviceID a)
{
  fprintf(stderr,"SDL_CloseAudioDevice...not implemented\n"); fflush(stderr);
}


SDL_Joystick *SDL_JoystickOpen(int val)
{
  //fprintf(stderr,"SDL_JoystickOpen...not implemented\n"); fflush(stderr);
  return NULL;
}


void SDL_JoystickClose(SDL_Joystick *j)
{
  fprintf(stderr,"SDL_JoystickClose...not implemented\n"); fflush(stderr);
}


int SDL_JoystickGetAxis(SDL_Joystick *j,int val)
{
  fprintf(stderr,"SDL_JoystickGetAxis...not implemented\n"); fflush(stderr);
  return 0;
}


SDL_Surface *SDL_GetWindowSurface(SDL_Window *w)
{
  fprintf(stderr,"SDL_GetWindowSurface...\n"); fflush(stderr);
  SDL_Surface *s=(SDL_Surface *)malloc(sizeof(SDL_Surface));
  if(!s) { fprintf(stderr,"No memory...\n"); fflush(stderr); return NULL; }
  s->g=(GWorldPtr)w;
  return s;
}


SDL_Window *SDL_CreateWindow(const char *name,int x,int y,int w,int h,int val)
{
  fprintf(stderr,"Going to NewCWindow...\n"); fflush(stderr);
  Rect WindowBox;
  WindowBox.top=40;  WindowBox.left=4;
  WindowBox.bottom=PLATFORM_SCREEN_HEIGHT+40;  WindowBox.right=PLATFORM_SCREEN_WIDTH+4;
  WindowPtr window=NewCWindow(NULL,&WindowBox,(ConstStr255Param)"\pAttack of the PETSCII Robots",true,noGrowDocProc+8,(WindowPtr)(-1L),true,0L);
  SetPort((GrafPtr)window);
  ShowWindow((WindowPtr)window);
  fprintf(stderr,"(Window done)\n"); fflush(stderr);
  return (CGrafPtr)window;
}


void SDL_DestroyWindow(SDL_Window *w)
{
  fprintf(stderr,"SDL_DestroyWindow...not implemented\n"); fflush(stderr);
  //CloseWindow(w);
}


void SDL_SetColorKey(SDL_Surface *s,int v1,int v2)
{
  fprintf(stderr,"SDL_SetColorKey...not implemented\n"); fflush(stderr);
}


void SDL_SetSurfaceBlendMode(SDL_Surface *s,int v)
{
  fprintf(stderr,"SDL_SetSurfaceBlendMode...not implemented\n"); fflush(stderr);
}


void SDL_SetSurfaceColorMod(SDL_Surface *s,int r,int g,int b)
{
  fprintf(stderr,"SDL_SetSurfaceColorMod...not implemented\n"); fflush(stderr);
}


int SDL_PollEvent(SDL_Event *e)
{
  fprintf(stderr,"SDL_PollEvent...\n"); fflush(stderr);
  e->type=0;
  //
  EventRecord event;
  int type,val;
#if !TARGET_API_CARBON
  SystemTask();
#endif
  val=EventAvail(everyEvent,&event);
  if(val) {
	  GetNextEvent(everyEvent,&event);
    type=event.what;
    switch(type) {
      case nullEvent: break;
	    case keyDown:
        e->type=SDL_KEYDOWN;
	    case keyUp:
	      if(!e->type) e->type=SDL_KEYUP;
		    if(event.modifiers&cmdKey) {
          int mchoice=MenuKey(event.message&0xff);
          fprintf(stderr,"mac menu '%c' mchoice=%d\n",event.message&0xff,mchoice); fflush(stderr);
          if((event.message&0xff)=='q') {
            fprintf(stderr,"Command-Q...quiting...\n"); fflush(stderr);
            ExitToShell();
          }
        }
        else {
	        e->key.keysym.scancode=event.message&0xff;
   	  	  fprintf(stderr,"mac keypress '%c' (%d)\n",event.message&0xff,event.message&0xff); fflush(stderr);
        }
	      break;
	    case updateEvt:
	      break;
	    default:
	      fprintf(stderr,"mac event.what=%d skipped!\n",type); fflush(stderr);
	      break;
	  }
  }
  return 0;
}


uint32_t SDL_MapRGB(SDL_MyFormat *s,int r,int g,int b)
{
  fprintf(stderr,"SDL_MapRGB...not implemented\n"); fflush(stderr);
  return 0;
}


int IMG_Init(int val)
{
  //fprintf(stderr,"IMG_Init...not implemented\n"); fflush(stderr);
  return val;
}


const char *IMG_GetError(void)
{
  //fprintf(stderr,"IMG_GetError...not implemented\n"); fflush(stderr);
  return "Who knows?";
;
}


#ifdef _MAC
SDL_Surface *IMG_Load(const char *na)
{
  fprintf(stderr,"IMG_Load...\n"); fflush(stderr);
  if(!na) { fprintf(stderr,"na was NULL!\n"); fflush(stderr); return NULL; }
  char *n=strdup(na);
  n[strlen(n)-3]='r';
  n[strlen(n)-2]='a';
  n[strlen(n)-1]='w';
  FILE *f=fopen(n,"rb");
  if(!f) { fprintf(stderr,"Couldn't open %s!\n",n); fflush(stderr); return NULL; }
  uint32_t w,h,d,np;
  fread(&w,sizeof(uint32_t),1,f);
  fread(&h,sizeof(uint32_t),1,f);
  fread(&d,sizeof(uint32_t),1,f);
  fread(&np,sizeof(uint32_t),1,f);
#if 0
  w=ntohl(w); h=ntohl(h); d=ntohl(d); np=ntohl(np); 
#endif
  fprintf(stderr,"Image %s is %dx%dx%d(%d)\n",n,w,h,d,np); fflush(stderr);
  if((!d)||(d>32)) { fprintf(stderr,"Bad d %d!\n",d); fflush(stderr); return NULL; }
  if((!np)||(np>32)) { fprintf(stderr,"Bad np %d!\n",np); fflush(stderr); return NULL; }
  if((!w)||(w>1024)) { fprintf(stderr,"Bad w %d!\n",w); fflush(stderr); return NULL; }
  if((!h)||(h>10240)) { fprintf(stderr,"Bad h %d!\n",h); fflush(stderr); return NULL; }
  SDL_Surface *s=SDL_CreateRGBSurface(0,w,h,d,0,0,0,0);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return NULL; }
  if(!s->g) { fprintf(stderr,"s->g was NULL!\n"); fflush(stderr); return NULL; }
  //
  PixMapHandle pm=GetGWorldPixMap(s->g);
  if(!pm) { fprintf(stderr,"pm was NULL!\n"); fflush(stderr); return NULL; }
  LockPixels(pm);
  //
  unsigned int bpl=0;
#if TARGET_API_CARBON
  bpl=GetPixRowBytes(pm);
#else
  bpl=(*pm)->rowBytes&0x3fff;
#endif
  char *dst=GetPixBaseAddr(pm);
  fprintf(stderr,"bpl=%d\n",bpl); fflush(stderr);
  char c;
  unsigned int off=0;
  unsigned int sbpl=w;
  for(unsigned int r=0;r<h;r++) {
    for(unsigned int b=0;b<sbpl;b++) {
	  fread(&c,1,1,f);
	  dst[off+b]=c;
	}
	off+=bpl;
  }
  //
  UnlockPixels(pm);
  return s;
}
#endif


// Because the real SDL hides it
#undef main

#define STDOUT_FILE     "stdout.txt"
#define STDERR_FILE     "stderr.txt"


extern int main(int artc, char *argv[]);


int main(int argc,char **argv)
{
  freopen (STDOUT_FILE, "w", stdout);
  freopen (STDERR_FILE, "w", stderr);
  fprintf(stderr,"SDL main about to call SDL_main...\n"); fflush(stderr);
  return SDL_main(argc,argv);
}
