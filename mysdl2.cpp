#ifndef _MAC
#define _MAC
#endif


#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>

#include <string.h>

#define TARGET_API_MAC_CARBON 0  // Set to 1 if you are targeting Carbon
#define OPAQUE_TOOLBOX_STRUCTS 0
#define QuickTime_STUBS 1        // Forces exposure of deep QuickTime pipeline APIs

#include <QuickTimeComponents.h>


// We only support a single window and keep track of it...for now...
SDL_Window *__theOnlySDLWindow=NULL;
Rect __theOnlySDLWindowRect;
int __theGuessedSDLDepth=32;  // Just a guess for now...


char *strdup(const char *s)
{
  if(!s) return NULL;
  char *d=(char *)malloc(strlen(s));
  strcpy(d,s);
  return d;
}


void __myDumpSurface(SDL_Surface *s)
{
  fprintf(stderr,"*** Surface is1 g=%x isReallyWin=%d format=%x r=%d %d %d %d\n",(long)s->g,(int)s->isReallyWin,long(s->format),s->r.top,s->r.bottom,s->r.left,s->r.right);
  fprintf(stderr,"*** Surface is2 \"%s\" %d %d %d\n",s->name,s->width,s->height,s->depth);
  fflush(stderr);
}


short __GetMainScreenDepth(void) {
    GDHandle mainDevice;
    short depth = 0;
    
    mainDevice = GetMainDevice();
    if (mainDevice != NULL) {
        // Dereference the GDHandle to get the GrafDevice, 
        // then access gdPMap (PixMapHandle) and its pixelSize field.
        depth = (*(*mainDevice)->gdPMap)->pixelSize;
    }
    
    fprintf(stderr,"Monitor is set to %d depth\n",depth); fflush(stderr);
    return depth; // Returns 1, 2, 4, 8, 16, or 32 bits per pixel
}


#ifdef _MAC
SDL_Surface *SDL_CreateRGBSurface(uint32_t flags,uint32_t w,uint32_t h,uint32_t d,uint32_t rm,uint32_t gm,uint32_t bm,uint32_t am)
{
  fprintf(stderr,"SDL_CreateRGBSurface...\n"); fflush(stderr);
  //d=8; // Fake out and only create 8 bit surfaces...for now...
  /*if(d!=__theGuessedSDLDepth)
  {
    fprintf(stderr,"WARNING depth mismatch, asked %d, maybe should be %d\n",d,__theGuessedSDLDepth); fflush(stderr);
  }
  */
  SDL_Surface *s=(SDL_Surface *)malloc(sizeof(SDL_Surface));
  if(!s) { fprintf(stderr,"No memory...\n"); fflush(stderr); return NULL; }
  Rect r;  r.left=0; r.top=0;
  r.bottom=h; r.right=w;
  fprintf(stderr,"r is %d %d %d %d\n",r.left,r.top,r.right,r.bottom); fflush(stderr);
  //
  PixMap *pm=(PixMap *)calloc(1,sizeof(PixMap));
  if(!pm) { fprintf(stderr,"No pm...\n"); fflush(stderr); return NULL; }
  int pixSize=1;
  if(d==32) pixSize=4;
  if(d==24) pixSize=3;
  int rowBytes=w*pixSize;
  if(d!=(pixSize*8)) { fprintf(stderr,"WARNING Odd depth can't handle, sorry!\n"); fflush(stderr); return NULL; }
  fprintf(stderr,"using pixSize %d\n",pixSize); fflush(stderr);
  fprintf(stderr,"using rowBytes %d\n",rowBytes); fflush(stderr);
  fprintf(stderr,"requested depth %d\n",d); fflush(stderr);
  Ptr buffer=(Ptr)calloc(1,rowBytes*h);
  if(!pm) { fprintf(stderr,"No buffer...\n"); fflush(stderr); return NULL; }
  pm->baseAddr=buffer;
  pm->rowBytes=rowBytes|0x8000;
  pm->bounds=r;
  pm->pmVersion=0;
  pm->packType=0;
  pm->hRes=0x00480000;
  pm->vRes=0x00480000;
  pm->pixelType=RGBDirect;
  pm->pixelSize=d;
  pm->cmpCount=pixSize;
  pm->packType=4;
  pm->packSize=0;
#if TARGET_API_MAC_CARBON
    pm->pixelFormat = k24RGBPixelFormat; // for now
    pm->pmTable=NULL; 
    pm->pmExt=NULL;
#else
    pm->planeBytes=0; 
    pm->pmTable=NULL;//(*macport->portPixMap)->pmTable;
    pm->pmReserved=0;
#endif
  //
  s->g=pm;
  s->isReallyWin=false;
  s->format=NULL;
  s->name=strdup("anonymous");
  s->width=w;
  s->height=h;
  s->depth=d;
  s->r.top=r.top;
  s->r.bottom=r.bottom;
  s->r.left=r.left;
  s->r.right=r.right;
  //
  __myDumpSurface(s);
  //
  return s;
}
#endif


#ifdef _MAC
void SDL_FreeSurface(SDL_Surface *s)
{
  fprintf(stderr,"SDL_FreeSurface...\n"); fflush(stderr);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return; }
  //if(s->g) DisposeGWorld(s->g);
  // TODO actually free up custom pixmap
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
  //fprintf(stderr,"SDL_BlitSurface msr is %d %d %d %d\n",msr.left,msr.top, msr.right,msr.bottom); fflush(stderr);
  //fprintf(stderr,"SDL_BlitSurface mdr is %d %d %d %d\n",mdr.left,mdr.top,mdr.right,mdr.bottom); fflush(stderr);
  if(s->isReallyWin) { fprintf(stderr,"(s is win)\n"); fflush(stderr); }
  if(d->isReallyWin) { 
    //fprintf(stderr,"(d is win)\n"); fflush(stderr); 
    //PixMapHandle srcBits=GetGWorldPixMap(s->g);
    PixMapHandle srcBits=&s->g;
    GrafPtr dstBits=(GrafPtr)d->g;
    if(LockPixels(srcBits)) {
      CopyBits((BitMapPtr)*srcBits,&(dstBits->portBits),&msr,&mdr,srcCopy,NULL);
      //CopyBitsDeep((BitMapPtr)*srcBits,&(dstBits->portBits),&msr,&mdr,srcCopy,NULL,NULL,NULL);
      UnlockPixels(srcBits);
    }
  }
  else {
    PixMapHandle srcBits=&s->g;
    PixMapHandle dstBits=&d->g;
    //PixMapHandle srcBits=GetGWorldPixMap(s->g);
    //PixMapHandle dstBits=GetGWorldPixMap(d->g);
    if(LockPixels(srcBits)) {
      if(LockPixels(dstBits)) {
        CopyBits((BitMapPtr)*srcBits,(BitMapPtr)*dstBits,&msr,&mdr,srcCopy,NULL);
        //CopyBitsDeep((BitMapPtr)*srcBits,(BitMapPtr)*dstBits,&msr,&mdr,srcCopy,NULL,NULL,NULL);
        UnlockPixels(dstBits);
      }
      UnlockPixels(srcBits);
    }
  }
  /*
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
  srcBits=GetPortBitMapForCopyBits(s->g);
  //srcBits=(BitMap *)&((GrafPtr)s)->portBits;
  dstBits=(BitMap *)*GetPortBitMapForCopyBits(d->g);
  //dstBits=(BitMap *)&((GrafPtr)d)->portBits;
#endif
  //
  __myDumpSurface(s);
  __myDumpSurface(d);
  //
  PixMapHandle spm=GetGWorldPixMap(s->g);
  PixMapHandle dpm=GetGWorldPixMap(d->g);
  LockPixels(dpm);
  LockPixels(spm);
  CopyBits(srcBits,dstBits,&msr,&mdr,srcCopy,NULL);
  UnlockPixels(dpm);
  UnlockPixels(spm);
  */
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
  fprintf(stderr,"SDL_FillRect msr is %d %d %d %d with %x\n",msr.left,msr.top,msr.right,msr.bottom,v); fflush(stderr);
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
  // I think this is right...but not sure
  for(unsigned int t=0;t<n;t++) {
    SDL_Rect *sr=&(rs[t]);
    SDL_FillRect(s,sr,v);
  }
}
#endif


#ifdef _MAC
void  SDL_UpdateWindowSurface(SDL_Window *w)
{
  fprintf(stderr,"SDL_UpdateWindowSurface...\n"); fflush(stderr);
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
  SetGWorld( (GWorldPtr)GetWindowPort(FrontWindow()), NULL );
  if(__theOnlySDLWindow) CloseWindow((WindowRef)__theOnlySDLWindow);
  // TODO: Free up other stuff?
  exit(0);
}


void SDL_zero(SDL_AudioSpec)
{
  fprintf(stderr,"SDL_zero...not implemented\n"); fflush(stderr);
}


void SDL_SetHint(int what,const char *val)
{
  if(!val) {
    fprintf(stderr,"SDL_SetHint val was NULL!\n"); fflush(stderr);
    return;
  }
  fprintf(stderr,"SDL_SetHint %d %s...not implemented\n",what,val); fflush(stderr);
}


SDL_AudioDeviceID SDL_OpenAudioDevice(void *t,int val,SDL_AudioSpec *want,SDL_AudioSpec *got,int setting)
{
  if(!t) {
    fprintf(stderr,"SDL_OpenAudioDevice t was NULL!\n"); fflush(stderr);
    return 0;
  }
  if(!want) {
    fprintf(stderr,"SDL_OpenAudioDevice want was NULL!\n"); fflush(stderr);
    return 0;
  }
  fprintf(stderr,"SDL_OpenAudioDevice...not implemented\n"); fflush(stderr);
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
  fprintf(stderr,"SDL_JoystickOpen...not implemented\n"); fflush(stderr);
  return NULL;
}


void SDL_JoystickClose(SDL_Joystick *j)
{
  if(!j) {
    fprintf(stderr,"SDL_JoystickClose j was NULL!\n"); fflush(stderr);
    return;
  }
  fprintf(stderr,"SDL_JoystickClose...not implemented\n"); fflush(stderr);
}


int SDL_JoystickGetAxis(SDL_Joystick *j,int val)
{
  if(!j) {
    fprintf(stderr,"SDL_JoystickGetAxis j was NULL!\n"); fflush(stderr);
    return 0;
  }
  //fprintf(stderr,"SDL_JoystickGetAxis...not implemented\n"); fflush(stderr);
  return 0;
}


SDL_Surface *SDL_GetWindowSurface(SDL_Window *w)
{
  fprintf(stderr,"SDL_GetWindowSurface...\n"); fflush(stderr);
  if(!w) {
    fprintf(stderr,"SDL_GetWindowSurface w was NULL!\n"); fflush(stderr);
    return NULL;
  }
  //
  // Create fake surface for window
  // Need to make sure this looks as much like "real surface" as possible
  SDL_Surface *s=(SDL_Surface *)malloc(sizeof(SDL_Surface));
  if(!s) { fprintf(stderr,"No memory...\n"); fflush(stderr); return NULL; }
  s->g=(PixMap *)w;
  s->isReallyWin=true;
  s->format=NULL;
  s->name=(char *)"__theOnlySDLWindow";
  s->r.top=__theOnlySDLWindowRect.top;
  s->r.bottom=__theOnlySDLWindowRect.bottom;
  s->r.left=__theOnlySDLWindowRect.left;
  s->r.right=__theOnlySDLWindowRect.right;
  s->width=s->r.right-s->r.left;
  s->height=s->r.bottom-s->r.top;
  s->depth=__theGuessedSDLDepth;
  //
  __myDumpSurface(s);
  //
  return s;
}


SDL_Window *SDL_CreateWindow(const char *name,int x,int y,int w,int h,int val)
{
  if(__theOnlySDLWindow) {
    fprintf(stderr,"WARNING already have a window...only one supported for now\n"); fflush(stderr);
    SDL_Quit();
    return NULL;
  }
  //
  fprintf(stderr,"Going to NewCWindow...\n"); fflush(stderr);
  Rect WindowBox;
  WindowBox.top=40;  WindowBox.left=4;
  WindowBox.bottom=PLATFORM_SCREEN_HEIGHT+40;  WindowBox.right=PLATFORM_SCREEN_WIDTH+4;
  WindowPtr window=NewCWindow(NULL,&WindowBox,(ConstStr255Param)"\pAttack of the PETSCII Robots",true,noGrowDocProc+8,(WindowPtr)(-1L),true,0L);
  if(!window) { fprintf(stderr,"No window...\n"); fflush(stderr); return NULL; }
  SetPort((GrafPtr)window);
  ShowWindow((WindowPtr)window);
  __theOnlySDLWindowRect.top=WindowBox.top;
  __theOnlySDLWindowRect.bottom=WindowBox.bottom;
  __theOnlySDLWindowRect.left=WindowBox.left;
  __theOnlySDLWindowRect.right=WindowBox.right;
  fprintf(stderr,"(Window done)\n"); fflush(stderr);
  __theOnlySDLWindow=(SDL_Window *)window;
  __theGuessedSDLDepth=__GetMainScreenDepth();
  return (CGrafPtr)window;
}


void SDL_DestroyWindow(SDL_Window *w)
{
  if(!w) {
    fprintf(stderr,"SDL_DestroyWindow w was NULL!\n"); fflush(stderr);
    return;
  }
  fprintf(stderr,"SDL_DestroyWindow...not implemented\n"); fflush(stderr);
  //CloseWindow(w);
}


void SDL_SetColorKey(SDL_Surface *s,int v1,int v2)
{
  if(!s) {
    fprintf(stderr,"SDL_SetColorKey s was NULL!\n"); fflush(stderr);
    return;
  }
  fprintf(stderr,"SDL_SetColorKey...not implemented\n"); fflush(stderr);
}


void SDL_SetSurfaceBlendMode(SDL_Surface *s,int v)
{
  if(!s) {
    fprintf(stderr,"SDL_SetSurfaceBlendMode s was NULL!\n"); fflush(stderr);
    return;
  }
  fprintf(stderr,"SDL_SetSurfaceBlendMode...not implemented\n"); fflush(stderr);
}


void SDL_SetSurfaceColorMod(SDL_Surface *s,int r,int g,int b)
{
  if(!s) {
    fprintf(stderr,"SDL_SetSurfaceColorMod s was NULL!\n"); fflush(stderr);
    return;
  }
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
            SDL_Quit();
            exit(0);
          }
        }
        else {
	        e->key.keysym.scancode=event.message&0xff;
   	  	  fprintf(stderr,"mac keypress '%c' (%x)\n",event.message&0xff,event.message); fflush(stderr);
        }
        e->key.repeat=0;
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
  if(!s) {
    fprintf(stderr,"SDL_MapRGB s was NULL!\n"); fflush(stderr);
    return 0;
  }
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
  uint32_t width,height,depth,nplanes;
  fread(&width,sizeof(uint32_t),1,f);
  fread(&height,sizeof(uint32_t),1,f);
  fread(&depth,sizeof(uint32_t),1,f);
  fread(&nplanes,sizeof(uint32_t),1,f);
#if 0
  // This is for when endian doesn't match raw file...
  // should be able to detect it as the multibyte width or height will be "weird"
  width=ntohl(width); height=ntohl(height); depth=ntohl(depth); nplanes=ntohl(nplanes); 
#endif
  //
  fprintf(stderr,"Image \"%s\" is %dx%dx%d(%d)\n",n,width,height,depth,nplanes); fflush(stderr);
  if((!depth)||(depth>32)) { fprintf(stderr,"Bad depth %d!\n",depth); fflush(stderr); return NULL; }
  if((!nplanes)||(nplanes>32)) { fprintf(stderr,"Bad nplanes %d!\n",nplanes); fflush(stderr); return NULL; }
  if((!width)||(width>1024)) { fprintf(stderr,"Bad width %d!\n",width); fflush(stderr); return NULL; }
  if((!height)||(height>10240)) { fprintf(stderr,"Bad height %d!\n",height); fflush(stderr); return NULL; }
  SDL_Surface *s=SDL_CreateRGBSurface(0,width,height,depth,0,0,0,0);
  if(!s) { fprintf(stderr,"s was NULL!\n"); fflush(stderr); return NULL; }
  if(!s->g) { fprintf(stderr,"s->g was NULL!\n"); fflush(stderr); return NULL; }
  s->name=strdup(n);
  //
  PixMapHandle pm=&s->g;
  if(!pm) { fprintf(stderr,"pm was NULL!\n"); fflush(stderr); return NULL; }
#if TARGET_API_MAC_CARBON
  short qddepth = GetPixDepth(pm);
#else
  short qddepth = (**pm).pixelSize;
#endif
  if(qddepth!=depth) {
    fprintf(stderr,"WARNING we asked for %d but qd gave us %d, will handle...\n",depth,qddepth); fflush(stderr);
  }
  //
  LockPixels(pm);
  //
  unsigned int pixSize=1;
  if(depth>8) pixSize=depth/8;
  if(depth!=(pixSize*8)) { fprintf(stderr,"WARNING Odd depth can't handle, sorry!\n"); fflush(stderr); return NULL; }
  if(depth<8) { fprintf(stderr,"WARNING Can't handle less than 8 bit pixels, sorry!\n"); fflush(stderr); return NULL; }
  fprintf(stderr,"pixSize=%d\n",pixSize); fflush(stderr);
  //
  
  unsigned int dBpl = 0, sBpl = 0;
  dBpl = GetPixRowBytes(pm);
  sBpl = width * pixSize;
  
  fprintf(stderr, "dBpl=%u sBpl=%u\n", dBpl, sBpl); fflush(stderr);
  
  // Use size_t for memory sizes to prevent 32-bit overflow
  size_t totalrsize = (size_t)sBpl * height; 
  size_t totalwsize = (size_t)dBpl * height;
  fprintf(stderr, "will read totalrsize=%zu will write totalwsize=%zu\n", totalrsize, totalwsize); fflush(stderr);
  
  char *dst = GetPixBaseAddr(pm);
  fprintf(stderr, "dst is %x\n", (int)dst); fflush(stderr);
  
  fprintf(stderr, "Going to read...\n"); fflush(stderr);  
  uint8_t* row_buffer = (uint8_t*)malloc(sBpl);
  if (!row_buffer) {
      fprintf(stderr, "No row_buffer!\n"); fflush(stderr);
      SDL_Quit();
      exit(1);
  }  
  
  size_t off = 0;
  size_t count = 0;
  
  for(int r = 0; r < height; r++) {
    size_t br = fread(row_buffer, 1, sBpl, f);
    if(br != sBpl) {
        fprintf(stderr, "Problem reading at row %d!\n", r); fflush(stderr);
        free(row_buffer); // Free memory before exiting
        SDL_Quit();
        exit(1);
    }
    count += br;
    
    for(int i = 0; i < width; i++) {
      for(int p = 0; p < pixSize; p++) {
        dst[off + (i * pixSize) + p] = row_buffer[(i * pixSize) + p];
      }
    }
    off += dBpl;
  }
  
  free(row_buffer);

  //
  __myDumpSurface(s);
  //
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
