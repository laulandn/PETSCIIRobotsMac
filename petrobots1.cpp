/**
 * PETSCII Robots (Portable version)
 * by David Murray 2020
 * dfwgreencars@gmail.com
 * ported by Vesa Halttunen 2021-2022
 * vesuri@jormas.com
 */
 
#include <SAT.h>

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

#include "PlatformSAT.h"
#include "petrobots.h"

// MAP FILES CONSIST OF EVERYTHING FROM THIS POINT ON
uint8_t MAP_DATA[8960];
// END OF MAP FILE

uint8_t* DESTRUCT_PATH; // Destruct path array (256 bytes)
uint8_t* TILE_ATTRIB;   // Tile attrib array (256 bytes)
#ifndef PLATFORM_SPRITE_SUPPORT
uint8_t* TILE_DATA_TL;  // Tile character top-left (256 bytes)
uint8_t* TILE_DATA_TM;  // Tile character top-middle (256 bytes)
uint8_t* TILE_DATA_TR;  // Tile character top-right (256 bytes)
uint8_t* TILE_DATA_ML;  // Tile character middle-left (256 bytes)
uint8_t* TILE_DATA_MM;  // Tile character middle-middle (256 bytes)
uint8_t* TILE_DATA_MR;  // Tile character middle-right (256 bytes)
uint8_t* TILE_DATA_BL;  // Tile character bottom-left (256 bytes)
uint8_t* TILE_DATA_BM;  // Tile character bottom-middle (256 bytes)
uint8_t* TILE_DATA_BR;  // Tile character bottom-right (256 bytes)
#endif

// These arrays can go anywhere in RAM
uint8_t UNIT_TIMER_A[64];   // Primary timer for units (64 bytes)
uint8_t UNIT_TIMER_B[64];   // Secondary timer for units (64 bytes)
uint8_t UNIT_TILE[32];      // Current tile assigned to unit (32 bytes)
uint8_t UNIT_DIRECTION[32]; // Movement direction of unit (32 bytes)
uint8_t EXP_BUFFER[16];     // Explosion Buffer (16 bytes)
uint8_t MAP_PRECALC[MAP_WINDOW_SIZE];    // Stores pre-calculated objects for map window (77 bytes)
uint8_t MAP_PRECALC_DIRECTION[MAP_WINDOW_SIZE];    // Stores pre-calculated object directions for map window (77 bytes)
uint8_t MAP_PRECALC_TYPE[MAP_WINDOW_SIZE];    // Stores pre-calculated object types for map window (77 bytes)
#ifdef OPTIMIZED_MAP_RENDERING
uint8_t PREVIOUS_MAP_BACKGROUND[MAP_WINDOW_SIZE];
uint8_t PREVIOUS_MAP_BACKGROUND_VARIANT[MAP_WINDOW_SIZE];
uint8_t PREVIOUS_MAP_FOREGROUND[MAP_WINDOW_SIZE];
uint8_t PREVIOUS_MAP_FOREGROUND_VARIANT[MAP_WINDOW_SIZE];
#endif

// The following are the locations where the current
// key controls are stored.  These must be set before
// the game can start.
uint8_t KEY_CONFIG[26];
enum KEYS {
    KEY_MOVE_UP,
    KEY_MOVE_DOWN,
    KEY_MOVE_LEFT,
    KEY_MOVE_RIGHT,
    KEY_FIRE_UP,
    KEY_FIRE_DOWN,
    KEY_FIRE_LEFT,
    KEY_FIRE_RIGHT,
    KEY_CYCLE_WEAPONS,
    KEY_CYCLE_ITEMS,
    KEY_USE,
    KEY_SEARCH,
    KEY_MOVE,
    KEY_LIVE_MAP,
    KEY_LIVE_MAP_ROBOTS,
    KEY_PAUSE,
    KEY_MUSIC,
    KEY_CHEAT,
    KEY_CURSOR_UP,
    KEY_CURSOR_DOWN,
    KEY_CURSOR_LEFT,
    KEY_CURSOR_RIGHT,
    KEY_SPACE,
    KEY_RETURN,
    KEY_YES,
    KEY_NO
};

uint8_t TILE;           // The tile number to be plotted
uint8_t DIRECTION;      // The direction of the tile to be plotted
uint8_t WALK_FRAME;     // Player walking animation frame
uint8_t DEMATERIALIZE_FRAME; // Dematerialize animation frame
uint8_t MAP_X;          // Current X location on map
uint8_t MAP_Y;          // Current Y location on map
uint8_t MAP_WINDOW_X;   // Top left location of what is displayed in map window
uint8_t MAP_WINDOW_Y;   // Top left location of what is displayed in map window
uint8_t DECNUM;         // a decimal number to be displayed onscreen as 3 digits.
uint8_t ATTRIB;         // Tile attribute value
uint8_t UNIT;           // Current unit being processed
uint8_t TEMP_A;         // used within some routines
uint8_t TEMP_B;         // used within some routines
uint8_t TEMP_C;         // used within some routines
uint8_t TEMP_D;         // used within some routines
uint8_t CURSOR_X;       // For on-screen cursor
uint8_t CURSOR_Y;       // For on-screen cursor
uint8_t CURSOR_ON;      // Is cursor active or not? 1=yes 0=no
uint8_t REDRAW_WINDOW;  // 1=yes 0=no
uint8_t MOVE_RESULT;    // 1=Move request success, 0=fail.
uint8_t UNIT_FIND;      // 255=no unit present.
uint8_t MOVE_TYPE;      // %00000001=WALK %00000010=HOVER
uint8_t* CUR_PATTERN;   // stores the memory location of the current musical pattern being played.

uint8_t* MAP_SOURCE;    // $FD
uint8_t SCREEN_MEMORY[SCREEN_WIDTH_IN_CHARACTERS * SCREEN_HEIGHT_IN_CHARACTERS]; // $8000

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


  return false;
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
	
	GetKeys((long *)km);
	return((km[k>>3]>>(k&7))&1);
}

/*

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

*/

int main(int argc, char *argv[])
{
    PlatformClass platformInstance;

    if (!platform) {
        return 1;
    }

    convertToPETSCII(INTRO_MESSAGE);
    convertToPETSCII(MSG_CANTMOVE);
    convertToPETSCII(MSG_BLOCKED);
    convertToPETSCII(MSG_SEARCHING);
    convertToPETSCII(MSG_NOTFOUND);
    convertToPETSCII(MSG_FOUNDKEY);
    convertToPETSCII(MSG_FOUNDGUN);
    convertToPETSCII(MSG_FOUNDEMP);
    convertToPETSCII(MSG_FOUNDBOMB);
    convertToPETSCII(MSG_FOUNDPLAS);
    convertToPETSCII(MSG_FOUNDMED);
    convertToPETSCII(MSG_FOUNDMAG);
    convertToPETSCII(MSG_MUCHBET);
    convertToPETSCII(MSG_EMPUSED);
    convertToPETSCII(MSG_TERMINATED);
    convertToPETSCII(MSG_TRANS1);
    convertToPETSCII(MSG_ELEVATOR);
    convertToPETSCII(MSG_LEVELS);
    convertToPETSCII(MSG_PAUSED);
    convertToPETSCII(MSG_MUSICON);
    convertToPETSCII(MSG_MUSICOFF);
    convertToPETSCII(MAP_NAMES);
    convertToPETSCII(LOAD_MSG2);
    convertToPETSCII(INTRO_OPTIONS);
    convertToPETSCII(DIFF_LEVEL_WORDS);
    convertToPETSCII(WIN_MSG);
    convertToPETSCII(LOS_MSG);
    convertToPETSCII(CONTROLTEXT);
    convertToPETSCII(CINEMA_MESSAGE);

    for (int i = 0; i < PLATFORM_MAP_WINDOW_TILES_HEIGHT; i++) {
        MAP_CHART[i] = i * 3 * SCREEN_WIDTH_IN_CHARACTERS;
    }

    platform->stopNote(); // RESET SOUND TO ZERO
#ifdef PLATFORM_STDOUT_MESSAGES
    DISPLAY_LOAD_MESSAGE1();
#endif
    TILE_LOAD_ROUTINE();
    SETUP_INTERRUPT();
    SET_CONTROLS(); // copy initial key controls
    while (!platform->quit) {
        INTRO_SCREEN();
    }
    return 0;
}

void INIT_GAME()
{
    SCREEN_SHAKE = 0;
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    LIVE_MAP_ON = 0;
#ifdef PLATFORM_LIVE_MAP_SINGLE_KEY
    LIVE_MAP_ROBOTS_ON = 0;
#endif
#endif
    RESET_KEYS_AMMO();
    platform->fadeScreen(0, false);
    DISPLAY_GAME_SCREEN();
#ifndef INACTIVITY_TIMEOUT_GAME
    DISPLAY_LOAD_MESSAGE2();
#endif
    platform->fadeScreen(15, false);
    MAP_LOAD_ROUTINE();
#ifdef PLATFORM_MODULE_BASED_AUDIO
    START_IN_GAME_MUSIC();
#endif
    SET_DIFF_LEVEL();
    ANIMATE_PLAYER();
    CACULATE_AND_REDRAW();
#ifdef OPTIMIZED_MAP_RENDERING
    INVALIDATE_PREVIOUS_MAP();
#endif
    DRAW_MAP_WINDOW();
    DISPLAY_PLAYER_HEALTH();
    DISPLAY_KEYS();
    DISPLAY_WEAPON();
    UNIT_TYPE[0] = 1;
    SET_INITIAL_TIMERS();
    PRINT_INTRO_MESSAGE();
    KEYTIMER = 30;
    MAIN_GAME_LOOP();
}

//char MAPNAME[] = "level-a";
#ifdef PLATFORM_STDOUT_MESSAGES
const char* LOADMSG1 = "loading tiles...\x0d";
#endif
uint8_t KEYS = 0; // bit0=spade bit2=heart bit3=star
uint8_t AMMO_PISTOL = 0; // how much ammo for the pistol
uint8_t AMMO_PLASMA = 0; // how many shots of the plasmagun
uint8_t INV_BOMBS = 0; // How many bombs do we have
uint8_t INV_EMP = 0; // How many EMPs do we have
uint8_t INV_MEDKIT = 0; // How many medkits do we have?
uint8_t INV_MAGNET = 0; // How many magnets do we have?
uint8_t SELECTED_WEAPON = 0; // 0=none 1=pistol 2=plasmagun
uint8_t SELECTED_ITEM = 0; // 0=none 1=bomb 2=emp 3=medkit 4=magnet
uint8_t SELECT_TIMEOUT = 0; // can only change weapons once it hits zero
uint8_t ANIMATE = 1; // 0=DISABLED 1=ENABLED
uint8_t BIG_EXP_ACT = 0; // 0=No explosion active 1=big explosion active
uint8_t MAGNET_ACT = 0; // 0=no magnet active 1=magnet active
uint8_t PLASMA_ACT = 0; // 0=No plasma fire active 1=plasma fire active
uint8_t RANDOM = 0; // used for random number generation
uint8_t BORDER = 0; // Used for border flash timing
uint8_t SCREEN_SHAKE = 0; // 1=shake 0=no shake
uint8_t CONTROL = PLATFORM_DEFAULT_CONTROL; // 0=keyboard 1=custom keys 2=snes 3=analog
uint16_t BORDER_COLOR = 0xf00; // Used for border flash coloring
char INTRO_MESSAGE[] = "welcome to "
                       PLATFORM_NAME
                       "-robots!\xff"
                       "by david murray 2021\xff"
                       PLATFORM_NAME
                       " port by vesa halttunen";
char MSG_CANTMOVE[] = "can't move that!";
char MSG_BLOCKED[] = "blocked!";
char MSG_SEARCHING[] = "searching";
char MSG_NOTFOUND[] = "nothing found here.";
char MSG_FOUNDKEY[] = "you found a key card!";
char MSG_FOUNDGUN[] = "you found a pistol!";
char MSG_FOUNDEMP[] = "you found an emp device!";
char MSG_FOUNDBOMB[] = "you found a timebomb!";
char MSG_FOUNDPLAS[] = "you found a plasma gun!";
char MSG_FOUNDMED[] = "you found a medkit!";
char MSG_FOUNDMAG[] = "you found a magnet!";
char MSG_MUCHBET[] = "ahhh, much better!";
char MSG_EMPUSED[] = "emp activated!\xff"
                     "nearby robots are rebooting.";
char MSG_TERMINATED[] = "you're terminated!";
char MSG_TRANS1[] = "transporter will not activate\xff"
                    "until all robots destroyed.";
char MSG_ELEVATOR[] = "[ elevator panel ]  down\xff"
                      "[  select level  ]  opens";
char MSG_LEVELS[] = "[                ]  door";
#ifdef INACTIVITY_TIMEOUT_GAME
char MSG_PAUSED[] = "exit game?\xff"
                    "left=yes right=no";
#else
char MSG_PAUSED[] = "game paused.\xff"
                    "exit game (y/n)";
#endif
char MSG_MUSICON[] = "music on.";
char MSG_MUSICOFF[] = "music off.";
uint8_t SELECTED_MAP = 0;
char MAP_NAMES[] = "01-research lab "
                   "02-headquarters "
                   "03-the village  "
                   "04-the islands  "
                   "05-downtown     "
                   "06-pi university"
                   "07-more islands "
                   "08-robot hotel  "
                   "09-forest moon  "
                   "10-death tower  "
                   "11-river death  "
                   "12-bunker       "
                   "13-castle robot "
                   "14-rocket center";

#ifdef PLATFORM_MODULE_BASED_AUDIO
uint8_t MUSIC_ON = 1; // 0=off 1=on
#else
// THE FOLLOWING ARE USED BY THE SOUND SYSTEM*
uint8_t TEMPO_TIMER = 0; // used for counting down to the next tick
uint8_t TEMPO = 7; // How many IRQs between ticks
uint8_t DATA_LINE = 0; // used for playback to keep track of which line we are executing.
uint8_t ARP_MODE = 0; // 0=no 1=major 2=minor 3=sus4
uint8_t CHORD_ROOT = 0; // root note of the chord
uint8_t MUSIC_ON = 0; // 0=off 1=on
uint8_t SOUND_EFFECT = 0xff; // FF=OFF or number of effect in progress
#endif

#ifdef PLATFORM_STDOUT_MESSAGES
void DISPLAY_LOAD_MESSAGE1()
{
    for (int Y = 0; Y != 17; Y++) {
        platform->chrout(LOADMSG1[Y]);
    }
}
#endif

// Displays loading message for map.
void DISPLAY_LOAD_MESSAGE2()
{
    int Y;
    for (Y = 0; Y != 12; Y++) {
        writeToScreenMemory((PLATFORM_SCREEN_HEIGHT - 32) / 2 / 8 * SCREEN_WIDTH_IN_CHARACTERS + (PLATFORM_SCREEN_WIDTH - 320) / 2 / 8 + Y, LOAD_MSG2[Y]);
    }
    char* name = CALC_MAP_NAME();
    for (Y = 0; Y != 16; Y++) {
        writeToScreenMemory((PLATFORM_SCREEN_HEIGHT - 32) / 2 / 8 * SCREEN_WIDTH_IN_CHARACTERS + (PLATFORM_SCREEN_WIDTH - 320) / 2 / 8 + 12 + Y, name[Y]);
    }
}

char LOAD_MSG2[] = "loading map:";

void SETUP_INTERRUPT()
{
    platform->setInterrupt(&RUNIRQ);
}

// This is the routine that runs every 60 seconds from the IRQ.
// BGTIMER1 is always set to 1 every cycle, after which the main
// program will reset it to 0 when it is done with it's work for
// that cycle.  BGTIMER2 is a count-down to zero and then stays
// there.
void RUNIRQ()
{
#ifndef PLATFORM_MODULE_BASED_AUDIO
    MUSIC_ROUTINE();
#endif
    UPDATE_GAME_CLOCK();
    ANIMATE_WATER();
    BGTIMER1 = 1;
    if (BGTIMER2 != 0) {
        BGTIMER2--;
    }
    if (KEYTIMER != 0) {
        KEYTIMER--;
    }
    if (BORDER != 0) {
        BORDER--;
        platform->fadeScreen(15 - BORDER);
    }
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    if (CLOCK_ACTIVE != 0 && SCREEN_SHAKE != 0) {
        platform->shakeScreen();
    }
#endif
    // Back to usual IRQ routine
}
uint8_t BGTIMER1 = 0;
uint8_t BGTIMER2 = 0;
uint8_t KEYTIMER = 0; // Used for repeat of movement

// Since the PET has no real-time clock, and the Jiffy clock
// is a pain to read from assembly language, I have created my own.
void UPDATE_GAME_CLOCK()
{
    if (CLOCK_ACTIVE != 1) {
        return;
    }
    CYCLES++;
    if (CYCLES != platform->framesPerSecond()) { // 60 for ntsc or 50 for pal
        return;
    }
    CYCLES = 0;
    SECONDS++;
#ifdef INACTIVITY_TIMEOUT_GAME
    INACTIVE_SECONDS++;
#endif
    if (SECONDS != 60) {
        return;
    }
    SECONDS = 0;
    MINUTES++;
    if (MINUTES != 60) {
        return;
    }
    SECONDS = 0;
    MINUTES = 0;
    HOURS++;
}

uint8_t HOURS = 0;
uint8_t MINUTES = 0;
uint8_t SECONDS = 0;
uint8_t CYCLES = 0;
uint8_t CLOCK_ACTIVE = 0;
#ifdef INACTIVITY_TIMEOUT_GAME
uint8_t INACTIVE_SECONDS = 0;
#endif

// This routine spaces out the timers so that not everything
// is running out once. It also starts the game_clock.
void SET_INITIAL_TIMERS()
{
    CLOCK_ACTIVE = 1;
    for (int X = 1; X != 48; X++) {
        UNIT_TIMER_A[X] = X;
        UNIT_TIMER_B[X] = 0;
    }
}

void MAIN_GAME_LOOP()
{
    platform->renderFrame();
    bool done = false;
    while (!done && !platform->quit) {
#ifdef INACTIVITY_TIMEOUT_GAME
        if (INACTIVE_SECONDS >= INACTIVITY_TIMEOUT_GAME) {
            return;
        }
#endif
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] != 1) { // Is player unit alive
            GAME_OVER();
            return;
        }
        KEY_REPEAT(platform->isKeyOrJoystickPressed(CONTROL >= 2 ? true : false));
        uint8_t A = platform->readKeyboard();
        uint16_t B = platform->readJoystick(CONTROL >= 2 ? true : false);
        // Keyboard controls here.
        if (A != 0xff) {
#ifdef INACTIVITY_TIMEOUT_GAME
            INACTIVE_SECONDS = 0;
#endif
            KEYTIMER = 5;
            if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT]) { // CURSOR RIGHT
                UNIT = 0;
                MOVE_TYPE = 1; // %00000001
                REQUEST_WALK_RIGHT();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT]) { // CURSOR LEFT
                UNIT = 0;
                MOVE_TYPE = 1;  // %00000001
                REQUEST_WALK_LEFT();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                UNIT = 0;
                MOVE_TYPE = 1;  // %00000001
                REQUEST_WALK_DOWN();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP]) { // CURSOR UP
                UNIT = 0;
                MOVE_TYPE = 1;  // %00000001
                REQUEST_WALK_UP();
                AFTER_MOVE();
            } else if (A == KEY_CONFIG[KEY_CYCLE_WEAPONS]) {
                CYCLE_WEAPON();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_CYCLE_ITEMS]) {
                CYCLE_ITEM();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_MOVE]) {
                MOVE_OBJECT();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_SEARCH]) {
                SEARCH_OBJECT();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_USE]) {
                USE_ITEM();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_FIRE_UP]) {
                FIRE_UP();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_FIRE_LEFT]) {
                FIRE_LEFT();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_FIRE_DOWN]) {
                FIRE_DOWN();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_FIRE_RIGHT]) {
                FIRE_RIGHT();
                KEYTIMER = 20;
            } else if (A == KEY_CONFIG[KEY_PAUSE]) { // RUN/STOP
                done = PAUSE_GAME();
            } else if (A == KEY_CONFIG[KEY_CHEAT]) { // SHIFT-C
                CHEATER();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_MUSIC]) { // SHIFT-M
                TOGGLE_MUSIC();
                CLEAR_KEY_BUFFER();
            }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            else if (A == KEY_CONFIG[KEY_LIVE_MAP]) {
                TOGGLE_LIVE_MAP();
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_LIVE_MAP_ROBOTS]) {
                TOGGLE_LIVE_MAP_ROBOTS();
                CLEAR_KEY_BUFFER();
            }
#endif
        }
        // SNES CONTROLLER starts here
        if (B != 0) {
#ifdef INACTIVITY_TIMEOUT_GAME
            INACTIVE_SECONDS = 0;
#endif
            // first we start with the 4 directional buttons.
            if ((CONTROL >= 2 && (B & Platform::JoystickPlay) == 0) ||
                (CONTROL < 2 && (B & Platform::JoystickBlue) == 0)) {
                if (CONTROL < 2 && B & Platform::JoystickRed) {
                    if (B & Platform::JoystickLeft) {
                        FIRE_LEFT();
                        KEYTIMER = 20;
                    } else if (B & Platform::JoystickRight) {
                        FIRE_RIGHT();
                        KEYTIMER = 20;
                    } else if (B & Platform::JoystickUp) {
                        FIRE_UP();
                        KEYTIMER = 20;
                    } else if (B & Platform::JoystickDown) {
                        FIRE_DOWN();
                        KEYTIMER = 20;
                    }
                } else {
                    if (B & Platform::JoystickLeft) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_LEFT();
                        AFTER_MOVE_SNES();
                    } else if (B & Platform::JoystickRight) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_RIGHT();
                        AFTER_MOVE_SNES();
                    } else if (B & Platform::JoystickUp) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_UP();
                        AFTER_MOVE_SNES();
                    } else if (B & Platform::JoystickDown) {
                        UNIT = 0;
                        MOVE_TYPE = 1; // %00000001
                        REQUEST_WALK_DOWN();
                        AFTER_MOVE_SNES();
                    }
                }
            }
            // Now check for non-repeating buttons
            switch (CONTROL) {
            case 3:
                if (B & Platform::JoystickPlay) {
                    if (B & Platform::JoystickLeft) {
                        FIRE_LEFT();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickRight) {
                        FIRE_RIGHT();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickUp) {
                        FIRE_UP();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickDown) {
                        FIRE_DOWN();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickExtra) {
                        TOGGLE_MUSIC();
                        CLEAR_KEY_BUFFER();
                    }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                    if (B & Platform::JoystickYellow) {
                        TOGGLE_LIVE_MAP_ROBOTS();
                        CLEAR_KEY_BUFFER();
                    }
#endif
                } else {
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                    if (B & Platform::JoystickYellow) {
                        TOGGLE_LIVE_MAP();
                        CLEAR_KEY_BUFFER();
                    }
#endif
                    if (B & Platform::JoystickGreen) {
                        CYCLE_ITEM();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickBlue) {
                        CYCLE_WEAPON();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickRed) {
                        USE_ITEM();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickReverse) {
                        SEARCH_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickForward) {
                        MOVE_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickExtra) {
                        done = PAUSE_GAME();
                    }
                }
                break;
            case 2:
                if (B & Platform::JoystickPlay) {
                    if (B & Platform::JoystickReverse) {
                        CYCLE_ITEM();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickForward) {
                        CYCLE_WEAPON();
                        KEYTIMER = 15;
                    }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                    if (B & Platform::JoystickLeft) {
                        TOGGLE_LIVE_MAP();
                        CLEAR_KEY_BUFFER();
                    }
                    if (B & Platform::JoystickDown) {
                        TOGGLE_LIVE_MAP_ROBOTS();
                        CLEAR_KEY_BUFFER();
                    }
#endif
                    if (B & Platform::JoystickBlue) {
                        done = PAUSE_GAME();
                    }
                    if (B & Platform::JoystickRed) {
                        TOGGLE_MUSIC();
                        CLEAR_KEY_BUFFER();
                    }
#ifdef GAMEPAD_CD32
                    if (B == Platform::JoystickPlay) {
                        USE_ITEM();
                        KEYTIMER = 15;
                    }
#endif
                } else {
                    if (B & Platform::JoystickGreen) {
                        FIRE_LEFT();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickBlue) {
                        FIRE_RIGHT();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickYellow) {
                        FIRE_UP();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickRed) {
                        FIRE_DOWN();
                        KEYTIMER = 20;
                    }
                    if (B & Platform::JoystickReverse) {
                        SEARCH_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickForward) {
                        MOVE_OBJECT();
                        KEYTIMER = 15;
                    }
#ifndef GAMEPAD_CD32
                    if (B == Platform::JoystickExtra) {
                        USE_ITEM();
                        KEYTIMER = 15;
                    }
#endif
                }
                break;
            default:
                if (B & Platform::JoystickBlue) {
                    if (B & Platform::JoystickLeft) {
                        CYCLE_ITEM();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickRight) {
                        CYCLE_WEAPON();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickUp) {
                        MOVE_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B & Platform::JoystickDown) {
                        SEARCH_OBJECT();
                        KEYTIMER = 15;
                    }
                    if (B == Platform::JoystickBlue) {
                        USE_ITEM();
                        KEYTIMER = 15;
                    }
                }
                break;
            }
        }
    }
}

// This routine handles things that are in common to
// all 4 directions of movement.
void AFTER_MOVE_SNES()
{
    if (MOVE_RESULT == 1) {
        ANIMATE_PLAYER();
    }
    CACULATE_AND_REDRAW();
    if (KEY_FAST == 0) {
        KEYTIMER = 15;
        KEY_FAST = 1;
    } else {
        KEYTIMER = 6;
    }
}

void TOGGLE_MUSIC()
{
    if (MUSIC_ON == 1) {
        PRINT_INFO(MSG_MUSICOFF);
        MUSIC_ON = 0;
#ifdef PLATFORM_MODULE_BASED_AUDIO
        platform->playModule(Platform::ModuleSoundFX);
#else
        platform->stopNote(); // turn off sound
#endif
    } else {
        PRINT_INFO(MSG_MUSICON);
        MUSIC_ON = 1;
        START_IN_GAME_MUSIC();
    }
}

void START_IN_GAME_MUSIC()
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playModule(MUSIC_ON == 1 ? LEVEL_MUSIC[SELECTED_MAP] : Platform::ModuleSoundFX);
#else
    MUSIC_ON = 1;
    if (SOUND_EFFECT == 0xFF) { // FF=NO sound effect in progress
        DATA_LINE = 0;
        CUR_PATTERN = IN_GAME_MUSIC1 + (LEVEL_MUSIC[SELECTED_MAP] << 8);
    } else {
        // apparently a sound-effect is active, so we do things differently.
        DATA_LINE_TEMP = 0;
        PATTERN_TEMP = IN_GAME_MUSIC1 + (LEVEL_MUSIC[SELECTED_MAP] << 8);
    }
#endif
}

#ifdef PLATFORM_MODULE_BASED_AUDIO
Platform::Module LEVEL_MUSIC[] = {
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2
};
#else
uint8_t LEVEL_MUSIC[] = { 0,1,2,0,1,2,0,1,2,0,1,2,0,1 };
#endif

// TEMP ROUTINE TO GIVE ME ALL ITEMS AND WEAPONS
void CHEATER()
{
    PLAY_SOUND(12);
    KEYS = 7;
    AMMO_PISTOL = 100;
    AMMO_PLASMA = 100;
    INV_BOMBS = 100;
    INV_EMP = 100;
    INV_MEDKIT = 100;
    INV_MAGNET = 100;
    SELECTED_WEAPON = 1;
    SELECTED_ITEM = 1;
#ifdef PLATFORM_IMAGE_SUPPORT
    REDRAW_WINDOW = 1;
#endif
    DISPLAY_KEYS();
    DISPLAY_WEAPON();
    DISPLAY_ITEM();
}

bool PAUSE_GAME()
{
    PLAY_SOUND(15);
    // pause clock
    CLOCK_ACTIVE = 0;
    // display message to user
    SCROLL_INFO();
    PRINT_INFO(MSG_PAUSED);
//    for (BGTIMER1 = 0; BGTIMER1 != 1;); // to prevent double-tap of run/stop
    CLEAR_KEY_BUFFER();
    platform->renderFrame();
    while (!platform->quit) {
        uint8_t A = platform->readKeyboard();
        uint16_t B = platform->readJoystick(CONTROL >= 2 ? true : false);
        if (A == KEY_CONFIG[KEY_PAUSE] || // RUN/STOP
            A == KEY_CONFIG[KEY_NO] ||
            (B & Platform::JoystickBlue)) { // N-KEY
            SCROLL_INFO();
            SCROLL_INFO();
            SCROLL_INFO();
            CLEAR_KEY_BUFFER();
            CLOCK_ACTIVE = 1;
            PLAY_SOUND(15);
            return false;
        } else if (A == KEY_CONFIG[KEY_YES] || (B & Platform::JoystickRed)) { // Y-KEY
            UNIT_TYPE[0] = 0; // make player dead
            PLAY_SOUND(15);
            GOM4();
            return true;
        }
        platform->renderFrame(true);
    }
    return false;
}

void CLEAR_KEY_BUFFER()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    KEYTIMER = 20;
}

void USE_ITEM()
{
    // check select timeout to prevent accidental double-tap
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    // First figure out which item to use.
    switch (SELECTED_ITEM) {
    case 1: // BOMB
        USE_BOMB();
        break;
    case 2: // EMP
        USE_EMP();
        break;
    case 3: // MEDKIT
        USE_MEDKIT();
        break;
    case 4: // MAGNET
        USE_MAGNET();
        break;
    default:
        break;
    }
}

void USE_BOMB()
{
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    platform->setCursorShape(Platform::ShapeUse);
#endif
    USER_SELECT_OBJECT();
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    if (BOMB_MAGNET_COMMON1()) {
        // Now scan for any units at that location:
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) { // 255 means no unit found.
            for (int X = 28; X != 32; X++) { // Start of weapons units
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 6; // bomb AI
                    UNIT_TILE[X] = 130; // bomb tile
                    UNIT_LOC_X[X] = MAP_X;
                    UNIT_LOC_Y[X] = MAP_Y;
                    UNIT_TIMER_A[X] = 100; // How long until explosion?
                    UNIT_A[X] = 0;
                    INV_BOMBS--;
                    DISPLAY_ITEM();
                    REDRAW_WINDOW = 1;
                    SELECT_TIMEOUT = 3; // 3 cycles before next item can be used, pet version only
                    PLAY_SOUND(6); // SOUND PLAY
                    return;
                }
            }
            return; // no slots available right now, abort.
        }
    }
    BOMB_MAGNET_COMMON2();
}

void USE_MAGNET()
{
    if (MAGNET_ACT != 0) { // only one magnet active at a time.
        return;
    }
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    platform->setCursorShape(Platform::ShapeUse);
#endif
    USER_SELECT_OBJECT();
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    if (BOMB_MAGNET_COMMON1()) {
        for (int X = 28; X != 32; X++) { // Start of weapons units
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 20; // MAGNET AI
                UNIT_TILE[X] = 134; // MAGNET tile
                UNIT_LOC_X[X] = MAP_X;
                UNIT_LOC_Y[X] = MAP_Y;
                UNIT_TIMER_A[X] = 1; // How long until ACTIVATION
                UNIT_TIMER_B[X] = 255; // how long does it live -A
                UNIT_A[X] = 3; // how long does it live -B
                MAGNET_ACT = 1; // only one magnet allowed at a time.
                INV_MAGNET--;
                DISPLAY_ITEM();
                REDRAW_WINDOW = 1;
                PLAY_SOUND(6); // move sound, SOUND PLAY
                return;
            }
        }
        return; // no slots available right now, abort.
    }
    BOMB_MAGNET_COMMON2();
}

bool BOMB_MAGNET_COMMON1()
{
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->hideCursor();
#else
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MOVTEMP_UX = MAP_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
    MOVTEMP_UY = MAP_Y;
    GET_TILE_FROM_MAP();
    return (TILE_ATTRIB[TILE] & 0x01) == 0x01; // %00000001 is that spot available for something to move onto it?
}

void BOMB_MAGNET_COMMON2()
{
    PRINT_INFO(MSG_BLOCKED);
    PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
}

void USE_EMP()
{
    EMP_FLASH();
//    REDRAW_WINDOW = 0;  // attempt to delay window redrawing (pet only)
    PLAY_SOUND(3);  // EMP sound, SOUND PLAY
    INV_EMP--;
    DISPLAY_ITEM();
    for (int X = 1; X != 28; X++) { // start with unit#1 (skip player)
        if (UNIT_TYPE[X] != 0 &&                    // CHECK THAT UNIT EXISTS
            UNIT_LOC_X[X] >= MAP_WINDOW_X &&        // CHECK HORIZONTAL POSITION
            UNIT_LOC_X[X] <= (MAP_WINDOW_X + PLATFORM_MAP_WINDOW_TILES_WIDTH - 1) &&  // NOW CHECK VERTICAL
            UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
            UNIT_LOC_Y[X] <= (MAP_WINDOW_Y + PLATFORM_MAP_WINDOW_TILES_HEIGHT - 1)) {
            UNIT_TIMER_A[X] = 255;
            // test to see if unit is above water
            MAP_X = UNIT_LOC_X[X];
            MAP_Y = UNIT_LOC_Y[X];
            GET_TILE_FROM_MAP();
            if (TILE == 204) {  // WATER
                UNIT_TYPE[X] = 5;
                UNIT_TIMER_A[X] = 5;
                UNIT_TIMER_B[X] = 3;
                UNIT_A[X] = 60; // how long to show sparks.
                UNIT_TILE[X] = 140; // Electrocuting tile
            }
        }
    }
    PRINT_INFO(MSG_EMPUSED);
    SELECT_TIMEOUT = 3; // 3 cycles before next item can be used
}

void USE_MEDKIT()
{
    if (UNIT_HEALTH[0] == 12) {    // Do we even need the medkit?
        return;
    }
    // Now figure out how many HP we need to be healthy.
    TEMP_A = 12 - UNIT_HEALTH[0];  // how many we need.
    if (INV_MEDKIT >= TEMP_A) {
        // we had more than we need, so go to full health.
        UNIT_HEALTH[0] = 12;
        INV_MEDKIT -= TEMP_A;
    } else {
        // we had less than we need, so we'll use what is available.
        UNIT_HEALTH[0] += INV_MEDKIT;
        INV_MEDKIT = 0;
    }
    DISPLAY_PLAYER_HEALTH();
    DISPLAY_ITEM();
    PLAY_SOUND(2); // MEDKIT SOUND, SOUND PLAY
    PRINT_INFO(MSG_MUCHBET);
}

void FIRE_UP()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_UP_PISTOL();
    } else {
        FIRE_UP_PLASMA();
    }
    UNIT_DIRECTION[0] = 0;
}

void FIRE_UP_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 12; // Fire pistol up AI routine
            UNIT_TILE[X] = 244; // tile for vertical weapons fire
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_UP_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 12; // Fire pistol up AI routine
            UNIT_TILE[X] = 240; // tile for vertical plasma bolt
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_DOWN()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_DOWN_PISTOL();
    } else {
        FIRE_DOWN_PLASMA();
    }
    UNIT_DIRECTION[0] = 1;
}

void FIRE_DOWN_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 13; // Fire pistol DOWN AI routine
            UNIT_TILE[X] = 244; // tile for vertical weapons fire
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_DOWN_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 13; // Fire pistol DOWN AI routine
            UNIT_TILE[X] = 240; // tile for vertical plasma bolt
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_LEFT()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_LEFT_PISTOL();
    } else {
        FIRE_LEFT_PLASMA();
    }
    UNIT_DIRECTION[0] = 2;
}

void FIRE_LEFT_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 14; // Fire pistol LEFT AI routine
            UNIT_TILE[X] = 245; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_LEFT_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 14; // Fire pistol LEFT AI routine
            UNIT_TILE[X] = 241; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_RIGHT()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_RIGHT_PISTOL();
    } else {
        FIRE_RIGHT_PLASMA();
    }
    UNIT_DIRECTION[0] = 3;
}

void FIRE_RIGHT_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 15; // Fire pistol RIGHT AI routine
            UNIT_TILE[X] = 245; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_RIGHT_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 15; // Fire pistol RIGHT AI routine
            UNIT_TILE[X] = 241; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void AFTER_FIRE(int X)
{
    UNIT_TIMER_A[X] = 0;
    UNIT_LOC_X[X] = UNIT_LOC_X[0];
    UNIT_LOC_Y[X] = UNIT_LOC_Y[0];
    UNIT = X;
    if (SELECTED_WEAPON != 2) {
        PLAY_SOUND(9); // PISTOL-SOUND, SOUND PLAY
        AMMO_PISTOL--;
        DISPLAY_WEAPON();
    } else {
        PLAY_SOUND(8); // PLASMA-GUN-SOUND, SOUND PLAY
        AMMO_PLASMA--;
        DISPLAY_WEAPON();
    }
}

// This routine checks KEYTIMER to see if it has
// reached zero yet.  If so, it clears the LSTX
// variable used by the kernal, so that it will
// register a new keypress.
void KEY_REPEAT(bool keyDown)
{
    if (KEYTIMER != 0) {
        return;
    }
    if (keyDown) { // no key pressed
        platform->keyRepeat(); // clear LSTX register
        KEYTIMER = 6;
    } else {
        // No key pressed, reset all to defaults
        KEY_FAST = 0;
        KEYTIMER = 6;
    }
}

// This routine handles things that are in common to
// all 4 directions of movement.
void AFTER_MOVE()
{
    if (MOVE_RESULT == 1) {
        ANIMATE_PLAYER();
    }
    CACULATE_AND_REDRAW();
    // now reset key-repeat rate
    if (KEY_FAST == 0) {
        // FIRST REPEAT
        KEYTIMER = 13;
        KEY_FAST++;
    } else {
        // SUBSEQUENT REPEATS
        KEYTIMER = 6;
    }
}

uint8_t KEY_FAST = 0; // 0=DEFAULT STATE

// This routine is invoked when the user presses S to search
// an object such as a crate, chair, or plant.
void SEARCH_OBJECT()
{
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    platform->setCursorShape(Platform::ShapeSearch);
#endif
    USER_SELECT_OBJECT();
    REDRAW_WINDOW = 1;
    // first check of object is searchable
    CALC_COORDINATES();
    GET_TILE_FROM_MAP();
    if ((TILE_ATTRIB[TILE] & 0x40) == 0) { // %01000000 can search attribute
#ifdef PLATFORM_CURSOR_SUPPORT
        platform->hideCursor();
#else
        CURSOR_ON = 0;
#endif
    } else {
        // is the tile a crate?
        if (TILE == 41 || TILE == 45 || TILE == 199) { // BIG CRATE / small CRATE / "Pi" CRATE
            TILE = DESTRUCT_PATH[TILE];
            PLOT_TILE_TO_MAP();
        }
        // Now check if there is an object there.
        PRINT_INFO(MSG_SEARCHING);
        for (SEARCHBAR = 0; SEARCHBAR != 8; SEARCHBAR++) {
            for (BGTIMER2 = 18; BGTIMER2 != 0;) { // delay time between search periods
                if (BGTIMER1 != 1) {
                    platform->renderFrame(true);
                }
                PET_SCREEN_SHAKE();
                BACKGROUND_TASKS();
            }
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 9 + SEARCHBAR, 46); // PERIOD
        }
#ifdef PLATFORM_CURSOR_SUPPORT
        platform->hideCursor();
#else
        CURSOR_ON = 0;
        DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
        CALC_COORDINATES();
        CHECK_FOR_HIDDEN_UNIT();
        if (UNIT_FIND == 255) {
            PRINT_INFO(MSG_NOTFOUND);
            return;
        }
        TEMP_A = UNIT_TYPE[UNIT_FIND];  // store object type
        TEMP_B = UNIT_A[UNIT_FIND]; // store secondary info
        UNIT_TYPE[UNIT_FIND] = 0; // DELETE ITEM ONCE FOUND
        // ***NOW PROCESS THE ITEM FOUND***
        PLAY_SOUND(10); // ITEM-FOUND-SOUND, SOUND PLAY
        if (TEMP_A == 128) {    // key
            if (TEMP_B == 0) {  // WHICH SORT OF KEY?
                KEYS |= 0x01; // %00000001 Add spade key
            } else if (TEMP_B == 1) {
                KEYS |= 0x02; // %00000010 Add heart key
            } else {
                KEYS |= 0x04; // %00000100 Add star key
            }
            PRINT_INFO(MSG_FOUNDKEY);
            DISPLAY_KEYS();
        } else if (TEMP_A == 129) { // TIME BOMB
            INV_BOMBS += TEMP_B;
            PRINT_INFO(MSG_FOUNDBOMB);
            DISPLAY_ITEM();
        } else if (TEMP_A == 130) { // EMP
            INV_EMP += TEMP_B;
            PRINT_INFO(MSG_FOUNDEMP);
            DISPLAY_ITEM();
        } else if (TEMP_A == 131) { // PISTOL
            if (AMMO_PISTOL + TEMP_B > 255) { // If we rolled over past 255
                AMMO_PISTOL = 255; // set it to 255.
            } else {
                AMMO_PISTOL += TEMP_B;
            }
            PRINT_INFO(MSG_FOUNDGUN);
            DISPLAY_WEAPON();
        } else if (TEMP_A == 132) { // PLASMA GUN
            AMMO_PLASMA += TEMP_B;
            PRINT_INFO(MSG_FOUNDPLAS);
            DISPLAY_WEAPON();
        } else if (TEMP_A == 133) { // MEDKIT
            INV_MEDKIT += TEMP_B;
            PRINT_INFO(MSG_FOUNDMED);
            DISPLAY_ITEM();
        } else if (TEMP_A == 134) { // MAGNET
            INV_MAGNET += TEMP_B;
            PRINT_INFO(MSG_FOUNDMAG);
            DISPLAY_ITEM();
        }
        // ADD CODE HERE FOR OTHER OBJECT TYPES
    }
}

uint8_t SEARCHBAR = 0; // to count how many periods to display.

// combines cursor location with window location
// to determine coordinates for MAP_X and MAP_Y
void CALC_COORDINATES()
{
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
}

// This routine is called by routines such as the move, search,
// or use commands.  It displays a cursor and allows the user
// to pick a direction of an object.
void USER_SELECT_OBJECT()
{
    PLAY_SOUND(16); // beep sound, SOUND PLAY
    CURSOR_X = UNIT_LOC_X[0] - MAP_WINDOW_X;
    CURSOR_Y = UNIT_LOC_Y[0] - MAP_WINDOW_Y;
#ifdef PLATFORM_CURSOR_SUPPORT
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 0) {
#endif
        platform->showCursor(CURSOR_X, CURSOR_Y);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    }
#endif
#else
    CURSOR_ON = 1;
    REVERSE_TILE();
#endif
    // First ask user which object to move
    while (!platform->quit) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die while moving something?
#ifdef PLATFORM_CURSOR_SUPPORT
            platform->hideCursor();
#else
            CURSOR_ON = 0;
#endif
            return;
        }
        uint8_t A = platform->readKeyboard();
        // SNES controls for this routine
        uint16_t B = platform->readJoystick(CONTROL >= 2 ? true : false);
        if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT] || (B & Platform::JoystickRight)) { // CURSOR RIGHT
            UNIT_DIRECTION[0] = 3;
            CURSOR_X++;
#ifdef PLATFORM_CURSOR_SUPPORT
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            if (LIVE_MAP_ON == 0) {
#endif
                platform->showCursor(CURSOR_X, CURSOR_Y);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            }
#endif
#endif
            return;
        } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT] || (B & Platform::JoystickLeft)) { // CURSOR LEFT
            UNIT_DIRECTION[0] = 2;
            CURSOR_X--;
#ifdef PLATFORM_CURSOR_SUPPORT
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            if (LIVE_MAP_ON == 0) {
#endif
                platform->showCursor(CURSOR_X, CURSOR_Y);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            }
#endif
#endif
            return;
        } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
            UNIT_DIRECTION[0] = 1;
            CURSOR_Y++;
#ifdef PLATFORM_CURSOR_SUPPORT
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            if (LIVE_MAP_ON == 0) {
#endif
                platform->showCursor(CURSOR_X, CURSOR_Y);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            }
#endif
#endif
            return;
        } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP] || (B & Platform::JoystickUp)) { // CURSOR UP
            UNIT_DIRECTION[0] = 0;
            CURSOR_Y--;
#ifdef PLATFORM_CURSOR_SUPPORT
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            if (LIVE_MAP_ON == 0) {
#endif
                platform->showCursor(CURSOR_X, CURSOR_Y);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            }
#endif
#endif
            return;
        }
    }
}

void MOVE_OBJECT()
{
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    platform->setCursorShape(Platform::ShapeMove);
#endif
    USER_SELECT_OBJECT();
    // now test that object to see if it
    // is allowed to be moved.
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->hideCursor();
#else
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
    CALC_COORDINATES();
    CHECK_FOR_HIDDEN_UNIT();
    MOVTEMP_U = UNIT_FIND;
    GET_TILE_FROM_MAP();
    if ((TILE_ATTRIB[TILE] & 0x04) == 0) { // can it be moved?
        PRINT_INFO(MSG_CANTMOVE);
        PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
        return;
    }
    MOVTEMP_O = TILE; // Store which tile it is we are moving
    MOVTEMP_X = MAP_X; // Store original location of object
    MOVTEMP_Y = MAP_Y;
#ifdef PLATFORM_CURSOR_SUPPORT
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 0) {
#endif
        platform->showCursor(CURSOR_X, CURSOR_Y);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    }
#endif
#else
    CURSOR_ON = 1;
    REVERSE_TILE();
#endif
    // NOW ASK THE USER WHICH DIRECTION TO MOVE IT TO
    while (!platform->quit) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die while moving something?
#ifdef PLATFORM_CURSOR_SUPPORT
            platform->hideCursor();
#else
            CURSOR_ON = 0;
#endif
            return;
        }
        // keyboard control
        uint8_t A = platform->readKeyboard();
        // SNES controls
        uint16_t B = platform->readJoystick(CONTROL >= 2 ? true : false);
        if (A != 0xff || B != 0) {
            if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT] || (B & Platform::JoystickRight)) { // CURSOR RIGHT
                CURSOR_X++;
                break;
            } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT] || (B & Platform::JoystickLeft)) { // CURSOR LEFT
                CURSOR_X--;
                break;
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
                CURSOR_Y++;
                break;
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP] || (B & Platform::JoystickUp)) { // CURSOR UP
                CURSOR_Y--;
                break;
            }
        }
    }
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->hideCursor();
#else
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MOVTEMP_UX = MAP_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
    MOVTEMP_UY = MAP_Y;
    GET_TILE_FROM_MAP();
    if (TILE_ATTRIB[TILE] & 0x20) { // %00100000 is that spot available for something to move onto it?
        // Now scan for any units at that location:
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) { // 255 means no unit found.
            PLAY_SOUND(6); // move sound, SOUND PLAY
            MOVTEMP_D = MAP_SOURCE[0]; // Grab current object
            MAP_SOURCE[0] = MOVTEMP_O; // replace with object we are moving
            MAP_X = MOVTEMP_X; // RETRIEVE original location of object
            MAP_Y = MOVTEMP_Y;
            GET_TILE_FROM_MAP();
            int A = MOVTEMP_D;
            if (A == 148) { // trash compactor tile
                A = 9; // Floor tile
            }
            MAP_SOURCE[0] = A; // Replace former location
            REDRAW_WINDOW = 1; // See the result
            if (MOVTEMP_U == 255) {
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                if (LIVE_MAP_ON == 1) {
                    platform->renderLiveMapTile(MAP, MOVTEMP_X, MOVTEMP_Y);
                    platform->renderLiveMapTile(MAP, MOVTEMP_UX, MOVTEMP_UY);
                }
#endif
                return;
            }
            UNIT_LOC_X[MOVTEMP_U] = MOVTEMP_UX;
            UNIT_LOC_Y[MOVTEMP_U] = MOVTEMP_UY;
            return;
        }
    }
    PRINT_INFO(MSG_BLOCKED);
    PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
}

uint8_t MOVTEMP_O = 0; // origin tile
uint8_t MOVTEMP_D = 0; // destination tile
uint8_t MOVTEMP_X = 0; // x-coordinate
uint8_t MOVTEMP_Y = 0; // y-coordinate
uint8_t MOVTEMP_U = 0; // unit number (255=none)
uint8_t MOVTEMP_UX = 0;
uint8_t MOVTEMP_UY = 0;

void CACULATE_AND_REDRAW()
{
    MAP_WINDOW_X = MIN(MAX(UNIT_LOC_X[0] - PLATFORM_MAP_WINDOW_TILES_WIDTH / 2, 0), 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH); // no index needed since it's player unit
    MAP_WINDOW_Y = MIN(MAX(UNIT_LOC_Y[0] - PLATFORM_MAP_WINDOW_TILES_HEIGHT / 2, 0), 64 - PLATFORM_MAP_WINDOW_TILES_HEIGHT); // no index needed since it's player unit
    REDRAW_WINDOW = 1;
}

// This routine checks all units from 0 to 31 and figures out if it should be displayed
// on screen, and then grabs that unit's tile and stores it in the MAP_PRECALC array
// so that when the window is drawn, it does not have to search for units during the
// draw, speeding up the display routine.
void MAP_PRE_CALCULATE()
{
    // CLEAR OLD BUFFER
    for (int Y = 0; Y != MAP_WINDOW_SIZE; Y++) {
        MAP_PRECALC[Y] = 0;
    }
    for (int X = 0; X != 32; X++) {
        if (X == 0 || // skip the check for unit zero, always draw it.
            (UNIT_TYPE[X] != 0 &&                    // CHECK THAT UNIT EXISTS
             UNIT_LOC_X[X] >= MAP_WINDOW_X &&        // CHECK HORIZONTAL POSITION
             UNIT_LOC_X[X] <= (MAP_WINDOW_X + PLATFORM_MAP_WINDOW_TILES_WIDTH - 1) && // NOW CHECK VERTICAL
             UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
             UNIT_LOC_Y[X] <= (MAP_WINDOW_Y + PLATFORM_MAP_WINDOW_TILES_HEIGHT - 1))) {
            // Unit found in map window, now add that unit's
            // tile to the precalc map.
            int Y = UNIT_LOC_X[X] - MAP_WINDOW_X + (UNIT_LOC_Y[X] - MAP_WINDOW_Y) * PLATFORM_MAP_WINDOW_TILES_WIDTH;
            if (UNIT_TILE[X] == 130 || UNIT_TILE[X] == 134) { // is it a bomb, is it a magnet?
                // What to do in case of bomb or magnet that should
                // go underneath the unit or robot.
                if (MAP_PRECALC[Y] != 0) {
                    continue;
                }
            }
            MAP_PRECALC[Y] = UNIT_TILE[X];
            MAP_PRECALC_DIRECTION[Y] = UNIT_DIRECTION[X];
            MAP_PRECALC_TYPE[Y] = UNIT_A[X];
        }
        // continue search
    }
}

// This routine is where the MAP is displayed on the screen
#ifdef OPTIMIZED_MAP_RENDERING
void INVALIDATE_PREVIOUS_MAP()
{
    for (int i = 0; i < MAP_WINDOW_SIZE; i++) {
        PREVIOUS_MAP_BACKGROUND[i] = 255;
    }
}

void DRAW_MAP_WINDOW()
{
    MAP_PRE_CALCULATE();
    REDRAW_WINDOW = 0;
    MAP_SOURCE = MAP + ((MAP_WINDOW_Y << 7) + MAP_WINDOW_X);
    for (uint16_t TEMP_Y = 0, PRECALC_COUNT = 0; TEMP_Y != PLATFORM_MAP_WINDOW_TILES_HEIGHT; TEMP_Y++, MAP_SOURCE += 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH, PRECALC_COUNT += PLATFORM_MAP_WINDOW_TILES_WIDTH - PLATFORM_MAP_WINDOW_TILES_WIDTH) {
        for (uint16_t TEMP_X = 0; TEMP_X != PLATFORM_MAP_WINDOW_TILES_WIDTH; TEMP_X++, MAP_SOURCE++, PRECALC_COUNT++) {
            // NOW FIGURE OUT WHERE TO PLACE IT ON SCREEN.
            TILE = MAP_SOURCE[0];
            uint8_t VARIANT = 0;
#ifdef PLATFORM_IMAGE_BASED_TILES
            switch (TILE) {
            case 204: // WATER
            case 66:  // FLAG
            case 148: // TRASH COMPACTOR
            case 143: // SERVER
                VARIANT = ANIM_STATE & 3;
                break;
            case 196: // HVAC
            case 197:
            case 200:
            case 201:
                VARIANT = ANIM_STATE & 1;
                break;
            case 20: // CINEMA
            case 21:
            case 22:
                VARIANT = ANIM_STATE;
                break;
            default:
                break;
            }
#endif
            uint8_t FG_TILE = MAP_PRECALC[PRECALC_COUNT];
            uint8_t FG_VARIANT = 0;
#ifdef PLATFORM_SPRITE_SUPPORT
            if (FG_TILE != 0) {
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
                if (FG_TILE == 96 || (FG_TILE >= 100 && FG_TILE <= 103)) { // PLAYER OR EVILBOT
                    if (DIRECTION == 0) {
                        FG_VARIANT = 8;
                    } else if (DIRECTION == 2) {
                        FG_VARIANT = 12;
                    } else if (DIRECTION == 3) {
                        FG_VARIANT = 4;
                    }
                    if (FG_TILE == 96) {
                        FG_VARIANT += WALK_FRAME + (SELECTED_WEAPON << 4);
                    }
                } else if (FG_TILE == 243) { // TRANSPORT
                    if (DEMATERIALIZE_FRAME < 7) {
                        FG_VARIANT = DEMATERIALIZE_FRAME;
                    } else {
                        FG_TILE = 0;
                    }
                } else if (FG_TILE == 115) { // DEAD ROBOT
                    switch (MAP_PRECALC_TYPE[PRECALC_COUNT]) {
                    case 17:
                    case 18:
                        FG_VARIANT = 1;
                        break;
                    case 9:
                        FG_VARIANT = 2;
                        break;
                    default:
                        break;
                    }
                }
            }
#endif
            if (TILE != PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] ||
                VARIANT != PREVIOUS_MAP_BACKGROUND_VARIANT[PRECALC_COUNT] ||
                FG_TILE != PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] ||
                FG_VARIANT != PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT]) {
                if (FG_TILE != 0) {
                    platform->renderTiles(TILE, FG_TILE, TEMP_X * 24, TEMP_Y * 24, VARIANT, FG_VARIANT);
                    PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] = TILE;
                    PREVIOUS_MAP_BACKGROUND_VARIANT[PRECALC_COUNT] = VARIANT;
                    PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] = FG_TILE;
                    PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT] = FG_VARIANT;
                } else {
                    platform->renderTile(TILE, TEMP_X * 24, TEMP_Y * 24, VARIANT);
                    PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] = TILE;
                    PREVIOUS_MAP_BACKGROUND_VARIANT[PRECALC_COUNT] = VARIANT;
                    PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] = FG_TILE;
                    PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT] = FG_VARIANT;

                    switch (TILE) {
                    case 20: {
                        platform->waitForScreenMemoryAccess();
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 1, CINEMA_MESSAGE[CINEMA_STATE], 1, 0);
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 2, CINEMA_MESSAGE[CINEMA_STATE + 1], 1, 0);
                        break;
                    }
                    case 21: {
                        platform->waitForScreenMemoryAccess();
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 0, CINEMA_MESSAGE[CINEMA_STATE + 2], 1, 0);
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 1, CINEMA_MESSAGE[CINEMA_STATE + 3], 1, 0);
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 2, CINEMA_MESSAGE[CINEMA_STATE + 4], 1, 0);
                        break;
                    }
                    case 22: {
                        platform->waitForScreenMemoryAccess();
                        platform->writeToScreenMemory(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X + SCREEN_WIDTH_IN_CHARACTERS + 0, CINEMA_MESSAGE[CINEMA_STATE + 5], 1, 0);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }
}
#else
// This is a temporary routine, taken from the map editor.
void DRAW_MAP_WINDOW()
{
    MAP_PRE_CALCULATE();
    REDRAW_WINDOW = 0;
    for (uint8_t TEMP_Y = 0, PRECALC_COUNT = 0; TEMP_Y != PLATFORM_MAP_WINDOW_TILES_HEIGHT; TEMP_Y++) {
        for (uint8_t TEMP_X = 0; TEMP_X != PLATFORM_MAP_WINDOW_TILES_WIDTH; TEMP_X++) {
            // FIRST CALCULATE WHERE THE BYTE IS STORED IN THE MAP
            MAP_SOURCE = MAP + (((MAP_WINDOW_Y + TEMP_Y) << 7) + TEMP_X + MAP_WINDOW_X);
            TILE = MAP_SOURCE[0];
            // NOW FIGURE OUT WHERE TO PLACE IT ON SCREEN.
#ifdef PLATFORM_SPRITE_SUPPORT
            if (MAP_PRECALC[PRECALC_COUNT] != 0) {
                uint8_t FG_TILE = MAP_PRECALC[PRECALC_COUNT];
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
                uint8_t variant = 0;
                if (FG_TILE == 96 || (FG_TILE >= 100 && FG_TILE <= 103)) {
                    if (DIRECTION == 0) {
                        variant = 8;
                    } else if (DIRECTION == 2) {
                        variant = 12;
                    } else if (DIRECTION == 3) {
                        variant = 4;
                    }
                    if (FG_TILE == 96) {
                        variant += WALK_FRAME + (SELECTED_WEAPON << 4);
                    }
                }
                platform->renderTiles(TILE, FG_TILE, TEMP_X * 24, TEMP_Y * 24, 0, variant);
            } else {
                platform->renderTile(TILE, TEMP_X * 24, TEMP_Y * 24);
            }
#else
#ifdef PLATFORM_TILE_BASED_RENDERING
            PLOT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X, TEMP_X, TEMP_Y);
#else
            PLOT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X);
#endif
            // now check for sprites in this location
            if (MAP_PRECALC[PRECALC_COUNT] != 0) {
                TILE = MAP_PRECALC[PRECALC_COUNT];
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
#ifdef PLATFORM_TILE_BASED_RENDERING
                PLOT_TRANSPARENT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X, TEMP_X, TEMP_Y);
#else
                PLOT_TRANSPARENT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X);
#endif
            }
#endif
            PRECALC_COUNT++;
        }
#ifndef PLATFORM_CURSOR_SUPPORT
        // CHECK FOR CURSOR
        if (CURSOR_ON == 1) { // Is cursor even on?
            if (TEMP_Y == CURSOR_Y) { // is cursor on the same row that were drawing?
                REVERSE_TILE();
            }
        }
#endif
    }
}
#endif

#ifdef PLATFORM_LIVE_MAP_SUPPORT
void TOGGLE_LIVE_MAP()
{
    if (LIVE_MAP_ON != 1) {
        LIVE_MAP_ON = 1;

        platform->renderLiveMap(MAP);
    } else {
#ifdef PLATFORM_LIVE_MAP_SINGLE_KEY
        TOGGLE_LIVE_MAP_ROBOTS();
        if (LIVE_MAP_ROBOTS_ON == 0)
#endif
        {
            LIVE_MAP_ON = 0;

#ifdef OPTIMIZED_MAP_RENDERING
            INVALIDATE_PREVIOUS_MAP();
#endif
        }
    }
    REDRAW_WINDOW = 1;
}

void TOGGLE_LIVE_MAP_ROBOTS()
{
    LIVE_MAP_ROBOTS_ON = LIVE_MAP_ROBOTS_ON == 1 ? 0 : 1;
}

void DRAW_LIVE_MAP()
{
    platform->renderLiveMapUnits(MAP, UNIT_TYPE, UNIT_LOC_X, UNIT_LOC_Y, LIVE_MAP_PLAYER_BLINK < 128 ? 1 : 0, LIVE_MAP_ROBOTS_ON == 1 ? true : false);

    LIVE_MAP_PLAYER_BLINK += 10;
}

uint8_t LIVE_MAP_ON = 0;
uint8_t LIVE_MAP_ROBOTS_ON = 0;
uint8_t LIVE_MAP_PLAYER_BLINK = 0;
#endif

#ifdef PLATFORM_TILE_BASED_RENDERING
// This routine plots a 3x3 tile from the tile database anywhere
// on screen.  But first you must define the tile number in the
// TILE variable, as well as the starting screen address must
// be defined in $FB.
void PLOT_TILE(uint16_t destination, uint16_t x, uint16_t y)
{
#ifndef PLATFORM_IMAGE_BASED_TILES
    // DRAW THE TOP 3 CHARACTERS
    SCREEN_MEMORY[destination + 0] = TILE_DATA_TL[TILE];
    SCREEN_MEMORY[destination + 1] = TILE_DATA_TM[TILE];
    SCREEN_MEMORY[destination + 2] = TILE_DATA_TR[TILE];
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    SCREEN_MEMORY[destination + 40] = TILE_DATA_ML[TILE];
    SCREEN_MEMORY[destination + 41] = TILE_DATA_MM[TILE];
    SCREEN_MEMORY[destination + 42] = TILE_DATA_MR[TILE];
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    SCREEN_MEMORY[destination + 80] = TILE_DATA_BL[TILE];
    SCREEN_MEMORY[destination + 81] = TILE_DATA_BM[TILE];
    SCREEN_MEMORY[destination + 82] = TILE_DATA_BR[TILE];
#endif
    platform->renderTile(TILE, x * 24, y * 24);
}

// This routine plots a transparent tile from the tile database
// anywhere on screen.  But first you must define the tile number
// in the TILE variable, as well as the starting screen address must
// be defined in $FB.  Also, this routine is slower than the usual
// tile routine, so is only used for sprites.  The ":" character ($3A)
// is not drawn.
void PLOT_TRANSPARENT_TILE(uint16_t destination, uint16_t x, uint16_t y)
{
#ifndef PLATFORM_IMAGE_BASED_TILES
    // DRAW THE TOP 3 CHARACTERS
    if (TILE_DATA_TL[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 0] = TILE_DATA_TL[TILE];
    }
    if (TILE_DATA_TM[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 1] = TILE_DATA_TM[TILE];
    }
    if (TILE_DATA_TR[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 2] = TILE_DATA_TR[TILE];
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    if (TILE_DATA_ML[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 40] = TILE_DATA_ML[TILE];
    }
    if (TILE_DATA_MM[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 41] = TILE_DATA_MM[TILE];
    }
    if (TILE_DATA_MR[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 42] = TILE_DATA_MR[TILE];
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    if (TILE_DATA_BL[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 80] = TILE_DATA_BL[TILE];
    }
    if (TILE_DATA_BM[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 81] = TILE_DATA_BM[TILE];
    }
    if (TILE_DATA_BR[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 82] = TILE_DATA_BR[TILE];
    }
#endif
    uint8_t variant = 0;
    if (TILE == 96 || (TILE >= 100 && TILE <= 103)) {
        if (DIRECTION == 0) {
            variant = 8;
        } else if (DIRECTION == 2) {
            variant = 12;
        } else if (DIRECTION == 3) {
            variant = 4;
        }
        if (TILE == 96) {
            variant += WALK_FRAME + (SELECTED_WEAPON << 4);
        }
    }
    platform->renderTile(TILE, x * 24, y * 24, variant, true);
}
#else
void PLOT_TILE(uint16_t destination)
{
    // DRAW THE TOP 3 CHARACTERS
    writeToScreenMemory(destination + 0, TILE_DATA_TL[TILE]);
    writeToScreenMemory(destination + 1, TILE_DATA_TM[TILE]);
    writeToScreenMemory(destination + 2, TILE_DATA_TR[TILE]);
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    writeToScreenMemory(destination + 40, TILE_DATA_ML[TILE]);
    writeToScreenMemory(destination + 41, TILE_DATA_MM[TILE]);
    writeToScreenMemory(destination + 42, TILE_DATA_MR[TILE]);
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    writeToScreenMemory(destination + 80, TILE_DATA_BL[TILE]);
    writeToScreenMemory(destination + 81, TILE_DATA_BM[TILE]);
    writeToScreenMemory(destination + 82, TILE_DATA_BR[TILE]);
}

void PLOT_TRANSPARENT_TILE(uint16_t destination)
{
    // DRAW THE TOP 3 CHARACTERS
    if (TILE_DATA_TL[TILE] != 0x3A) {
        writeToScreenMemory(destination + 0, TILE_DATA_TL[TILE]);
    }
    if (TILE_DATA_TM[TILE] != 0x3A) {
        writeToScreenMemory(destination + 1, TILE_DATA_TM[TILE]);
    }
    if (TILE_DATA_TR[TILE] != 0x3A) {
        writeToScreenMemory(destination + 2, TILE_DATA_TR[TILE]);
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    if (TILE_DATA_ML[TILE] != 0x3A) {
        writeToScreenMemory(destination + 40, TILE_DATA_ML[TILE]);
    }
    if (TILE_DATA_MM[TILE] != 0x3A) {
        writeToScreenMemory(destination + 41, TILE_DATA_MM[TILE]);
    }
    if (TILE_DATA_MR[TILE] != 0x3A) {
        writeToScreenMemory(destination + 42, TILE_DATA_MR[TILE]);
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    if (TILE_DATA_BL[TILE] != 0x3A) {
        writeToScreenMemory(destination + 80, TILE_DATA_BL[TILE]);
    }
    if (TILE_DATA_BM[TILE] != 0x3A) {
        writeToScreenMemory(destination + 81, TILE_DATA_BM[TILE]);
    }
    if (TILE_DATA_BR[TILE] != 0x3A) {
        writeToScreenMemory(destination + 82, TILE_DATA_BR[TILE]);
    }
}
#endif

#ifndef PLATFORM_CURSOR_SUPPORT
void REVERSE_TILE()
{
    uint16_t destination = MAP_CHART[CURSOR_Y] + CURSOR_X + CURSOR_X + CURSOR_X;
    writeToScreenMemory(destination + 0, SCREEN_MEMORY[destination + 0] ^= 0x80);
    writeToScreenMemory(destination + 1, SCREEN_MEMORY[destination + 1] ^= 0x80);
    writeToScreenMemory(destination + 2, SCREEN_MEMORY[destination + 2] ^= 0x80);
    writeToScreenMemory(destination + 40, SCREEN_MEMORY[destination + 40] ^= 0x80);
    writeToScreenMemory(destination + 41, SCREEN_MEMORY[destination + 41] ^= 0x80);
    writeToScreenMemory(destination + 42, SCREEN_MEMORY[destination + 42] ^= 0x80);
    writeToScreenMemory(destination + 80, SCREEN_MEMORY[destination + 80] ^= 0x80);
    writeToScreenMemory(destination + 81, SCREEN_MEMORY[destination + 81] ^= 0x80);
    writeToScreenMemory(destination + 82, SCREEN_MEMORY[destination + 82] ^= 0x80);
}
#endif

// This routine checks to see if UNIT is occupying any space
// that is currently visible in the window.  If so, the
// flag for redrawing the window will be set.
void CHECK_FOR_WINDOW_REDRAW()
{
    if (UNIT_LOC_X[UNIT] >= MAP_WINDOW_X && // FIRST CHECK HORIZONTAL
        UNIT_LOC_X[UNIT] <= (MAP_WINDOW_X + PLATFORM_MAP_WINDOW_TILES_WIDTH - 1) &&
        UNIT_LOC_Y[UNIT] >= MAP_WINDOW_Y && // NOW CHECK VERTICAL
        UNIT_LOC_Y[UNIT] <= (MAP_WINDOW_Y + PLATFORM_MAP_WINDOW_TILES_HEIGHT - 1)) {
        REDRAW_WINDOW = 1;
    }
}

void DECWRITE(uint16_t destination, uint8_t color)
{
    for (int X = 2; X >= 0; X--) {
        writeToScreenMemory(destination + X, 0x30 + (DECNUM % 10), color);
        DECNUM /= 10;
    }
}

// The following routine loads the tileset from disk
void TILE_LOAD_ROUTINE()
{
    uint8_t* tileset = platform->loadTileset();
    DESTRUCT_PATH = tileset + 2 + 0 * 256;
    TILE_ATTRIB = tileset + 2 + 1 * 256;
#ifdef PLATFORM_SPRITE_SUPPORT
    platform->generateTiles(0, TILE_ATTRIB);
#else
    TILE_DATA_TL = tileset + 2 + 2 * 256;
    TILE_DATA_TM = tileset + 2 + 3 * 256;
    TILE_DATA_TR = tileset + 2 + 4 * 256;
    TILE_DATA_ML = tileset + 2 + 5 * 256;
    TILE_DATA_MM = tileset + 2 + 6 * 256;
    TILE_DATA_MR = tileset + 2 + 7 * 256;
    TILE_DATA_BL = tileset + 2 + 8 * 256;
    TILE_DATA_BM = tileset + 2 + 9 * 256;
    TILE_DATA_BR = tileset + 2 + 10 * 256;
    platform->generateTiles(TILE_DATA_TL, TILE_ATTRIB);
#endif
}

// The following routine loads the map from disk
void MAP_LOAD_ROUTINE()
{
    platform->loadMap((Platform::Map)SELECTED_MAP, UNIT_TYPE);
}

void DISPLAY_GAME_SCREEN()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->displayImage(Platform::ImageGame);

    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 6, 0x71, 15);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 5, 0x71, 15);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 4, 0x71, 12);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 3, 0x71, 12);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 2, 0x71, 9);
    writeToScreenMemory(25 * SCREEN_WIDTH_IN_CHARACTERS - 1, 0x71, 9);
#else
    DECOMPRESS_SCREEN(SCR_TEXT);
#endif
}

char INTRO_OPTIONS[] = "start game"
                       "select map"
                       "difficulty"
                       "controls  ";

void DISPLAY_INTRO_SCREEN()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    uint8_t* row = SCREEN_MEMORY + MENU_CHART[0];
    for (int Y = 0, i = 0; Y < PLATFORM_INTRO_OPTIONS; Y++, row += SCREEN_WIDTH_IN_CHARACTERS) {
        for (int X = 0; X < 10; X++, i++) {
            row[X] = INTRO_OPTIONS[i];
        }
    }
    platform->displayImage(Platform::ImageIntro);
#else
    DECOMPRESS_SCREEN(INTRO_TEXT);
#endif
}

void DISPLAY_ENDGAME_SCREEN()
{
    int X;
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->displayImage(Platform::ImageGameOver);
#else
    DECOMPRESS_SCREEN(SCR_ENDGAME);
#endif
    // display map name
    char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
        writeToScreenMemory(7 * SCREEN_WIDTH_IN_CHARACTERS + 22 + Y, name[Y], 4);
    }
    // display elapsed time
    DECNUM = HOURS;
    DECWRITE(9 * SCREEN_WIDTH_IN_CHARACTERS + 21, 4);
    DECNUM = MINUTES;
    DECWRITE(9 * SCREEN_WIDTH_IN_CHARACTERS + 24, 4);
    DECNUM = SECONDS;
    DECWRITE(9 * SCREEN_WIDTH_IN_CHARACTERS + 27, 4);
    writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 21, 32, 4); // SPACE
    writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 24, 58, 4); // COLON
    writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 27, 58, 4);
    // count robots remaining
    DECNUM = 0;
    for (X = 1; X != 28; X++) {
        if (UNIT_TYPE[X] != 0) {
            DECNUM++;
        }
    }
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS + 22, 4);
    // Count secrets remaining
    DECNUM = 0;
    for (X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0) {
            DECNUM++;
        }
    }
    DECWRITE(13 * SCREEN_WIDTH_IN_CHARACTERS + 22, 4);
    // display difficulty level
    char* WORD = DIFF_LEVEL_WORDS + (DIFF_LEVEL * 6);
    for (X = 0; X < 6; X++) {
        writeToScreenMemory(15 * SCREEN_WIDTH_IN_CHARACTERS + 22 + X, WORD[X], 4);
    }
}

char DIFF_LEVEL_WORDS[] = "easy  "
                          "normal"
                          "hard  "
;

void DECOMPRESS_SCREEN(uint8_t* source, uint8_t color)
{
    uint16_t destination = 0;

    while (!platform->quit) {
        if (*source != 96) { // REPEAT FLAG
            writeToScreenMemory(destination, *source, color);
        } else {
            // REPEAT CODE
            source++;
            RPT = *source;
            source++;
            for (int X = *source; X >= 0; X--) {
                writeToScreenMemory(destination++, RPT, color);
            }
            destination--;
        }
        // CHECK TO SEE IF WE REACHED $83E7 YET.
        if (destination == 0x3E7) {
            return;
        }
        source++;
        destination++;
    }
}

uint8_t RPT = 0; // repeat value

void DISPLAY_PLAYER_HEALTH()
{
    TEMP_A = UNIT_HEALTH[0] >> 1; // No index needed because it is the player, divide by two
    int Y = 0;
    while (Y != TEMP_A) {
        writeToScreenMemory(24 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y++, 0x66); // GRAY BLOCK
    }
    if (UNIT_HEALTH[0] & 0x01) {
        writeToScreenMemory(24 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y++, 0x5C); // HALF GRAY BLOCK
    }
    while (Y != 6) {
        writeToScreenMemory(24 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y++, 0x20); // SPACE
    }

#ifdef PLATFORM_IMAGE_SUPPORT
    int health = 5 - MIN(TEMP_A, 5);
    platform->renderHealth(health, PLATFORM_SCREEN_WIDTH - 48, 131 + (health >> 1));
#endif
}

void CYCLE_ITEM()
{
    PLAY_SOUND(13); // CHANGE-ITEM-SOUND, SOUND PLAY
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    SELECT_TIMEOUT = 3; // RESET THE TIMEOUT
    KEYTIMER = 20;
    SELECTED_ITEM++;
    if (SELECTED_ITEM == 5) {
        SELECTED_ITEM = 0;
    }
    DISPLAY_ITEM();
}

void DISPLAY_ITEM()
{
    PRESELECT_ITEM();
    while (SELECTED_ITEM != 0) {
        if (SELECTED_ITEM == 5) { // number too high!
            SELECTED_ITEM = 0;
            return;
        } else if (SELECTED_ITEM == 1) { // bomb
            if (INV_BOMBS != 0) { // did we run out?
                DISPLAY_TIMEBOMB();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 2) { // emp
            if (INV_EMP != 0) { // did we run out?
                DISPLAY_EMP();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 3) { // medkit
            if (INV_MEDKIT != 0) { // did we run out?
                DISPLAY_MEDKIT();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 4) { // magnet
            if (INV_MAGNET != 0) { // did we run out?
                DISPLAY_MAGNET();
                return;
            }
            SELECTED_ITEM = 0;
            PRESELECT_ITEM();
        } else {
            SELECTED_ITEM = 0;
            PRESELECT_ITEM();
        }
    }
    // no items to show
    // add routine to draw blank space
}

// This routine checks to see if currently selected
// item is zero.  And if it is, then it checks inventories
// of other items to decide which item to automatically
// select for the user.
void PRESELECT_ITEM()
{
    if (SELECTED_ITEM != 0) { // If item already selected, return
        return;
    }
    if (INV_BOMBS != 0) {
        SELECTED_ITEM = 1; // BOMB
        return;
    }
    if (INV_EMP != 0) {
        SELECTED_ITEM = 2; // EMP
        return;
    }
    if (INV_MEDKIT != 0) {
        SELECTED_ITEM = 3; // MEDKIT
        return;
    }
    if (INV_MAGNET != 0) {
        SELECTED_ITEM = 4; // MAGNET
        return;
    }
    // Nothing found in inventory at this point, so set
    // selected-item to zero.
    SELECTED_ITEM = 0; // nothing in inventory
    DISPLAY_BLANK_ITEM();
}

void DISPLAY_TIMEBOMB()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(5, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_BOMBS;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, TBOMB1A[Y]);
        writeToScreenMemory(10 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, TBOMB1B[Y]);
        writeToScreenMemory(11 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, TBOMB1C[Y]);
        writeToScreenMemory(12 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, TBOMB1D[Y]);
    }
    DECNUM = INV_BOMBS;
    DECWRITE(13 * SCREEN_WIDTH_IN_CHARACTERS - 3);
#endif
}

void DISPLAY_EMP()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(3, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_EMP;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, EMP1A[Y]);
        writeToScreenMemory(10 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, EMP1B[Y]);
        writeToScreenMemory(11 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, EMP1C[Y]);
        writeToScreenMemory(12 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, EMP1D[Y]);
    }
    DECNUM = INV_EMP;
    DECWRITE(13 * SCREEN_WIDTH_IN_CHARACTERS - 3);
#endif
}

void DISPLAY_MEDKIT()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(2, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_MEDKIT;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MED1A[Y]);
        writeToScreenMemory(10 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MED1B[Y]);
        writeToScreenMemory(11 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MED1C[Y]);
        writeToScreenMemory(12 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MED1D[Y]);
    }
    DECNUM = INV_MEDKIT;
    DECWRITE(13 * SCREEN_WIDTH_IN_CHARACTERS - 3);
#endif
}

void DISPLAY_MAGNET()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(4, PLATFORM_SCREEN_WIDTH - 48, 54);
    DECNUM = INV_MAGNET;
    DECWRITE(11 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MAG1A[Y]);
        writeToScreenMemory(10 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MAG1B[Y]);
        writeToScreenMemory(11 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MAG1C[Y]);
        writeToScreenMemory(12 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, MAG1D[Y]);
    }
    DECNUM = INV_MAGNET;
    DECWRITE(13 * SCREEN_WIDTH_IN_CHARACTERS - 3);
#endif
}

void DISPLAY_BLANK_ITEM()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 48, 48, 40);
#else
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 64, 48, 40);
    /*
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(10 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(11 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(12 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(13 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
    }
    */
#endif
}

void CYCLE_WEAPON()
{
    PLAY_SOUND(12); // CHANGE WEAPON-SOUND, SOUND PLAY
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    SELECT_TIMEOUT = 3; // RESET THE TIMEOUT
    KEYTIMER = 20;
    SELECTED_WEAPON++;
    if (SELECTED_WEAPON != 2) {
        SELECTED_WEAPON = 0;
    }
#ifdef PLATFORM_IMAGE_SUPPORT
    REDRAW_WINDOW = 1;
#endif
    DISPLAY_WEAPON();
}

void DISPLAY_WEAPON()
{
    while (!platform->quit) {
        PRESELECT_WEAPON();
        if (SELECTED_WEAPON == 0) { // no weapon to show
            // add routine to draw blank space
            return;
        } else if (SELECTED_WEAPON == 1) { // PISTOL
            if (AMMO_PISTOL != 0) { // did we run out?
                DISPLAY_PISTOL();
                return;
            }
            SELECTED_WEAPON = 0;
        } else if (SELECTED_WEAPON == 2) { // PLASMA GUN
            if (AMMO_PLASMA != 0) { // did we run out?
                DISPLAY_PLASMA_GUN();
                return;
            }
            SELECTED_WEAPON = 0;
        } else {
            SELECTED_WEAPON = 0; // should never happen
        }
    }
}

// This routine checks to see if currently selected
// weapon is zero.  And if it is, then it checks inventories
// of other weapons to decide which item to automatically
// select for the user.
void PRESELECT_WEAPON()
{
    if (SELECTED_WEAPON != 0) { // If item already selected, return
        return;
    }
    if (AMMO_PISTOL != 0) {
        SELECTED_WEAPON = 1; // PISTOL
        return;
    }
    if (AMMO_PLASMA != 0) {
        SELECTED_WEAPON = 2; // PLASMAGUN
        return;
    }
    // Nothing found in inventory at this point, so set
    // selected-item to zero.
    SELECTED_WEAPON = 0; // nothing in inventory
    DISPLAY_BLANK_WEAPON();
}

void DISPLAY_PLASMA_GUN()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(1, PLATFORM_SCREEN_WIDTH - 48, 13);
    DECNUM = AMMO_PLASMA;
    DECWRITE(5 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(2 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, WEAPON1A[Y]);
        writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, WEAPON1B[Y]);
        writeToScreenMemory(4 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, WEAPON1C[Y]);
        writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, WEAPON1D[Y]);
    }
    DECNUM = AMMO_PLASMA;
    DECWRITE(6 * SCREEN_WIDTH_IN_CHARACTERS - 3);
#endif
}

void DISPLAY_PISTOL()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(0, PLATFORM_SCREEN_WIDTH - 48, 13);
    DECNUM = AMMO_PISTOL;
    DECWRITE(5 * SCREEN_WIDTH_IN_CHARACTERS - 3, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(2 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, PISTOL1A[Y]);
        writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, PISTOL1B[Y]);
        writeToScreenMemory(4 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, PISTOL1C[Y]);
        writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, PISTOL1D[Y]);
    }
    DECNUM = AMMO_PISTOL;
    DECWRITE(6 * SCREEN_WIDTH_IN_CHARACTERS - 3);
#endif
}

void DISPLAY_BLANK_WEAPON()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 8, 48, 32);
#else
    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 16, 48, 32);
    /*
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(2 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(4 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
        writeToScreenMemory(6 * SCREEN_WIDTH_IN_CHARACTERS - 6 + Y, 32);
    }
    */
#endif
}

void DISPLAY_KEYS()
{
#ifdef PLATFORM_IMAGE_SUPPORT
//    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 106, 48, 14); // ERASE ALL 3 SPOTS
    if (KEYS & 0x01) { // %00000001 Spade key
        platform->renderKey(0, PLATFORM_SCREEN_WIDTH - 48, 106);
    }
    if (KEYS & 0x02) { // %00000010 heart key
        platform->renderKey(1, PLATFORM_SCREEN_WIDTH - 32, 106);
    }
    if (KEYS & 0x04) { // %00000100 star key
        platform->renderKey(2, PLATFORM_SCREEN_WIDTH - 16, 106);
    }
#else
//    platform->clearRect(PLATFORM_SCREEN_WIDTH - 48, 120, 48, 16); // ERASE ALL 3 SPOTS
    /*
    writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 6, 32); // ERASE ALL 3 SPOTS
    writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 5, 32);
    writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 4, 32);
    writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 3, 32);
    writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 2, 32);
    writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 1, 32);
    writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 6, 32);
    writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 5, 32);
    writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 4, 32);
    writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 3, 32);
    writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 2, 32);
    writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 1, 32);
    */
    if (KEYS & 0x01) { // %00000001 Spade key
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 6, 0x63);
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 5, 0x4D);
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 6, 0x41);
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 5, 0x67);
    }
    if (KEYS & 0x02) { // %00000010 heart key
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 4, 0x63);
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 3, 0x4D);
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 4, 0x53);
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 3, 0x67);
    }
    if (KEYS & 0x04) { // %00000100 star key
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 2, 0x63);
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS - 1, 0x4D);
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 2, 0x2A);
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS - 1, 0x67);
    }
#endif
}

void GAME_OVER()
{
    platform->renderFrame();
    // stop game clock
    CLOCK_ACTIVE = 0;
    // disable music
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->pauseModule();
#else
    MUSIC_ON = 0;
    platform->stopNote(); // turn off sound
#endif
    // Did player die or win?
    if (UNIT_TYPE[0] == 0) {
        UNIT_TILE[0] = 111; // // dead player tile
        KEYTIMER = 100;
    }
    while (KEYTIMER != 0) {
        if (BGTIMER1 != 1) {
            platform->renderFrame(true);
        }
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
    }
    for (int X = 0; X != 11; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3 - 3) / 2 * SCREEN_WIDTH_IN_CHARACTERS + (SCREEN_WIDTH_IN_CHARACTERS - 7 - 11) / 2 + X, GAMEOVER1[X]);
        writeToScreenMemory(((SCREEN_HEIGHT_IN_CHARACTERS - 3 - 3) / 2 + 1) * SCREEN_WIDTH_IN_CHARACTERS + (SCREEN_WIDTH_IN_CHARACTERS - 7 - 11) / 2 + X, GAMEOVER2[X]);
        writeToScreenMemory(((SCREEN_HEIGHT_IN_CHARACTERS - 3 - 3) / 2 + 2) * SCREEN_WIDTH_IN_CHARACTERS + (SCREEN_WIDTH_IN_CHARACTERS - 7 - 11) / 2 + X, GAMEOVER3[X]);
    }
    platform->renderFrame();
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    platform->stopModule();
    if (MUSIC_ON == 1) {
        platform->loadModule(UNIT_TYPE[0] != 0 ? Platform::ModuleWin : Platform::ModuleLose);
    }
#else
    KEYTIMER = 100;
    while (KEYTIMER != 0) {
        platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    }
#endif
#ifdef INACTIVITY_TIMEOUT_GAME
    for (int frames = 0; platform->readKeyboard() == 0xff && platform->readJoystick(CONTROL >= 2 ? true : false) == 0 && !platform->quit && frames < 5 * platform->framesPerSecond(); frames++) {
#else
    while (platform->readKeyboard() == 0xff && platform->readJoystick(CONTROL >= 2 ? true : false) == 0 && !platform->quit) {
#endif
        platform->renderFrame(true);
    }
    GOM4();
}

void GOM4()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->stopModule();
#else
    MUSIC_ON = 0;
#endif
    platform->startFadeScreen(0x000, 15);
    platform->fadeScreen(0, false);
    DISPLAY_ENDGAME_SCREEN();
    DISPLAY_WIN_LOSE();
    platform->renderFrame();
    platform->fadeScreen(15, false);
#ifdef INACTIVITY_TIMEOUT_GAME
    for (int frames = 0; platform->readKeyboard() == 0xff && platform->readJoystick(CONTROL >= 2 ? true : false) == 0 && !platform->quit && frames < 15 * platform->framesPerSecond(); frames++) {
#else
    while (platform->readKeyboard() == 0xff && platform->readJoystick(CONTROL >= 2 ? true : false) == 0 && !platform->quit) {
#endif
        platform->renderFrame(true);
    }
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->stopModule();
#endif
}

uint8_t GAMEOVER1[] = { 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x6e };
uint8_t GAMEOVER2[] = { 0x5d, 0x07, 0x01, 0x0d, 0x05, 0x20, 0x0f, 0x16, 0x05, 0x12, 0x5d };
uint8_t GAMEOVER3[] = { 0x6d, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7d };

void DISPLAY_WIN_LOSE()
{
    STOP_SONG();
    if (UNIT_TYPE[0] != 0) {
        // WIN MESSAGE
        for (int X = 0; X != 8; X++) {
            writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS + 16 + X, WIN_MSG[X], 4, 1);
        }
#ifdef PLATFORM_MODULE_BASED_AUDIO
        if (MUSIC_ON == 1) {
            platform->playModule(Platform::ModuleWin);
        }
#else
        PLAY_SOUND(18); // win music
#endif
    } else {
        // LOSE MESSAGE
        for (int X = 0; X != 9; X++) {
            writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS + 16 + X, LOS_MSG[X], 4, 1);
        }
#ifdef PLATFORM_MODULE_BASED_AUDIO
        if (MUSIC_ON == 1) {
            platform->playModule(Platform::ModuleLose);
        }
#else
        PLAY_SOUND(19); // LOSE music
#endif
    }
}

char WIN_MSG[] = "you win!";
char LOS_MSG[] = "you lose!";

void PRINT_INTRO_MESSAGE()
{
    PRINT_INFO(INTRO_MESSAGE);
}

// This routine will print something to the "information" window
// at the bottom left of the screen.  You must first define the 
// source of the text in $FB. The text should terminate with
// a null character.
void PRINT_INFO(const char* text)
{
    SCROLL_INFO(); // New text always causes a scroll
    PRINTX = 0;
    for (int Y = 0; text[Y] != 0; Y++) {
        if (text[Y] == -1) { // return
            PRINTX = 0;
            SCROLL_INFO();
        } else {
            writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + PRINTX, text[Y]);
            PRINTX++;
        }
    }
}
