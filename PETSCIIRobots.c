#include <SAT.h>
#include "myPlatform.h"

SATPatHandle	thepat;
SpritePtr		ignoreSp;
Point			p;
SpritePtr		playerSp;
WindowPtr		gWind;
short			gVBLInstalled;
Rect 			r;


enum {
	scrollSizeH = 512,
	scrollSizeV = 384
};

Point nowOff = {0,0};

#define max(x, y)	(x>y?x:y)
#define min(x, y)	(x>y?y:x)
#define abs(x)		((x)>0?(x):-(x))

pascal Boolean ScrollScreen() {
	#define	sco	8	//the speed of the scroll, number of pixels per *tick*

	long startTicks, frameTime = 1;

	Rect srcRect;
	Point	where,were;

// If the player sprite is at the border, scroll!
	if ((playerSp->position.h + 64 > scrollSizeH + gSAT.wind.port->portRect.left) ||
		(playerSp->position.h < gSAT.wind.port->portRect.left) || 
		(playerSp->position.v + 64 > scrollSizeV + gSAT.wind.port->portRect.top) ||
		(playerSp->position.v  < gSAT.wind.port->portRect.top) ) {
		were = nowOff;
		where = playerSp->position;
		where.h -= (scrollSizeH >> 1);
		where.v -= (scrollSizeV >> 1);
		if (where.h < 0)
			where.h = 0;
		if (where.v < 0)
			where.v = 0;
		if (where.h + scrollSizeH > gSAT.offSizeH)
			where.h = gSAT.offSizeH - scrollSizeH;
		if (where.v + scrollSizeV > gSAT.offSizeV)
			where.v = gSAT.offSizeV - scrollSizeV;

		where.h &= 0xfff8; // Scroll only to multiples of 8, so we won't confuse the 4-bit and 1-bit blitters!

		do
		{
			startTicks = TickCount();

			if (nowOff.h > where.h) 
				nowOff.h = max(nowOff.h - sco*frameTime, where.h);
			if (nowOff.h < where.h) 
				nowOff.h = min(nowOff.h + sco*frameTime, where.h);
			if (nowOff.v > where.v) 
				nowOff.v = max(nowOff.v - sco*frameTime, where.v);
			if (nowOff.v < where.v) 
				nowOff.v = min(nowOff.v + sco*frameTime, where.v);
			SATSetPortScreen();
			SetOrigin(nowOff.h, nowOff.v);
			gSAT.wind.bounds = gSAT.wind.port->portRect; // Synch gSAT.wind.bounds with the portRect!
			srcRect = gSAT.wind.port->portRect;

			CopyBits(&gSAT.offScreen.port->portBits, &gSAT.wind.port->portBits, &srcRect, &srcRect, srcCopy, nil);

			frameTime = TickCount() - startTicks;
		}
		while ((nowOff.h != where.h) || (nowOff.v != where.v));
	}

} //ScrollScreen

void SetupWind() {
	Rect zr;
	SysEnvRec wrld;

	//¥ Since SAT hasn't been initialized, we can't use gSAT.colorFlag but 
	//¥ have to check environs ourselves.
	if ( noErr != SysEnvirons(1, &wrld))
		;//¥ ignore errors.

	SetRect(&zr, 0, 0, scrollSizeH, scrollSizeV);
	OffsetRect(&zr, (qd.screenBits.bounds.right - qd.screenBits.bounds.left - scrollSizeH) / 2,
					(qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - scrollSizeV) / 2);

	if ( wrld.hasColorQD ) {
		gWind = NewCWindow(nil, &zr, "\p", false, plainDBox, (WindowPtr)-1L, false, 0);
	} else {
		gWind = NewWindow(nil, &zr, "\p", false, plainDBox, (WindowPtr)-1L, false, 0); 
	}
} //SetupWind

Boolean IsOptionPressed(void) {
	KeyMap km;

	GetKeys(km);
	return (km[1] & 4)!=0;
}

Boolean IsPressed(unsigned short k) {
	unsigned char km[16];
	
	GetKeys((unsigned long *)km);
	return((km[k>>3]>>(k&7))&1);
}

main () {
	Rect		tempRect;
	EventRecord	e;
	long	startTicks;
	
	SATInitToolbox();
	GetDateTime((unsigned long *)&qd.randSeed);
	
	SATConfigure(true, kVPositionSort, kBackwardCollision, 64);
	SetupWind ();
	
	SetRect(&r, 0, 0, 1000, 484);	//the offscreen size
	SATCustomInit(0, 0, &r, gWind, nil, false, false, false, true, false);
	SATSetSpriteRecSize(sizeof(SATSprite));
	
	ShowWindow(gSAT.wind.port);
	SelectWindow(gSAT.wind.port);
	SATHideMBar(gWind);
	
	
	SATRedraw();
	

	gSAT.wind.bounds = (*gSAT.wind.port).portRect;
	SATRedraw();
	SATSetPortScreen();
	do {
		startTicks = TickCount();
		SATRun(true);	//!IsOptionPressed()
		ScrollScreen();
	//	if (IsOptionPressed())
		//	DrawProgrammerInfo();
						//if (IsOptionPressed()) if (WaitNextEvent ( everyEvent, &e, 10, nil )) ;
		while (TickCount() < startTicks + 1); // At least SOME speed limit! /Ingemar
	} while (! Button ());

	SATSetPortScreen();
#ifndef __MWERKS__
	if (gVBLInstalled)
		RemoveVBLCounter();
#endif
	SATSoundShutup();
	
	ShowCursor();
	FlushEvents(everyEvent, 0);
} //main
