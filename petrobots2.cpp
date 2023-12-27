/**
 * PETSCII Robots (Portable version)
 * by David Murray 2020
 * dfwgreencars@gmail.com
 * ported by Vesa Halttunen 2021-2022
 * vesuri@jormas.com
 */
 
#include <SAT.h>

extern SATPatHandle	thepat;
extern SpritePtr		ignoreSp;
extern Point			p;
extern SpritePtr		playerSp;
extern WindowPtr		gWind;
extern short			gVBLInstalled;
extern Rect 			r;


enum {
	scrollSizeH = 512,
	scrollSizeV = 384
};

extern Point nowOff;

#define max(x, y)	(x>y?x:y)
#define min(x, y)	(x>y?y:x)
#define abs(x)		((x)>0?(x):-(x))

#include "PlatformSAT.h"
#include "petrobots.h"

// MAP FILES CONSIST OF EVERYTHING FROM THIS POINT ON
extern uint8_t MAP_DATA[8960];
// END OF MAP FILE

extern uint8_t* DESTRUCT_PATH; // Destruct path array (256 bytes)
extern uint8_t* TILE_ATTRIB;   // Tile attrib array (256 bytes)
#ifndef PLATFORM_SPRITE_SUPPORT
extern uint8_t* TILE_DATA_TL;  // Tile character top-left (256 bytes)
extern uint8_t* TILE_DATA_TM;  // Tile character top-middle (256 bytes)
extern uint8_t* TILE_DATA_TR;  // Tile character top-right (256 bytes)
extern uint8_t* TILE_DATA_ML;  // Tile character middle-left (256 bytes)
extern uint8_t* TILE_DATA_MM;  // Tile character middle-middle (256 bytes)
extern uint8_t* TILE_DATA_MR;  // Tile character middle-right (256 bytes)
extern uint8_t* TILE_DATA_BL;  // Tile character bottom-left (256 bytes)
extern uint8_t* TILE_DATA_BM;  // Tile character bottom-middle (256 bytes)
extern uint8_t* TILE_DATA_BR;  // Tile character bottom-right (256 bytes)
#endif

// These arrays can go anywhere in RAM
extern uint8_t UNIT_TIMER_A[64];   // Primary timer for units (64 bytes)
extern uint8_t UNIT_TIMER_B[64];   // Secondary timer for units (64 bytes)
extern uint8_t UNIT_TILE[32];      // Current tile assigned to unit (32 bytes)
extern uint8_t UNIT_DIRECTION[32]; // Movement direction of unit (32 bytes)
extern uint8_t EXP_BUFFER[16];     // Explosion Buffer (16 bytes)
extern uint8_t MAP_PRECALC[MAP_WINDOW_SIZE];    // Stores pre-calculated objects for map window (77 bytes)
extern uint8_t MAP_PRECALC_DIRECTION[MAP_WINDOW_SIZE];    // Stores pre-calculated object directions for map window (77 bytes)
extern uint8_t MAP_PRECALC_TYPE[MAP_WINDOW_SIZE];    // Stores pre-calculated object types for map window (77 bytes)
#ifdef OPTIMIZED_MAP_RENDERING
extern uint8_t PREVIOUS_MAP_BACKGROUND[MAP_WINDOW_SIZE];
extern uint8_t PREVIOUS_MAP_BACKGROUND_VARIANT[MAP_WINDOW_SIZE];
extern uint8_t PREVIOUS_MAP_FOREGROUND[MAP_WINDOW_SIZE];
extern uint8_t PREVIOUS_MAP_FOREGROUND_VARIANT[MAP_WINDOW_SIZE];
#endif

// The following are the locations where the current
// key controls are stored.  These must be set before
// the game can start.
extern uint8_t KEY_CONFIG[26];
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

extern uint8_t TILE;           // The tile number to be plotted
extern uint8_t DIRECTION;      // The direction of the tile to be plotted
extern uint8_t WALK_FRAME;     // Player walking animation frame
extern uint8_t DEMATERIALIZE_FRAME; // Dematerialize animation frame
extern uint8_t MAP_X;          // Current X location on map
extern uint8_t MAP_Y;          // Current Y location on map
extern uint8_t MAP_WINDOW_X;   // Top left location of what is displayed in map window
extern uint8_t MAP_WINDOW_Y;   // Top left location of what is displayed in map window
extern uint8_t DECNUM;         // a decimal number to be displayed onscreen as 3 digits.
extern uint8_t ATTRIB;         // Tile attribute value
extern uint8_t UNIT;           // Current unit being processed
extern uint8_t TEMP_A;         // used within some routines
extern uint8_t TEMP_B;         // used within some routines
extern uint8_t TEMP_C;         // used within some routines
extern uint8_t TEMP_D;         // used within some routines
extern uint8_t CURSOR_X;       // For on-screen cursor
extern uint8_t CURSOR_Y;       // For on-screen cursor
extern uint8_t CURSOR_ON;      // Is cursor active or not? 1=yes 0=no
extern uint8_t REDRAW_WINDOW;  // 1=yes 0=no
extern uint8_t MOVE_RESULT;    // 1=Move request success, 0=fail.
extern uint8_t UNIT_FIND;      // 255=no unit present.
extern uint8_t MOVE_TYPE;      // %00000001=WALK %00000010=HOVER
extern uint8_t* CUR_PATTERN;   // stores the memory location of the current musical pattern being played.

extern uint8_t* MAP_SOURCE;    // $FD
extern uint8_t SCREEN_MEMORY[SCREEN_WIDTH_IN_CHARACTERS * SCREEN_HEIGHT_IN_CHARACTERS]; // $8000


extern uint8_t PRINTX = 0; // used to store X-cursor location

// This routine scrolls the info screen by one row, clearing
// a new row at the bottom.
void SCROLL_INFO()
{
    /*
    int X;
    for (X = 0; X != 33; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 3) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x398 + X]);
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 2) * SCREEN_WIDTH_IN_CHARACTERS + X, SCREEN_MEMORY[0x3C0 + X]); // BOTTOM ROW
    }
    // NOW CLEAR BOTTOM ROW
    for (X = 0; X != 33; X++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + X, 32); // BOTTOM ROW
    }
    */
    platform->copyRect(0, PLATFORM_SCREEN_HEIGHT - 16, 0, PLATFORM_SCREEN_HEIGHT - 24, PLATFORM_SCREEN_WIDTH - 56, 16);
    // NOW CLEAR BOTTOM ROW
    platform->clearRect(0, PLATFORM_SCREEN_HEIGHT - 8, PLATFORM_SCREEN_WIDTH - 56, 8);
    platform->renderFrame(true);
}

void RESET_KEYS_AMMO()
{
    KEYS = 0;
    AMMO_PISTOL = 0;
    AMMO_PLASMA = 0;
    INV_BOMBS = 0;
    INV_EMP = 0;
    INV_MEDKIT = 0;
    INV_MAGNET = 0;
    SELECTED_WEAPON = 0;
    SELECTED_ITEM = 0;
    MAGNET_ACT = 0;
    PLASMA_ACT = 0;
    BIG_EXP_ACT = 0;
    CYCLES = 0;
    SECONDS = 0;
    MINUTES = 0;
    HOURS = 0;
#ifdef INACTIVITY_TIMEOUT_GAME
    INACTIVE_SECONDS = 0;
#endif
}

void INTRO_SCREEN()
{
    platform->fadeScreen(0, false);
    DISPLAY_INTRO_SCREEN();
    DISPLAY_MAP_NAME();
    CHANGE_DIFFICULTY_LEVEL();
    START_INTRO_MUSIC();
    platform->show();
    platform->fadeScreen(15, false);
    MENUY = 0;
    REVERSE_MENU_OPTION(true);
    platform->renderFrame();
    bool done = false;
    while (!done && !platform->quit) {
        uint8_t A = platform->readKeyboard();
        uint16_t B = platform->readJoystick(false);
        if (A != 0xff || B != 0) {
            if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
                if (MENUY != (PLATFORM_INTRO_OPTIONS - 1)) {
                    REVERSE_MENU_OPTION(false);
                    MENUY++;
                    REVERSE_MENU_OPTION(true);
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP] || (B & Platform::JoystickUp)) { // CURSOR UP
                if (MENUY != 0) {
                    REVERSE_MENU_OPTION(false);
                    MENUY--;
                    REVERSE_MENU_OPTION(true);
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == KEY_CONFIG[KEY_SPACE] || // SPACE
                       A == KEY_CONFIG[KEY_RETURN] || (B & Platform::JoystickRed)) { // RETURN
                done = EXEC_COMMAND();
            }
#ifdef PLATFORM_MODULE_BASED_AUDIO
             else if (A == KEY_CONFIG[KEY_MUSIC]) { // SHIFT-M
                if (MUSIC_ON == 1) {
                    MUSIC_ON = 0;
                } else {
                    MUSIC_ON = 1;
                }
                START_INTRO_MUSIC();
            }
#endif
#ifdef INACTIVITY_TIMEOUT_INTRO
            else if (A == 0x7f) {
                REVERSE_MENU_OPTION(false);
                REVERSE_MENU_OPTION(true);
                DIFF_LEVEL = 0;
                SELECTED_MAP = 0;
                CHANGE_DIFFICULTY_LEVEL();
                DISPLAY_MAP_NAME();
                platform->fadeScreen(15, false);
            } else if (A == KEY_CONFIG[KEY_SEARCH] || A == KEY_CONFIG[KEY_MOVE]) {
                done = EXEC_COMMAND();
            }
#endif
        }
        platform->renderFrame(true);
    }
}

void START_INTRO_MUSIC()
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playModule(MUSIC_ON == 1 ? Platform::ModuleIntro : Platform::ModuleSoundFX);
#else
    DATA_LINE = 0;
    SOUND_EFFECT = 0xFF;
    CUR_PATTERN = INTRO_MUSIC;
    MUSIC_ON = 1;
#endif
}

bool EXEC_COMMAND()
{
    if (MENUY == 0) { // START GAME
        SET_CONTROLS();
#ifdef PLATFORM_MODULE_BASED_AUDIO
        platform->stopModule();
#else
        MUSIC_ON = 0;
#endif
        platform->stopNote(); // turn off sound
        INIT_GAME();
        return true;
    } else if (MENUY == 2) { // DIFF LEVEL
        DIFF_LEVEL++;
        if (DIFF_LEVEL == 3) {
            DIFF_LEVEL = 0;
        }
        CHANGE_DIFFICULTY_LEVEL();
        PLAY_SOUND(15); // menu beep
    } else if (MENUY == 1) { // cycle map
        PLAY_SOUND(15); // menu beep
        CYCLE_MAP();
    } else if (MENUY == 3) {
        CYCLE_CONTROLS();
        PLAY_SOUND(15); // menu beep
    }
    return false;
}

void CYCLE_CONTROLS()
{
    KEYS_DEFINED = 0;
    CONTROL++;
    if (CONTROL == 3) {
        CONTROL = 0;
    }
    // display control method on screen
    for (int X = 0, Y = CONTROLSTART[CONTROL]; X != 10; X++, Y++) {
#ifdef PLATFORM_IMAGE_SUPPORT
        writeToScreenMemory(4 * SCREEN_WIDTH_IN_CHARACTERS + 4 + X, CONTROLTEXT[Y], 14, 5);
#else
        writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 4 + X, CONTROLTEXT[Y] | 0x80);
#endif
    }
}

char CONTROLTEXT[] = "standard  "
                     "custom    "
#ifdef GAMEPAD_CD32
                     "cd32 pad  "
#else
                     "snes pad  "
#endif
                     "analog pad";
uint8_t CONTROLSTART[] = { 0, 10, 20, 30 };

void CYCLE_MAP()
{
#if PLATFORM_MAP_COUNT > 2
    SELECTED_MAP++;
#else
    SELECTED_MAP ^= 3;
#endif
    if (SELECTED_MAP == PLATFORM_MAP_COUNT) { // Maximum number of maps
        SELECTED_MAP = 0;
    }
    DISPLAY_MAP_NAME();
}

void DISPLAY_MAP_NAME()
{
    char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
#ifdef PLATFORM_IMAGE_SUPPORT
        writeToScreenMemory(7 * SCREEN_WIDTH_IN_CHARACTERS + 1 + Y, name[Y], 15, 5);
#else
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + 2 + Y, name[Y]);
#endif
    }
    // now set the mapname for the filesystem load
//    MAPNAME[6] = SELECTED_MAP + 65;
}

char* CALC_MAP_NAME()
{
    // FIND MAP NAME
    return MAP_NAMES + (SELECTED_MAP << 4); // multiply by 16 by shifting 4 times to left.
}

void REVERSE_MENU_OPTION(bool reverse)
{
#ifdef INACTIVITY_TIMEOUT_INTRO
    if (reverse) {
        platform->setHighlightedMenuRow(MENUY);
    }
#else
#ifdef PLATFORM_COLOR_SUPPORT
    for (int Y = 0; Y != 10; Y++) {
        writeToScreenMemory(MENU_CHART[MENUY] + Y, SCREEN_MEMORY[MENU_CHART[MENUY] + Y], reverse ? 14 : 15, 5);
    }
#else
    for (int Y = 0; Y != 10; Y++) {
        writeToScreenMemory(MENU_CHART[MENUY] + Y, SCREEN_MEMORY[MENU_CHART[MENUY] + Y] ^ 0x80);
    }
#endif
#endif
}

uint8_t MENUY = 0; // CURRENT MENU SELECTION
#if (PLATFORM_INTRO_OPTIONS == 4)
#ifdef PLATFORM_IMAGE_SUPPORT
menu_chart_t MENU_CHART[] = { 1 * SCREEN_WIDTH_IN_CHARACTERS + 4, 2 * SCREEN_WIDTH_IN_CHARACTERS + 4, 3 * SCREEN_WIDTH_IN_CHARACTERS + 4, 4 * SCREEN_WIDTH_IN_CHARACTERS + 4 };
#else
menu_chart_t MENU_CHART[] = { 2 * SCREEN_WIDTH_IN_CHARACTERS + 4, 3 * SCREEN_WIDTH_IN_CHARACTERS + 4, 4 * SCREEN_WIDTH_IN_CHARACTERS + 4, 5 * SCREEN_WIDTH_IN_CHARACTERS + 4 };
#endif
#else
menu_chart_t MENU_CHART[] = { 2 * SCREEN_WIDTH_IN_CHARACTERS + 4, 3 * SCREEN_WIDTH_IN_CHARACTERS + 4, 4 * SCREEN_WIDTH_IN_CHARACTERS + 4 };
#endif

void CHANGE_DIFFICULTY_LEVEL()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderFace(DIFF_LEVEL, 234, 75);
#else
    int Y = FACE_LEVEL[DIFF_LEVEL];
    // DO CHARACTERS FIRST
    writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 21, ROBOT_FACE[Y++]);
    writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 22, ROBOT_FACE[Y++]);
    writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 23, ROBOT_FACE[Y++]);
    writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 25, ROBOT_FACE[Y++]);
    writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 26, ROBOT_FACE[Y++]);
    writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + 27, ROBOT_FACE[Y++]);
    writeToScreenMemory(6 * SCREEN_WIDTH_IN_CHARACTERS + 23, ROBOT_FACE[Y++]);
    writeToScreenMemory(6 * SCREEN_WIDTH_IN_CHARACTERS + 25, ROBOT_FACE[Y]);
#endif
}

#ifdef INACTIVITY_TIMEOUT_GAME
uint8_t DIFF_LEVEL = 0; // default easy
#else
uint8_t DIFF_LEVEL = 1; // default medium
#endif
uint8_t ROBOT_FACE[] = {
    0x3A, 0x43, 0x49, 0x55, 0x43, 0x3A, 0x49, 0x55, // EASY LEVEL
    0x40, 0x40, 0x6E, 0x70, 0x40, 0x40, 0x49, 0x55, // MEDIUM LEVEL
    0x3A, 0x4D, 0x3A, 0x3A, 0x4E, 0x3A, 0x4D, 0x4E  // HARD LEVEL
};
uint8_t FACE_LEVEL[] = { 0, 8, 16 };

// This routine is run after the map is loaded, but before the
// game starts.  If the diffulcty is set to normal, nothing 
// actually happens.  But if it is set to easy or hard, then
// some changes occur accordingly.
void SET_DIFF_LEVEL()
{
    if (DIFF_LEVEL == 0) { // easy
        SET_DIFF_EASY();
    } else if (DIFF_LEVEL == 2) { // hard
        SET_DIFF_HARD();
    }
}

void SET_DIFF_EASY()
{
    // Find all hidden items and double the quantity.
    for (int X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0 &&
            UNIT_TYPE[X] != 128) { // KEY
            UNIT_A[X] <<= 1; // item qty
        }
    }
}

void SET_DIFF_HARD()
{
    // Find all hoverbots and change AI
    for (int X = 0; X != 28; X++) {
        if (UNIT_TYPE[X] == 2 || // hoverbot left/right
            UNIT_TYPE[X] == 3) { // hoverbot up/down
            UNIT_TYPE[X] = 4; // hoverbot attack mode
        }
    }
}

// This chart contains the left-most staring position for each
// row of tiles on the map-editor. 7 Rows.
uint16_t MAP_CHART[PLATFORM_MAP_WINDOW_TILES_HEIGHT];

void EMP_FLASH()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    BORDER_COLOR = 0x00f;
    BORDER = 10;
#else
#ifdef OPTIMIZED_MAP_RENDERING
    platform->fillRect(0, 0, MAP_WINDOW_WIDTH, MAP_WINDOW_HEIGHT, 15);
    INVALIDATE_PREVIOUS_MAP();
#else
    for (int Y = 0; Y != 33; Y++) {
        writeToScreenMemory(0 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[0 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 00
        writeToScreenMemory(1 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[1 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 01
        writeToScreenMemory(2 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[2 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 02
        writeToScreenMemory(3 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[3 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 03
        writeToScreenMemory(4 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[4 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 04
        writeToScreenMemory(5 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[5 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 05
        writeToScreenMemory(6 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[6 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 06
        writeToScreenMemory(7 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[7 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 07
        writeToScreenMemory(8 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[8 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 08
        writeToScreenMemory(9 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[9 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 09
        writeToScreenMemory(10 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[10 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 10
        writeToScreenMemory(11 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[11 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 11
        writeToScreenMemory(12 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[12 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 12
        writeToScreenMemory(13 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[13 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 13
        writeToScreenMemory(14 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[14 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 14
        writeToScreenMemory(15 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[15 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 15
        writeToScreenMemory(16 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[16 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 16
        writeToScreenMemory(17 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[17 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 17
        writeToScreenMemory(18 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[18 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 18
        writeToScreenMemory(19 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[19 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 19
        writeToScreenMemory(20 * SCREEN_WIDTH_IN_CHARACTERS + Y, SCREEN_MEMORY[20 * SCREEN_WIDTH_IN_CHARACTERS + Y] ^ 0x80); // screen row 20
    }
#endif
    REDRAW_WINDOW = 1;
#endif
}

// This routine animates the tile #204 (water) 
// and also tile 148 (trash compactor)
void ANIMATE_WATER()
{
    if (ANIMATE != 1) {
        return;
    }
    WATER_TIMER++;
    if (WATER_TIMER != 20) {
        return;
    }
    WATER_TIMER = 0;
#ifdef PLATFORM_IMAGE_BASED_TILES
    ANIM_STATE++;
    CINEMA_STATE++;
    if (CINEMA_STATE == 197) {
        CINEMA_STATE = 0;
    }
#else
    WATER_TEMP1 = TILE_DATA_BR[204];
    TILE_DATA_BR[204] = TILE_DATA_MM[204];
    TILE_DATA_BR[221] = TILE_DATA_MM[204];
    TILE_DATA_MM[204] = TILE_DATA_TL[204];
    TILE_DATA_TL[204] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_BL[204];
    TILE_DATA_BL[204] = TILE_DATA_MR[204];
    TILE_DATA_BL[221] = TILE_DATA_MR[204];
    TILE_DATA_MR[204] = TILE_DATA_TM[204];
    TILE_DATA_TM[204] = WATER_TEMP1;
    TILE_DATA_TM[221] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_BM[204];
    TILE_DATA_BM[204] = TILE_DATA_ML[204];
    TILE_DATA_BM[221] = TILE_DATA_ML[204];
    TILE_DATA_ML[204] = TILE_DATA_TR[204];
    TILE_DATA_TR[204] = WATER_TEMP1;
    TILE_DATA_TR[221] = WATER_TEMP1;

    // now do trash compactor
    WATER_TEMP1 = TILE_DATA_TR[148];
    TILE_DATA_TR[148] = TILE_DATA_TM[148];
    TILE_DATA_TM[148] = TILE_DATA_TL[148];
    TILE_DATA_TL[148] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_MR[148];
    TILE_DATA_MR[148] = TILE_DATA_MM[148];
    TILE_DATA_MM[148] = TILE_DATA_ML[148];
    TILE_DATA_ML[148] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_BR[148];
    TILE_DATA_BR[148] = TILE_DATA_BM[148];
    TILE_DATA_BM[148] = TILE_DATA_BL[148];
    TILE_DATA_BL[148] = WATER_TEMP1;
    // Now do HVAC fan
    if (HVAC_STATE != 0) {
        TILE_DATA_MM[196] = 0xCD;
        TILE_DATA_TL[201] = 0xCD;
        TILE_DATA_ML[197] = 0xCE;
        TILE_DATA_TM[200] = 0xCE;
        TILE_DATA_MR[196] = 0xA0;
        TILE_DATA_BM[196] = 0xA0;
        TILE_DATA_BL[197] = 0xA0;
        TILE_DATA_TR[200] = 0xA0;
        HVAC_STATE = 0;
    } else {
        TILE_DATA_MM[196] = 0xA0;
        TILE_DATA_TL[201] = 0xA0;
        TILE_DATA_ML[197] = 0xA0;
        TILE_DATA_TM[200] = 0xA0;
        TILE_DATA_MR[196] = 0xC2;
        TILE_DATA_TR[200] = 0xC2;
        TILE_DATA_BM[196] = 0xC0;
        TILE_DATA_BL[197] = 0xC0;
        HVAC_STATE = 1;
    }
    // now do cinema screen tiles
    // FIRST COPY OLD LETTERS TO THE LEFT.
    TILE_DATA_MM[20] = TILE_DATA_MR[20]; // #2 -> #1
    TILE_DATA_MR[20] = TILE_DATA_ML[21]; // #3 -> #2
    TILE_DATA_ML[21] = TILE_DATA_MM[21]; // #4 -> #3
    TILE_DATA_MM[21] = TILE_DATA_MR[21]; // #5 -> #4
    TILE_DATA_MR[21] = TILE_DATA_ML[22]; // #6 -> #5
    // now insert new character.
    TILE_DATA_ML[22] = CINEMA_MESSAGE[CINEMA_STATE]; // #6

    CINEMA_STATE++;
    if (CINEMA_STATE == 197) {
        CINEMA_STATE = 0;
    }
    // Now animate light on server computers
    if (TILE_DATA_MR[143] == 0xD7) {
        TILE_DATA_MR[143] = 0xD1;
    } else {
        TILE_DATA_MR[143] = 0xD7;
    }
    uint8_t tiles[] = { 204, 221, 148, 196, 197, 200, 201, 20, 21, 22, 143 };
    platform->updateTiles(TILE_DATA_TL, tiles, 11);
#ifdef OPTIMIZED_MAP_RENDERING
    INVALIDATE_PREVIOUS_MAP();
#endif
#endif
    REDRAW_WINDOW = 1;
}

uint8_t WATER_TIMER = 0;
#ifdef PLATFORM_IMAGE_BASED_TILES
uint8_t ANIM_STATE = 0;
#else
uint8_t WATER_TEMP1 = 0;
uint8_t HVAC_STATE = 0;
#endif
uint8_t CINEMA_STATE = 0;

// This is the routine that allows a person to select
// a level and highlights the selection in the information
// display. It is unique to each computer since it writes
// to the screen directly.
void ELEVATOR_SELECT()
{
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 0) {
#endif
        DRAW_MAP_WINDOW();
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    }
#endif
    ELEVATOR_MAX_FLOOR = UNIT_D[UNIT]; // get max levels
    // Now draw available levels on screen
    for (int Y = 0, A = 0x31; Y != ELEVATOR_MAX_FLOOR; A++, Y++) {
        writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 6 + Y, A);
    }
    ELEVATOR_CURRENT_FLOOR = UNIT_C[UNIT]; // what level are we on now?
    // Now highlight current level
    ELEVATOR_INVERT();
    platform->renderFrame();
    // Now get user input
    while (!platform->quit) {
        // KEYBOARD INPUT
        uint8_t A = platform->readKeyboard();
        // SNES INPUT
        uint16_t B = platform->readJoystick(CONTROL >= 2 ? true : false);
        if (A != 0xff || B != 0) {
#ifdef PLATFORM_LIVE_MAP_SUPPORT
            if (A == KEY_CONFIG[KEY_LIVE_MAP] || (CONTROL == 2 && (B & Platform::JoystickPlay) && (B & Platform::JoystickLeft)) || (CONTROL == 3 && (B & Platform::JoystickYellow))) {
                TOGGLE_LIVE_MAP();
                if (LIVE_MAP_ON == 0) {
                    DRAW_MAP_WINDOW();
                }
                CLEAR_KEY_BUFFER();
            } else if (A == KEY_CONFIG[KEY_LIVE_MAP_ROBOTS] || (CONTROL == 2 && (B & Platform::JoystickPlay) && (B & Platform::JoystickDown)) || (CONTROL == 3 && B == Platform::JoystickPlay)) {
                TOGGLE_LIVE_MAP_ROBOTS();
                CLEAR_KEY_BUFFER();
            } else
#endif
            if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT] || (B & Platform::JoystickLeft)) { // CURSOR LEFT
                ELEVATOR_DEC();
            } else if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT] || (B & Platform::JoystickRight)) { // CURSOR RIGHT
                ELEVATOR_INC();
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN] || (B & Platform::JoystickDown)) { // CURSOR DOWN
                SCROLL_INFO();
                SCROLL_INFO();
                SCROLL_INFO();
                CLEAR_KEY_BUFFER();
                return;
            }
        }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
        if (LIVE_MAP_ON == 1) {
            DRAW_LIVE_MAP();
        }
#endif
        platform->renderFrame(true);
    }
}

uint8_t ELEVATOR_MAX_FLOOR = 0;
uint8_t ELEVATOR_CURRENT_FLOOR = 0;

void ELEVATOR_INVERT()
{
    writeToScreenMemory((SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 5 + ELEVATOR_CURRENT_FLOOR, SCREEN_MEMORY[(SCREEN_HEIGHT_IN_CHARACTERS - 1) * SCREEN_WIDTH_IN_CHARACTERS + 5 + ELEVATOR_CURRENT_FLOOR] ^ 0x80); // %10000000
}

void ELEVATOR_INC()
{
    if (ELEVATOR_CURRENT_FLOOR != ELEVATOR_MAX_FLOOR) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR++;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
        platform->renderFrame();
    }
}

void ELEVATOR_DEC()
{
    if (ELEVATOR_CURRENT_FLOOR != 1) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR--;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
        platform->renderFrame();
    }
}

void ELEVATOR_FIND_XY()
{
    for (int X = 32; X != 48; X++) { // start of doors
        if (UNIT_TYPE[X] == 19) { // elevator
            if (UNIT_C[X] == ELEVATOR_CURRENT_FLOOR) {
                UNIT_LOC_X[0] = UNIT_LOC_X[X]; // player location = new elevator location
                MAP_WINDOW_X = MIN(MAX(UNIT_LOC_X[X] - PLATFORM_MAP_WINDOW_TILES_WIDTH / 2, 0), 128 - PLATFORM_MAP_WINDOW_TILES_WIDTH);
                UNIT_LOC_Y[0] = UNIT_LOC_Y[X] - 1; // player location = new elevator location
                MAP_WINDOW_Y = MIN(MAX(UNIT_LOC_Y[X] - PLATFORM_MAP_WINDOW_TILES_HEIGHT / 2 - 1, 0), 64 - PLATFORM_MAP_WINDOW_TILES_HEIGHT);
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                if (LIVE_MAP_ON == 0) {
#endif
                    DRAW_MAP_WINDOW();
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                }
#endif
                PLAY_SOUND(17); // elevator sound SOUND PLAY
                break;
            }
        }
    }
}

void SET_CONTROLS()
{
    if (CONTROL == 1) { // CUSTOM KEYS
        SET_CUSTOM_KEYS();
    } else {
        // load standard values for key controls
        uint8_t* STANDARD_CONTROLS = platform->standardControls();
        for (int Y = 0; Y != sizeof(KEY_CONFIG); Y++) {
            KEY_CONFIG[Y] = STANDARD_CONTROLS[Y];
        }
    }
}

void SET_CUSTOM_KEYS()
{
    if (KEYS_DEFINED != 0) {
        return;
    }
    platform->fadeScreen(0, false);
#ifdef PLATFORM_IMAGE_SUPPORT
    DECOMPRESS_SCREEN(SCR_CUSTOM_KEYS, 15);
#else
    DECOMPRESS_SCREEN(SCR_CUSTOM_KEYS);
#endif
    platform->renderFrame();
    platform->fadeScreen(15, false);
    uint16_t destination = 8 * SCREEN_WIDTH_IN_CHARACTERS + 17;
    for (TEMP_A = 0; TEMP_A != 13;) {
        uint8_t A = platform->readKeyboard();
        if (A != 0xff) {
            KEY_CONFIG[TEMP_A] = A;
            DECNUM = A;
#ifdef PLATFORM_IMAGE_SUPPORT
            DECWRITE(destination, 15);
#else
            DECWRITE(destination);
#endif
            destination += SCREEN_WIDTH_IN_CHARACTERS;
            TEMP_A++;
            platform->renderFrame();
        }
    }
    KEYS_DEFINED = 1;
}

uint8_t KEYS_DEFINED = 0; // DEFAULT 0

void PET_SCREEN_SHAKE()
{
    if (BGTIMER1 != 1) {
        return;
    }
    if (SELECT_TIMEOUT != 0) { // shoehorned this into the screenshake routine
        SELECT_TIMEOUT--; // this is to prevent accidental double-taps
    } // on cycle weapons or items.
    PET_BORDER_FLASH();
    if (SCREEN_SHAKE == 0) {
        return;
    }
#ifndef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON != 1) {
#endif
        if (SCREEN_SHAKE == 1) {
            platform->copyRect(8, 0, 0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32);
            platform->renderFrame(true);
#ifdef OPTIMIZED_MAP_RENDERING
            INVALIDATE_PREVIOUS_MAP();
#endif
        } else if (SCREEN_SHAKE == 3) {
            REDRAW_WINDOW = 1;
        }
        SCREEN_SHAKE++;
        if (SCREEN_SHAKE == 5) {
            SCREEN_SHAKE = 1;
        }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    }
#endif
#endif
}

// So, it doesn't really flash the PET border, instead it
// flashes the health screen.
void PET_BORDER_FLASH()
{
    if (BORDER != 0) {
        // border flash should be active
        if (FLASH_STATE != 1) { // Is it already flashing?
#ifndef PLATFORM_IMAGE_SUPPORT
            // copy flash message to screen
            for (int X = 0; X != 6; X++) {
                writeToScreenMemory(19 * SCREEN_WIDTH_IN_CHARACTERS - 6 + X, OUCH1[X]);
                writeToScreenMemory(20 * SCREEN_WIDTH_IN_CHARACTERS - 6 + X, OUCH2[X]);
                writeToScreenMemory(21 * SCREEN_WIDTH_IN_CHARACTERS - 6 + X, OUCH3[X]);
            }
#endif
            platform->startFadeScreen(BORDER_COLOR, 15 - BORDER);
            FLASH_STATE = 1;
        }
    } else {
        if (FLASH_STATE != 0) {
#ifndef PLATFORM_IMAGE_SUPPORT
            // Remove message from screen
            for (int X = 0; X != 6; X++) {
                writeToScreenMemory(19 * SCREEN_WIDTH_IN_CHARACTERS - 6 + X, 32);
                writeToScreenMemory(20 * SCREEN_WIDTH_IN_CHARACTERS - 6 + X, 32);
                writeToScreenMemory(21 * SCREEN_WIDTH_IN_CHARACTERS - 6 + X, 32);
            }
#endif
            FLASH_STATE = 0;
            platform->stopFadeScreen();
        }
    }
}

uint8_t FLASH_STATE = 0;
#ifndef PLATFORM_IMAGE_SUPPORT
uint8_t OUCH1[] = { 0xCD, 0xA0, 0xA0, 0xA0, 0xA0, 0xCE };
uint8_t OUCH2[] = { 0xA0, 0x8F, 0x95, 0x83, 0x88, 0xA0 };
uint8_t OUCH3[] = { 0xCE, 0xA0, 0xA0, 0xA0, 0xA0, 0xCD };
#endif

// This is actually part of a background routine, but it has to be in the main
// source because the screen effects used are unique on each system.
void DEMATERIALIZE()
{
#ifdef PLATFORM_SPRITE_SUPPORT
    UNIT_TILE[0] = 243; // dematerialize tile
#else
    UNIT_TILE[0] = (UNIT_TIMER_B[UNIT] & 0x01) + 252; // dematerialize tile
    UNIT_TILE[0] += (UNIT_TIMER_B[UNIT] & 0x08) >> 3;
#endif
    UNIT_TIMER_B[UNIT]++;
    DEMATERIALIZE_FRAME = UNIT_TIMER_B[UNIT] >> 2;
#ifdef PLATFORM_SPRITE_SUPPORT
    if (UNIT_TIMER_B[UNIT] != 0x20) {
#else
    if (UNIT_TIMER_B[UNIT] != 0x10) { // %00010000
#endif
        UNIT_TIMER_A[UNIT] = 1;
        REDRAW_WINDOW = 1;
    } else {
        // TRANSPORT COMPLETE
        if (UNIT_B[UNIT] != 1) { // transport somewhere
            UNIT_TYPE[0] = 2; // this mean game over condition, player type
            UNIT_TYPE[UNIT] = 7; // Normal transporter pad
        } else {
#ifdef PLATFORM_SPRITE_SUPPORT
            UNIT_TILE[0] = 97;
#else
            UNIT_TILE[0] = 247;
#endif
            UNIT_LOC_X[0] = UNIT_C[UNIT]; // target X coordinates
            UNIT_LOC_Y[0] = UNIT_D[UNIT]; // target Y coordinates
            UNIT_TYPE[UNIT] = 7; // Normal transporter pad
            CACULATE_AND_REDRAW();
        }
    }
}

void ANIMATE_PLAYER()
{
#ifdef PLATFORM_SPRITE_SUPPORT
    UNIT_TILE[0] = 96;
    WALK_FRAME++;
    WALK_FRAME &= 3;
#else
    if (UNIT_TILE[0] == 247) {
        UNIT_TILE[0] = 243;
    } else {
        UNIT_TILE[0] = 247;
    }
#endif
}

void PLAY_SOUND(int sound)
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playSample(sound);
#else
    // check if music is playing
    if (MUSIC_ON != 0 && SOUND_EFFECT == 0xFF) { // no sound effect currently being played
        PATTERN_TEMP = CUR_PATTERN;
        DATA_LINE_TEMP = DATA_LINE;
        TEMPO_TEMP = TEMPO;
    }
    // check if we should play new effect.
    if (SOUND_EFFECT != 0xFF) { // no sound effect currently being played
        if (sound > SOUND_EFFECT) { // Prioritize sounds that have lower number.
            return;
        }
    }
    // Now process sound effect.
    CUR_PATTERN = SOUND_LIBRARY[sound]; // Get waiting sound# from accumulator
    SOUND_EFFECT = sound;
    DATA_LINE = 0;
#endif
}

#ifndef PLATFORM_MODULE_BASED_AUDIO
uint8_t* PATTERN_TEMP;
uint8_t DATA_LINE_TEMP;
uint8_t TEMPO_TEMP;

uint8_t* SOUND_LIBRARY[] = {
    SND_EXPLOSION,      // sound 0
    SND_EXPLOSION,      // sound 1
    SND_MEDKIT,     // sound 2
    SND_EMP,        // sound 3
    SND_MAGNET,     // sound 4
    SND_SHOCK,      // sound 5
    SND_MOVE_OBJ,       // sound 6
    SND_SHOCK,      // sound 7
    SND_PLASMA,     // sound 8
    SND_PISTOL,     // sound 9
    SND_ITEM_FOUND,     // sound 10
    SND_ERROR,      // sound 11
    SND_CYCLE_WEAPON,   // sound 12
    SND_CYCLE_ITEM,     // sound 13
    SND_DOOR,       // sound 14
    SND_MENU_BEEP,      // sound 15
    SND_SHORT_BEEP,     // sound 16
    SND_SHORT_BEEP,     // sound 17
    WIN_MUSIC,      // sound 18   
    LOSE_MUSIC     // sound 19
};

// 0 explosion
// 1 small explosion
// 2 medkit
// 3 emp
// 4 haywire
// 5 evilbot
// 6 move
// 7 electric shock
// 8 plasma gun
// 9 fire pistol
// 10 item found
// 11 error
// 12 change weapons
// 13 change items
// 14 door
// 15 menu beep
// 16 walk
// 17 sfx (short beep)
// 18 sfx

void MUSIC_ROUTINE()
{
    if (SOUND_EFFECT == 0xFF && MUSIC_ON != 1) {
        return;
    }
    if (TEMPO_TIMER != 0) {
        TEMPO_TIMER--;
        return;
    }
    uint8_t A = CUR_PATTERN[DATA_LINE];
    if (A == 0) { // blank line (do nothing)
        TEMPO_TIMER = TEMPO; // reset timer to wait for next line
        DATA_LINE++;
        return;
    }
    if (A == 37) { // END pattern
        STOP_SONG();
        return;
    }
    if (A == 38) {
        ARP_MODE = 0;
        platform->stopNote(); // RESET SOUND TO ZERO
    }
    if (A > 38 && A <= 49) { // IS IT A TEMPO COMMAND? COMMAND IS BETWEEN 39 AND 49 (TEMPO ADJUST)
        TEMPO = A - 38;
        DATA_LINE++;
        return;
    }
    // PLAY A NOTE
    ARP_MODE = A >> 6;
    CHORD_ROOT = A & 0x3f; // %00111111
    platform->playNote(CHORD_ROOT);
    TEMPO_TIMER = TEMPO; // reset timer to wait for next line
    DATA_LINE++;
}
#endif

void STOP_SONG()
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->stopSample();
#else
    // actually, stop sound effect.
    platform->stopNote(); // turn off sound;
    SOUND_EFFECT = 0xFF;
    TEMPO_TIMER = TEMPO;
    /// now restore music info for continued play.
    if (MUSIC_ON != 1) {
        return;
    }
    CUR_PATTERN = PATTERN_TEMP;
    DATA_LINE = DATA_LINE_TEMP;
    TEMPO = TEMPO_TEMP;
#endif
}

void BACKGROUND_TASKS()
{
    if (BGTIMER1 == 1) {
#ifdef PLATFORM_LIVE_MAP_SUPPORT
        if (LIVE_MAP_ON) {
            DRAW_LIVE_MAP();
        } else
#endif
        if (REDRAW_WINDOW == 1) {
            REDRAW_WINDOW = 0;
            DRAW_MAP_WINDOW();
        }
        platform->renderFrame();
    }
    // Now check to see if it is time to run background tasks
    if (BGTIMER1 != 1) {
        return;
    }
    BGTIMER1 = 0; // RESET BACKGROUND TIMER
    for (UNIT = 1; UNIT != 64; UNIT++) {
        // ALL AI routines must JMP back to here at the end.
        if (UNIT_TYPE[UNIT] != 0) { // Does unit exist?
            // Unit found to exist, now check it's timer.
            // unit code won't run until timer hits zero.
            if (UNIT_TIMER_A[UNIT] != 0) {
                UNIT_TIMER_A[UNIT]--; // Decrease timer by one.
            } else {
                // Unit exists and timer has triggered
                // The unit type determines which AI routine is run.
                if (UNIT_TYPE[UNIT] < 24) { // MAX DIFFERENT UNIT TYPES IN CHART, ABORT IF GREATER
                    AI_ROUTINE_CHART[UNIT_TYPE[UNIT]]();
                }
            }
        }
    }
}

void (*AI_ROUTINE_CHART[])(void) =
{
    DUMMY_ROUTINE,      // UNIT TYPE 00   ;non-existent unit
    DUMMY_ROUTINE,      // UNIT TYPE 01   ;player unit - can't use
    LEFT_RIGHT_DROID,   // UNIT TYPE 02
    UP_DOWN_DROID,      // UNIT TYPE 03
    HOVER_ATTACK,       // UNIT TYPE 04
    WATER_DROID,        // UNIT TYPE 05
    TIME_BOMB,      // UNIT TYPE 06
    TRANSPORTER_PAD,    // UNIT TYPE 07
    DEAD_ROBOT,     // UNIT TYPE 08
    EVILBOT,        // UNIT TYPE 09 
    AI_DOOR,        // UNIT TYPE 10
    SMALL_EXPLOSION,    // UNIT TYPE 11
    PISTOL_FIRE_UP,     // UNIT TYPE 12
    PISTOL_FIRE_DOWN,   // UNIT TYPE 13
    PISTOL_FIRE_LEFT,   // UNIT TYPE 14
    PISTOL_FIRE_RIGHT,  // UNIT TYPE 15
    TRASH_COMPACTOR,    // UNIT TYPE 16
    UP_DOWN_ROLLERBOT,  // UNIT TYPE 17
    LEFT_RIGHT_ROLLERBOT,   // UNIT TYPE 18
    ELEVATOR,       // UNIT TYPE 19
    MAGNET,         // UNIT TYPE 20
    MAGNETIZED_ROBOT,   // UNIT TYPE 21
    WATER_RAFT_LR,      // UNIT TYPE 22
    DEMATERIALIZE      // UNIT TYPE 23
};

// Dummy routine does nothing, but I need it for development.
void DUMMY_ROUTINE()
{
    return;
}

void WATER_RAFT_LR()
{
    // First check which direction raft is moving.
    if (UNIT_A[UNIT] == 1) {
        RAFT_DELETE();
        // Check to see if player is on raft
        if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] &&
            UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
            UNIT_LOC_X[0]++; // player
            UNIT_LOC_X[UNIT]++; // raft
            RAFT_PLOT();
            CACULATE_AND_REDRAW();
        } else {
            CHECK_FOR_WINDOW_REDRAW();
            UNIT_LOC_X[UNIT]++; // raft
            RAFT_PLOT();
            CHECK_FOR_WINDOW_REDRAW();
        }
        // Now check if it has reached its destination
        if (UNIT_LOC_X[UNIT] != UNIT_C[UNIT]) {
            UNIT_TIMER_A[UNIT] = 6;
        } else {
            UNIT_TIMER_A[UNIT] = 100;
            UNIT_A[UNIT] = 0;
        }
    } else {
        RAFT_DELETE();
        // Check to see if player is on raft
        if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] &&
            UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
            UNIT_LOC_X[0]--; // player
            UNIT_LOC_X[UNIT]--; // raft
            RAFT_PLOT();
            CACULATE_AND_REDRAW();
        } else {
            CHECK_FOR_WINDOW_REDRAW();
            UNIT_LOC_X[UNIT]--; // raft
            RAFT_PLOT();
            CHECK_FOR_WINDOW_REDRAW();
        }
        // Now check if it has reached its destination
        if (UNIT_LOC_X[UNIT] != UNIT_B[UNIT]) {
            UNIT_TIMER_A[UNIT] = 6;
        } else {
            UNIT_TIMER_A[UNIT] = 100;
            UNIT_A[UNIT] = 1;
        }
    }
}

void RAFT_DELETE()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = 204; // WATER TILE
    PLOT_TILE_TO_MAP();
}

void RAFT_PLOT()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = 242; // RAFT TILE
    PLOT_TILE_TO_MAP();
}

void MAGNETIZED_ROBOT()
{
    CHECK_FOR_WINDOW_REDRAW();
    MOVE_TYPE = 0x01; // %00000001
    GENERATE_RANDOM_NUMBER();
    switch (RANDOM & 0x03) { // %00000011
    case 0:
        REQUEST_WALK_UP();
        break;
    case 1:
        REQUEST_WALK_DOWN();
        break;
    case 2:
        REQUEST_WALK_LEFT();
        break;
    case 3:
        REQUEST_WALK_RIGHT();
        break;
    default:
        break;
    }
    CHECK_FOR_WINDOW_REDRAW();
    UNIT_TIMER_A[UNIT] = 10;
    UNIT_TIMER_B[UNIT]--;
    if (UNIT_TIMER_B[UNIT] == 0) {
        UNIT_TYPE[UNIT] = UNIT_D[UNIT];
    }
}

void GENERATE_RANDOM_NUMBER()
{
    if (RANDOM != 0) { // added this
        if (RANDOM & 0x80) {
            RANDOM = (RANDOM << 1) ^ 0x1D;
        } else {
            RANDOM = (RANDOM << 1);
        }
    } else {
        RANDOM = 0x1D;
    }
}

void MAGNET()
{
    // First let's take care of the timers.  This unit runs
    // every cycle so that it can detect contact with another
    // unit.  But it still needs to count down to terminate
    // So, it uses two timers for a 16-bit value.
    UNIT_TIMER_B[UNIT]--;
    if (UNIT_TIMER_B[UNIT] == 0) {
        UNIT_A[UNIT]--;
        if (UNIT_A[UNIT] == 0) {
            // Both timers have reached zero, time to deactivate.
            UNIT_TYPE[UNIT] = 0;
            MAGNET_ACT = 0;
            return;
        }
    }
    // Now let's see if another units walks on the magnet.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    CHECK_FOR_UNIT();
    if (UNIT_FIND == 255) { // no unit found
        return;
    } else if (UNIT_FIND == 0) { // player unit
        INV_MAGNET++;
        DISPLAY_ITEM();
    } else {
        // Collision with robot detected.
        PLAY_SOUND(4); // HAYWIRE SOUND, SOUND PLAY
        UNIT_D[UNIT_FIND] = UNIT_TYPE[UNIT_FIND]; // make backup of unit type
        UNIT_TYPE[UNIT_FIND] = 21; // Crazy robot AI
        UNIT_TIMER_B[UNIT_FIND] = 60;
    }
    UNIT_TYPE[UNIT] = 0;
    MAGNET_ACT = 0;
}

void DEAD_ROBOT()
{
    UNIT_TYPE[UNIT] = 0;
}

void UP_DOWN_ROLLERBOT()
{
    UNIT_TIMER_A[UNIT] = 7;
    ROLLERBOT_ANIMATE();
    CHECK_FOR_WINDOW_REDRAW();
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=UP 1=DOWN
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_UP();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_DOWN();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void LEFT_RIGHT_ROLLERBOT()
{
    UNIT_TIMER_A[UNIT] = 7;
    ROLLERBOT_ANIMATE();
    CHECK_FOR_WINDOW_REDRAW();
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=LEFT 1=RIGHT
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_LEFT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_RIGHT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void ROLLERBOT_FIRE_DETECT()
{
    int X;
    TEMP_A = UNIT_LOC_X[UNIT];
    TEMP_B = UNIT_LOC_Y[UNIT];
    // See if we're lined up vertically
    if (UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) { // robot, player
        if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
            // Check to see if distance is less than 5
            if (UNIT_LOC_X[UNIT] - UNIT_LOC_X[0] >= 6) { // robot, player
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 14; // pistol fire left AI
                    ROLLERBOT_AFTER_FIRE(X, 245); // tile for horizontal weapons fire
                    return;
                }
            }
        } else {
            // Check to see if distance is less than 5
            if (UNIT_LOC_X[0] - UNIT_LOC_X[UNIT] >= 6) { // player, robot
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 15; // pistol fire right AI
                    ROLLERBOT_AFTER_FIRE(X, 245); // tile for horizontal weapons fire
                    return;
                }
            }
        }
    } else if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0]) { // robot, player
        // See if we're lined up horizontally
        if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
            // Check to see if distance is less than 5
            if (UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0] >= 4) { // robot, player
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 12; // pistol fire UP AI
                    ROLLERBOT_AFTER_FIRE(X, 244); // tile for horizontal weapons fire
                    return;
                }
            }
        } else {
            // Check to see if distance is less than 5
            if (UNIT_LOC_Y[0] - UNIT_LOC_Y[UNIT] >= 4) { // player, robot
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 13; // pistol fire DOWN AI
                    ROLLERBOT_AFTER_FIRE(X, 244); // tile for horizontal weapons fire
                    return;
                }
            }
        }
    }
}

void ROLLERBOT_AFTER_FIRE(uint8_t unit, uint8_t tile)
{
    UNIT_TILE[unit] = tile;
    UNIT_A[unit] = 5; // travel distance.
    UNIT_B[unit] = 0; // weapon-type = pistol
    UNIT_TIMER_A[unit] = 0;
    UNIT_LOC_X[unit] = TEMP_A;
    UNIT_LOC_Y[unit] = TEMP_B;
    PLAY_SOUND(9); // PISTOL SOUND SOUND PLAY
}

void ROLLERBOT_ANIMATE()
{
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
        return;
    }
    UNIT_TIMER_B[UNIT] = 3; // RESET ANIMATE TIMER
    if (UNIT_TILE[UNIT] == 164) {
        UNIT_TILE[UNIT] = 165; // ROLLERBOT TILE
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TILE[UNIT] = 164; // ROLLERBOT TILE
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// UNIT_A: 0=always active    1=only active when all robots are dead
// UNIT_B: 0=completes level 1=send to coordinates
// UNIT_C: X-coordinate
// UNIT_D: Y-coordinate

// The "DEMATERIALIZE" part of this AI routine has to be in the main 
// source for each individual computer, because the screen effects
// are created uniquely for each one.

void TRANSPORTER_PAD()
{
    // first determine if the player is standing here
    if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] && UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
        TRANS_PLAYER_PRESENT();
    } else {
        // player not present
        if (UNIT_A[UNIT] != 1) {
            TRANS_ACTIVE();
        } else {
            // test if all robots are dead
            for (int X = 1; X != 28; X++) {
                if (UNIT_TYPE[X] != 0) {
                    UNIT_TIMER_A[UNIT] = 30;
                    return;
                }
            }
            UNIT_A[UNIT] = 0; // make unit active
            UNIT_TIMER_A[UNIT] = 30;
        }
    }
}

void TRANS_PLAYER_PRESENT()
{
    if (UNIT_A[UNIT] != 0) { // unit active
        PRINT_INFO(MSG_TRANS1);
        PLAY_SOUND(11); // error-SOUND SOUND PLAY
        UNIT_TIMER_A[UNIT] = 100;
    } else {
        // start transport process
        UNIT_TYPE[UNIT] = 23; // Convert to different AI
        UNIT_TIMER_A[UNIT] = 5;
        UNIT_TIMER_B[UNIT] = 0;
    }
}

void TRANS_ACTIVE()
{
    if (UNIT_TIMER_B[UNIT] != 1) {
        UNIT_TIMER_B[UNIT] = 1;
        TILE = 30;
    } else {
        UNIT_TIMER_B[UNIT] = 0;
        TILE = 31;
    }
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    PLOT_TILE_TO_MAP();
    CHECK_FOR_WINDOW_REDRAW();
    UNIT_TIMER_A[UNIT] = 30;
}

void TIME_BOMB()
{
    if (UNIT_A[UNIT] == 0) {
        BIG_EXP_PHASE1();
    } else if (UNIT_A[UNIT] == 1) {
        BIG_EXP_PHASE2();
    }
}

// This is the large explosion used by the time-bomb
// and plasma gun, and maybe others.  This is the first
// phase of the explosion, which stores the tiles to
// a buffer and then changes each tile to an explosion.
void BIG_EXP_PHASE1()
{
    if (BIG_EXP_ACT != 0) { // Check that no other explosion active.
        UNIT_TIMER_A[UNIT] = 10;
        return; // wait for existing explosion to finish.
    }
    BIG_EXP_ACT = 1; // Set flag so no other explosions can begin until this one ends.
    SCREEN_SHAKE = 1;
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    platform->startShakeScreen();
#endif
    PLAY_SOUND(0); // explosion-sound SOUND PLAY
    BEX_PART1(); // check center piece for unit
    BEXCEN(); // check center piece for unit
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 1) {
        GET_TILE_FROM_MAP();
        MAP_SOURCE[0] = 246;
        platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
        MAP_SOURCE[0] = TILE;
    }
#endif
    BEX1_NORTH();
    BEX1_SOUTH();
    BEX1_EAST();
    BEX1_WEST();
    BEX1_NE();
    BEX1_NW();
    BEX1_SE();
    BEX1_SW();
    UNIT_TILE[UNIT] = 246; // explosion tile
    UNIT_A[UNIT] = 1; // move to next phase of explosion.
    UNIT_TIMER_A[UNIT] = 12;
    REDRAW_WINDOW = 1;
}

// There are 8 separate subroutines for the large explosion
// with each one handling a specific outward direction of motion.
// The "unit" itself changes tiles to an explosion, so we don't
// need to mess with the center tile.
void BEX1_NORTH()
{
    BEX_PART1();
    // first tile
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[0] = TILE;
    BEX_PART3();
    // second tile
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[1] = TILE;
    BEX_PART3();
}

void BEX1_SOUTH()
{
    BEX_PART1();
    // first tile
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[2] = TILE;
    BEX_PART3();
    // second tile
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[3] = TILE;
    BEX_PART3();
}

void BEX1_EAST()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[4] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[5] = TILE;
    BEX_PART3();
}

void BEX1_WEST()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[6] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[7] = TILE;
    BEX_PART3();
}

void BEX1_NE()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[8] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[9] = TILE;
    BEX_PART3();
}

void BEX1_NW()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[10] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[11] = TILE;
    BEX_PART3();
}

void BEX1_SE()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[12] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[13] = TILE;
    BEX_PART3();
}

void BEX1_SW()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[14] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[15] = TILE;
    BEX_PART3();
}

void BEX_PART1()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
}

bool BEX_PART2()
{
    GET_TILE_FROM_MAP();
    return (TILE_ATTRIB[TILE] & 0x10) == 0x10; // can see through tile?
}

void BEX_PART3()
{
    MAP_SOURCE[0] = 246;
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 1) {
        platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
    }
#endif
    BEXCEN();
}

void BEXCEN()
{
    CHECK_FOR_UNIT();
    if (UNIT_FIND != 255) {
        TEMP_A = 11;
        INFLICT_DAMAGE();
    }
}

void BIG_EXP_PHASE2()
{
    // Do the center tile first.
    BEX_PART1();
    GET_TILE_FROM_MAP();
    MAP_SOURCE[0] = 246;
    TEMP_A = TILE;
    RESTORE_TILE();
    // tile #0 north 1
    BEX_PART1();
    MAP_Y--;
    TEMP_A = EXP_BUFFER[0];
    RESTORE_TILE();
    // tile #1 north 2
    MAP_Y--;
    TEMP_A = EXP_BUFFER[1];
    RESTORE_TILE();
    // tile #2 south 1
    BEX_PART1();
    MAP_Y++;
    TEMP_A = EXP_BUFFER[2];
    RESTORE_TILE();
    // tile #3 south 2
    MAP_Y++;
    TEMP_A = EXP_BUFFER[3];
    RESTORE_TILE();
    // tile #4 east 1
    BEX_PART1();
    MAP_X++;
    TEMP_A = EXP_BUFFER[4];
    RESTORE_TILE();
    // tile #5 east 2
    MAP_X++;
    TEMP_A = EXP_BUFFER[5];
    RESTORE_TILE();
    // tile #6 west 1
    BEX_PART1();
    MAP_X--;
    TEMP_A = EXP_BUFFER[6];
    RESTORE_TILE();
    // tile #7 west 2
    MAP_X--;
    TEMP_A = EXP_BUFFER[7];
    RESTORE_TILE();
    // tile #8 northeast 1
    BEX_PART1();
    MAP_Y--;
    MAP_X++;
    TEMP_A = EXP_BUFFER[8];
    RESTORE_TILE();
    // tile #9 northeast 2
    MAP_Y--;
    MAP_X++;
    TEMP_A = EXP_BUFFER[9];
    RESTORE_TILE();
    // tile #10 northwest 1
    BEX_PART1();
    MAP_Y--;
    MAP_X--;
    TEMP_A = EXP_BUFFER[10];
    RESTORE_TILE();
    // tile #11 northwest 2
    MAP_Y--;
    MAP_X--;
    TEMP_A = EXP_BUFFER[11];
    RESTORE_TILE();
    // tile #12 southeast 1
    BEX_PART1();
    MAP_Y++;
    MAP_X++;
    TEMP_A = EXP_BUFFER[12];
    RESTORE_TILE();
    // tile #13 southeast 2
    MAP_Y++;
    MAP_X++;
    TEMP_A = EXP_BUFFER[13];
    RESTORE_TILE();
    // tile #14 southwest 1
    BEX_PART1();
    MAP_Y++;
    MAP_X--;
    TEMP_A = EXP_BUFFER[14];
    RESTORE_TILE();
    // tile #15 southwest 2
    MAP_Y++;
    MAP_X--;
    TEMP_A = EXP_BUFFER[15];
    RESTORE_TILE();
    REDRAW_WINDOW = 1;
    UNIT_TYPE[UNIT] = 0; // Deactivate this AI
    BIG_EXP_ACT = 0;
    SCREEN_SHAKE = 0;
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    platform->stopShakeScreen();
#endif
}

void RESTORE_TILE()
{
    GET_TILE_FROM_MAP();
    if (TILE != 246) {
        return;
    }
    if (TEMP_A != 131) { // Canister tile
        if ((TILE_ATTRIB[TEMP_A] & 0x08) == 0x08) { // %00001000 can it be destroyed?
            MAP_SOURCE[0] = DESTRUCT_PATH[TEMP_A];
        } else {
            MAP_SOURCE[0] = TEMP_A;
        }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
        if (LIVE_MAP_ON == 1) {
            platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
        }
#endif
    } else {
        // What to do if we encounter an explosive canister
        MAP_SOURCE[0] = 135; // Blown canister
#ifdef PLATFORM_LIVE_MAP_SUPPORT
        if (LIVE_MAP_ON == 1) {
            platform->renderLiveMapTile(MAP, MAP_X, MAP_Y);
        }
#endif
        for (int X = 28; X != 32; X++) { // Start of weapons units
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 6; // bomb AI
                UNIT_TILE[X] = 131; // Canister tile
                UNIT_LOC_X[X] = MAP_X;
                UNIT_LOC_Y[X] = MAP_Y;
                UNIT_TIMER_A[X] = 10; // How long until exposion?
                UNIT_A[X] = 0;
                return;
            }
        }
        // no slots available right now, abort.
    }
}

void TRASH_COMPACTOR()
{
    if (UNIT_A[UNIT] == 0) { // OPEN
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE == 148) { // Usual tile for trash compactor danger zone
            MAP_SOURCE[1] = TILE;
            UNIT_TIMER_A[UNIT] = 20;
            // now check for units in the compactor
            MAP_X = UNIT_LOC_X[UNIT];
            MAP_Y = UNIT_LOC_Y[UNIT];
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                return; // Nothing found, do nothing.
            }
        }
        // Object has been detected in TC, start closing.
        TCPIECE1 = 146;
        TCPIECE2 = 147;
        TCPIECE3 = 150;
        TCPIECE4 = 151;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 10;
        PLAY_SOUND(14); // door sound SOUND PLAY
    } else if (UNIT_A[UNIT] == 1) { // MID-CLOSING STATE
        TCPIECE1 = 152;
        TCPIECE2 = 153;
        TCPIECE3 = 156;
        TCPIECE4 = 157;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 50;
        // Now check for any live units in the compactor
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) {
            MAP_X++; // check second tile
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                return;
            }
        }
        // Found unit in compactor, kill it.
        PRINT_INFO(MSG_TERMINATED);
        PLAY_SOUND(0); // EXPLOSION sound SOUND PLAY
        UNIT_TYPE[UNIT_FIND] = 0;
        UNIT_HEALTH[UNIT_FIND] = 0;
        for (int X = 28; X != 32; X++) { // start of weapons
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 11; // SMALL EXPLOSION
                UNIT_TILE[X] = 248; // first tile for explosion
                UNIT_LOC_X[X] = UNIT_LOC_X[UNIT];
                UNIT_LOC_Y[X] = UNIT_LOC_Y[UNIT];
                if (UNIT_FIND == 0) { // is it the player
                    BORDER_COLOR = 0xf00;
                    BORDER = 10;
                }
                break;
            }
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else if (UNIT_A[UNIT] == 2) { // CLOSED STATE
        TCPIECE1 = 146;
        TCPIECE2 = 147;
        TCPIECE3 = 150;
        TCPIECE4 = 151;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 10;
    } else if (UNIT_A[UNIT] == 3) { // MID-OPENING STATE
        TCPIECE1 = 144;
        TCPIECE2 = 145;
        TCPIECE3 = 148;
        TCPIECE4 = 148;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT] = 0;
        UNIT_TIMER_A[UNIT] = 20;
        PLAY_SOUND(14); // door sound SOUND PLAY
    } else {
        // should never get here.
    }
}

void DRAW_TRASH_COMPACTOR()
{
    MAP_Y = UNIT_LOC_Y[UNIT];
    MAP_Y--; // start one tile above
    MAP_X = UNIT_LOC_X[UNIT];
    TILE = TCPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE[1] = TCPIECE2;
    MAP_SOURCE[128] = TCPIECE3;
    MAP_SOURCE[129] = TCPIECE4;
    CHECK_FOR_WINDOW_REDRAW();
}
uint8_t TCPIECE1 = 0;
uint8_t TCPIECE2 = 0;
uint8_t TCPIECE3 = 0;
uint8_t TCPIECE4 = 0;

void WATER_DROID()
{
    // first rotate the tiles
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
    } else {
        UNIT_TIMER_B[UNIT] = 3;
        UNIT_TILE[UNIT]++;
        if (UNIT_TILE[UNIT] == 143) {
            UNIT_TILE[UNIT] = 140;
        }
    }
    UNIT_A[UNIT]--;
    if (UNIT_A[UNIT] != 0) {
        CHECK_FOR_WINDOW_REDRAW();
        return;
    }
    // kill unit after countdown reaches zero. 
    UNIT_A[UNIT] = UNIT_TYPE[UNIT];
    UNIT_TYPE[UNIT] = 8; // Dead robot type
    UNIT_TIMER_A[UNIT] = 255;
    UNIT_TILE[UNIT] = 115; // dead robot tile
    CHECK_FOR_WINDOW_REDRAW();
}

void PISTOL_FIRE_UP()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_Y[UNIT]--; // move it up one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_DOWN()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_Y[UNIT]++; // move it down one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_LEFT()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_X[UNIT]--; // move it left one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_RIGHT()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        UNIT_LOC_X[UNIT]++; // move it right one.
        PISTOL_AI_COMMON();
    }
}

void DEACTIVATE_WEAPON()
{
    UNIT_TYPE[UNIT] = 0;
    if (UNIT_B[UNIT] == 1) {
        UNIT_B[UNIT] = 0;
        PLASMA_ACT = 0;
    }
}

void PISTOL_AI_COMMON()
{
    if (UNIT_B[UNIT] == 0) { // is it pistol or plasma?
        UNIT_A[UNIT]--; // reduce range by one
        // Now check what map object it is on.
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE == 131) { // explosive canister
            // hit an explosive canister
            MAP_SOURCE[0] = 135; // Blown canister
            UNIT_TYPE[UNIT] = 6; // bomb AI
            UNIT_TILE[UNIT] = 131; // Canister tile
            UNIT_LOC_X[UNIT] = MAP_X;
            UNIT_LOC_Y[UNIT] = MAP_Y;
            UNIT_TIMER_A[UNIT] = 5; // How long until exposion?
            UNIT_A[UNIT] = 0;
        } else if ((TILE_ATTRIB[TILE] & 0x10) != 0x10) { // can see through tile?
            // Hit object that can't pass through, convert to explosion
            UNIT_TYPE[UNIT] = 11; // SMALL EXPLOSION
            UNIT_TILE[UNIT] = 248; // first tile for explosion
            CHECK_FOR_WINDOW_REDRAW();
        } else {
            // check if it encountered a robot/human
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) { // NO UNIT ENCOUNTERED.
                CHECK_FOR_WINDOW_REDRAW();
            } else {
                // struck a robot/human
                UNIT_TYPE[UNIT] = 11; // SMALL EXPLOSION
                UNIT_TILE[UNIT] = 248; // first tile for explosion
                TEMP_A = 1; // set damage for pistol
                INFLICT_DAMAGE();
                ALTER_AI();
                CHECK_FOR_WINDOW_REDRAW();
            }
        }
    } else {
        UNIT_A[UNIT]--; // reduce range by one
        // find what tile we are over
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE != 131) { // canister tile
            if ((TILE_ATTRIB[TILE] & 0x10) == 0x10) {
                // check if it encountered a human/robot
                CHECK_FOR_UNIT();
                if (UNIT_FIND == 255) { // NO UNIT ENCOUNTERED.
                    // no impacts detected:
                    CHECK_FOR_WINDOW_REDRAW();
                    return;
                }
            }
        }
        // impact detected. convert to explosion
        UNIT_TYPE[UNIT] = 6; // bomb AI
        UNIT_TIMER_A[UNIT] = 1; // How long until exposion?
        UNIT_A[UNIT] = 0;
        PLASMA_ACT = 0;
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// This routine checks to see if the robot being shot
// is a hoverbot, if so it will alter it's AI to attack 
// mode.
void ALTER_AI()
{
    if (UNIT_TYPE[UNIT_FIND] == 2 || UNIT_TYPE[UNIT_FIND] == 3) { // hoverbot left/right UP/DOWN
        UNIT_TYPE[UNIT_FIND] = 4; // Attack AI
    }
}

// This routine will inflict damage on whatever is defined in
// UNIT_FIND in the amount set in TEMP_A.  If the damage is more
// than the health of that unit, it will delete the unit.
void INFLICT_DAMAGE()
{
    UNIT_HEALTH[UNIT_FIND] -= TEMP_A;
    if (UNIT_HEALTH[UNIT_FIND] > 0) {
        if (UNIT_FIND == 0) { // IS IT THE PLAYER?
            DISPLAY_PLAYER_HEALTH();
            BORDER_COLOR = 0xf00;
            BORDER = 10;
        }
        return;
    }
    UNIT_HEALTH[UNIT_FIND] = 0;
    if (UNIT_FIND != 0) { // Is it the player that is dead?
        if (UNIT_TYPE[UNIT_FIND] != 8) { // Dead robot type - is it a dead robot already?
            UNIT_A[UNIT_FIND] = UNIT_TYPE[UNIT_FIND];
            UNIT_TYPE[UNIT_FIND] = 8;
            UNIT_TIMER_A[UNIT_FIND] = 255;
            UNIT_TILE[UNIT_FIND] = 115; // dead robot tile
        }
    } else {
        UNIT_TYPE[UNIT_FIND] = 0;
        DISPLAY_PLAYER_HEALTH();
        BORDER_COLOR = 0xf00;
        BORDER = 10;
    }
}

void SMALL_EXPLOSION()
{
    UNIT_TIMER_A[UNIT] = 0;
    UNIT_TILE[UNIT]++;
    if (UNIT_TILE[UNIT] != 252) {
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TYPE[UNIT] = 0;
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void HOVER_ATTACK()
{
    UNIT_TIMER_B[UNIT] = 0;
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 7;
    CHECK_FOR_WINDOW_REDRAW();
    MOVE_TYPE = 0x02; // %00000010 HOVER
    // CHECK FOR HORIZONTAL MOVEMENT
    if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
        REQUEST_WALK_LEFT();
    } else if (UNIT_LOC_X[UNIT] < UNIT_LOC_X[0]) {
        REQUEST_WALK_RIGHT();
    }
    // NOW CHECK FOR VERTICAL MOVEMENT
    if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
        REQUEST_WALK_UP();
    } else if (UNIT_LOC_Y[UNIT] < UNIT_LOC_Y[0]) {
        REQUEST_WALK_DOWN();
    }
    ROBOT_ATTACK_RANGE();
    if (PROX_DETECT == 1) { // 1=Robot next to player 0=not
        TEMP_A = 1; // amount of damage it will inflict
        UNIT_FIND = 0; // unit to inflict damage on.
        INFLICT_DAMAGE();
        CREATE_PLAYER_EXPLOSION();
        PLAY_SOUND(7); // electric shock SOUND PLAY
        UNIT_TIMER_A[UNIT] = 30; // rate of attack on player.
        // add some code here to create explosion
    }
    CHECK_FOR_WINDOW_REDRAW();
}

void CREATE_PLAYER_EXPLOSION()
{
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 11; // Small explosion AI type
            UNIT_TILE[X] = 248; // first tile for explosion
            UNIT_TIMER_A[X] = 1;
            UNIT_LOC_X[X] = UNIT_LOC_X[0];
            UNIT_LOC_Y[X] = UNIT_LOC_Y[0];
            break;
        }
    }
}

void EVILBOT()
{
    UNIT_TIMER_A[UNIT] = 5;
    // first animate evilbot
    if (UNIT_TILE[UNIT] == 100) {
        UNIT_TILE[UNIT]++;
    } else if (UNIT_TILE[UNIT] == 101) {
        UNIT_TILE[UNIT]++;
    } else if (UNIT_TILE[UNIT] == 102) {
        UNIT_TILE[UNIT]++;
    } else {
        UNIT_TILE[UNIT] = 100;
    }
    // now figure out movement
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TIMER_B[UNIT] = 1; // Reset timer B
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x01; // %00000001 WALK
        // CHECK FOR HORIZONTAL MOVEMENT
        if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
            REQUEST_WALK_LEFT();
        } else if (UNIT_LOC_X[UNIT] < UNIT_LOC_X[0]) {
            REQUEST_WALK_RIGHT();
        }
        // NOW CHECK FOR VERTICAL MOVEMENT
        if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
            REQUEST_WALK_UP();
        } else if (UNIT_LOC_Y[UNIT] < UNIT_LOC_Y[0]) {
            REQUEST_WALK_DOWN();
        }
        ROBOT_ATTACK_RANGE();
        if (PROX_DETECT == 1) { // 1=Robot next to player 0=not
            TEMP_A = 5; // amount of damage it will inflict
            UNIT_FIND = 0; // unit to inflict damage on.
            INFLICT_DAMAGE();
            CREATE_PLAYER_EXPLOSION();
            PLAY_SOUND(7); // electric shock sound SOUND PLAY
            UNIT_TIMER_A[UNIT] = 15; // rate of attack on player.
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// This routine handles automatic sliding doors.
// UNIT_B register means:
// 0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
void AI_DOOR()
{
    if (UNIT_B[UNIT] < 6) { // make sure number is in bounds
        AIDB[UNIT_B[UNIT]]();
    }
    // -SHOULD NEVER NEED TO HAPPEN
}
void (*AIDB[])(void) = {
    DOOR_OPEN_A,
    DOOR_OPEN_B,
    DOOR_OPEN_FULL,
    DOOR_CLOSE_A,
    DOOR_CLOSE_B,
    DOOR_CLOSE_FULL
};

void DOOR_OPEN_A()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 88;
        DOORPIECE2 = 89;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 70;
        DOORPIECE2 = 74;
        DOORPIECE3 = 78;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 1;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_OPEN_B()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 17;
        DOORPIECE2 = 9;
        DOORPIECE3 = 91;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 27;
        DOORPIECE2 = 9;
        DOORPIECE3 = 15;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 2;
    UNIT_TIMER_A[UNIT] = 30;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_OPEN_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT == 1) {
        UNIT_TIMER_B[UNIT] = 30; // RESET TIMER
        return;
    }
    // if nobody near door, lets close it.
    // check for object in the way first.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    GET_TILE_FROM_MAP();
    if (TILE != 9) { // FLOOR-TILE
        // SOMETHING IN THE WAY, ABORT
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    PLAY_SOUND(14); // DOOR-SOUND SOUND PLAY
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 88;
        DOORPIECE2 = 89;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 70;
        DOORPIECE2 = 74;
        DOORPIECE3 = 78;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 3;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_A()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 84;
        DOORPIECE2 = 85;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 69;
        DOORPIECE2 = 73;
        DOORPIECE3 = 77;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 4;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_B()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 80;
        DOORPIECE2 = 81;
        DOORPIECE3 = 82;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 68;
        DOORPIECE2 = 72;
        DOORPIECE3 = 76;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 5;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT != 0) {
        // if player near door, lets open it.
        // first check if locked
        if (UNIT_C[UNIT] == 0 || // Lock status
            (UNIT_C[UNIT] == 1 && (KEYS & 0x01) == 0x01) || // SPADE KEY
            (UNIT_C[UNIT] == 2 && (KEYS & 0x02) == 0x02) || // HEART KEY
            (UNIT_C[UNIT] == 3 && (KEYS & 0x04) == 0x04)) { // STAR KEY
            // Start open door process
            PLAY_SOUND(14); // DOOR-SOUND SOUND PLAY
            if (UNIT_A[UNIT] != 1) {
                // HORIZONTAL DOOR
                DOORPIECE1 = 84;
                DOORPIECE2 = 85;
                DOORPIECE3 = 86;
                DRAW_HORIZONTAL_DOOR();
            } else {
                // VERTICAL DOOR
                DOORPIECE1 = 69;
                DOORPIECE2 = 73;
                DOORPIECE3 = 77;
                DRAW_VERTICAL_DOOR();
            }
            UNIT_B[UNIT] = 0;
            UNIT_TIMER_A[UNIT] = 5;
            CHECK_FOR_WINDOW_REDRAW();
            return;
        }
    }
    UNIT_TIMER_A[UNIT] = 20; // RESET TIMER
}

void DRAW_VERTICAL_DOOR()
{
    MAP_Y = UNIT_LOC_Y[UNIT];
    MAP_Y--;
    MAP_X = UNIT_LOC_X[UNIT];
    TILE = DOORPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE += 128;
    MAP_SOURCE[0] = DOORPIECE2;
    MAP_SOURCE += 128;
    MAP_SOURCE[0] = DOORPIECE3;
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 1) {
        platform->renderLiveMapTile(MAP, UNIT_LOC_X[UNIT], UNIT_LOC_Y[UNIT]);
    }
#endif
}

void DRAW_HORIZONTAL_DOOR()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_X--;
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = DOORPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE[1] = DOORPIECE2;
    MAP_SOURCE[2] = DOORPIECE3;
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    if (LIVE_MAP_ON == 1) {
        platform->renderLiveMapTile(MAP, UNIT_LOC_X[UNIT], UNIT_LOC_Y[UNIT]);
    }
#endif
}
uint8_t DOORPIECE1 = 0;
uint8_t DOORPIECE2 = 0;
uint8_t DOORPIECE3 = 0;

void ROBOT_ATTACK_RANGE()
{
    /*
    // This is the original code that does not quite work as intended
    // First check horizontal proximity to door
    int A = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]); // ROBOT UNIT, PLAYER UNIT
    if (A >= 1) { // 1 HORIZONTAL TILE FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // Now check vertical proximity
    A = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 1) { // 1 VERTICAL TILE FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // PLAYER DETECTED, CHANGE DOOR MODE.
    PROX_DETECT = 1;
    */

    int X = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]);
    int Y = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]);
    PROX_DETECT = (X == 1 && Y == 0) || (X == 0 && Y == 1) ? 1 : 0;
}

void DOOR_CHECK_PROXIMITY()
{
    // First check horizontal proximity to door
    int A = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 2) { // 2 HORIZONTAL TILES FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // Now check vertical proximity
    A = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 2) { // 2 VERTICAL TILES FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // PLAYER DETECTED, CHANGE DOOR MODE.
    PROX_DETECT = 1;
}
uint8_t PROX_DETECT = 0; // 0=NO 1=YES

// This routine handles automatic sliding doors.
// UNIT_B register means:
// 0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
void ELEVATOR()
{
    if (UNIT_B[UNIT] < 6) { // make sure number is in bounds
        ELDB[UNIT_B[UNIT]]();
    }
    // -SHOULD NEVER NEED TO HAPPEN
}
void (*ELDB[])(void) = {
    ELEV_OPEN_A,
    ELEV_OPEN_B,
    ELEV_OPEN_FULL,
    ELEV_CLOSE_A,
    ELEV_CLOSE_B,
    ELEV_CLOSE_FULL
};

void ELEV_OPEN_A()
{
    DOORPIECE1 = 181;
    DOORPIECE2 = 89;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 1;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_OPEN_B()
{
    DOORPIECE1 = 182;
    DOORPIECE2 = 9;
    DOORPIECE3 = 172;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 2;
    UNIT_TIMER_A[UNIT] = 50;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_OPEN_FULL()
{
    // CLOSE DOOR
    // check for object in the way first.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    GET_TILE_FROM_MAP();
    if (TILE != 9) { // FLOOR-TILE
        // SOMETHING IN THE WAY, ABORT
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    // check for player or robot in the way
    CHECK_FOR_UNIT();
    if (UNIT_FIND != 255) {
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    // START TO CLOSE ELEVATOR DOOR
    PLAY_SOUND(14); // DOOR SOUND SOUND PLAY
    DOORPIECE1 = 181;
    DOORPIECE2 = 89;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 3;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_CLOSE_A()
{
    DOORPIECE1 = 84;
    DOORPIECE2 = 85;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 4;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_CLOSE_B()
{
    DOORPIECE1 = 80;
    DOORPIECE2 = 81;
    DOORPIECE3 = 174;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 5;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
    ELEVATOR_PANEL();
}

void ELEV_CLOSE_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT == 0) {
        UNIT_TIMER_A[UNIT] = 20; // RESET TIMER
        return;
    }
    // Start open door process
    PLAY_SOUND(14); // DOOR SOUND SOUND PLAY
    DOORPIECE1 = 84;
    DOORPIECE2 = 85;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 0;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEVATOR_PANEL()
{
    // Check to see if player is standing in the
    // elevator first.
    if ((UNIT_LOC_X[UNIT] != UNIT_LOC_X[0]) || // elevator X location, player X location
        ((UNIT_LOC_Y[UNIT] - 1) != UNIT_LOC_Y[0])) { // elevator Y location, player Y location
        return;
    }
    // PLAYER DETECTED, START ELEVATOR PANEL
    PRINT_INFO(MSG_ELEVATOR);
    PRINT_INFO(MSG_LEVELS);
    ELEVATOR_SELECT();
}

void PLOT_TILE_TO_MAP()
{
    MAP_SOURCE = MAP + (MAP_Y << 7) + MAP_X;
    MAP_SOURCE[0] = TILE;
}

// This routine will return the tile for a specific X/Y
// on the map.  You must first define MAP_X and MAP-Y.
// The result is stored in TILE.
void GET_TILE_FROM_MAP()
{
    MAP_SOURCE = MAP + ((MAP_Y << 7) + MAP_X);
    TILE = MAP_SOURCE[0];
}

// In this AI routine, the droid simply goes left until it
// hits an object, and then reverses direction and does the
// same, bouncing back and forth.
void LEFT_RIGHT_DROID()
{
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 10; // reset timer to 10
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=LEFT 1=RIGHT
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_LEFT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_RIGHT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// In this AI routine, the droid simply goes UP until it
// hits an object, and then reverses direction and does the
// same, bouncing back and forth.
void UP_DOWN_DROID()
{
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 10; // reset timer to 10
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=UP 1=DOWN
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_UP();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        CHECK_FOR_WINDOW_REDRAW();
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_DOWN();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void HOVERBOT_ANIMATE(uint8_t X)
{
    if (UNIT_TIMER_B[X] != 0) {
        UNIT_TIMER_B[X]--;
    } else {
        UNIT_TIMER_B[X] = 3; // RESET ANIMATE TIMER
        if (UNIT_TILE[X] == 98) {
            UNIT_TILE[X] = 99; // HOVERBOT TILE
        } else {
            UNIT_TILE[X] = 98; // HOVERBOT TILE
        }
    }
}

// The following 4 routines are used by both the player and some
// of the enemy units.  It checks to see if you can walk in a
// specific direction.  It checks for edge of map and also that
// the tile you want to walk onto can allow that.  There is a
// separate routine for hovering robots.
void REQUEST_WALK_RIGHT()
{
    UNIT_DIRECTION[UNIT] = 3;
    if (UNIT_LOC_X[UNIT] != 122) {
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_X++;
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_X[UNIT]++;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_LEFT()
{
    UNIT_DIRECTION[UNIT] = 2;
    if (UNIT_LOC_X[UNIT] != 5) {
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_X--;
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_X[UNIT]--;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_DOWN()
{
    UNIT_DIRECTION[UNIT] = 1;
    if (UNIT_LOC_Y[UNIT] != 60) {
        MAP_Y = UNIT_LOC_Y[UNIT];
        MAP_Y++;
        MAP_X = UNIT_LOC_X[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_Y[UNIT]++;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_UP()
{
    UNIT_DIRECTION[UNIT] = 0;
    if (UNIT_LOC_Y[UNIT] != 3) {
        MAP_Y = UNIT_LOC_Y[UNIT];
        MAP_Y--;
        MAP_X = UNIT_LOC_X[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_Y[UNIT]--;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

// This routine checks a specific place on the map specified
// in MAP_X and MAP_Y to see if there is a unit present at 
// that spot. If so, the unit# will be stored in UNIT_FIND
// otherwise 255 will be stored. 
void CHECK_FOR_UNIT()
{
    for (int X = 0; X != 28; X++) {
        if (UNIT_TYPE[X] != 0 && UNIT_LOC_X[X] == MAP_X && UNIT_LOC_Y[X] == MAP_Y) {
            UNIT_FIND = X; // unit found
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

// This routine checks a specific place on the map specified
// in MAP_X and MAP_Y to see if there is a hidden unit present 
// at that spot. If so, the unit# will be stored in UNIT_FIND
// otherwise 255 will be stored. 
void CHECK_FOR_HIDDEN_UNIT()
{
    for (int X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0 &&
            (UNIT_LOC_X[X] == MAP_X || // first compare horizontal position
             (UNIT_LOC_X[X] <= MAP_X && (UNIT_LOC_X[X] + UNIT_C[X]) >= MAP_X)) && // add hidden unit width
            (UNIT_LOC_Y[X] == MAP_Y || // now compare vertical position
             (UNIT_LOC_Y[X] <= MAP_Y && (UNIT_LOC_Y[X] + UNIT_D[X]) >= MAP_Y))) { // add hidden unit HEIGHT
            UNIT_FIND = X;
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

#ifndef PLATFORM_IMAGE_SUPPORT
// These are the included binary files that contain the screen
// image for the main editor.
uint8_t INTRO_TEXT[] = {
    0x60, 0x20, 0x02, 0x4e, 0x60, 0x63, 0x0a, 0x4e, 0x65, 0x60, 0x20, 0x05, 0xe9, 0xce, 0x20, 0x20, 0xe9, 0xce, 0x60, 0x20,
    0x0d, 0xcd, 0x60, 0xa0, 0x09, 0xce, 0x20, 0x65, 0x60, 0x20, 0x05, 0x66, 0xa0, 0x20, 0x20, 0x66, 0xa0, 0x60, 0x20, 0x0d,
    0xa0, 0x13, 0x14, 0x01, 0x12, 0x14, 0x20, 0x07, 0x01, 0x0d, 0x05, 0xa0, 0x20, 0x65, 0x60, 0x20, 0x04, 0xe9, 0x66, 0xce,
    0xa0, 0xa0, 0x66, 0xce, 0xce, 0x60, 0x20, 0x0c, 0xa0, 0x13, 0x05, 0x0c, 0x05, 0x03, 0x14, 0x20, 0x0d, 0x01, 0x10, 0xa0,
    0x20, 0x65, 0x60, 0x20, 0x03, 0xe9, 0xa0, 0xe3, 0x60, 0xa0, 0x02, 0xe3, 0x60, 0xce, 0x02, 0x60, 0x20, 0x0b, 0xa0, 0x04,
    0x09, 0x06, 0x06, 0x09, 0x03, 0x15, 0x0c, 0x14, 0x19, 0xa0, 0x20, 0x65, 0x60, 0x20, 0x02, 0xe9, 0x60, 0x66, 0x06, 0xce,
    0xce, 0xa0, 0x60, 0x20, 0x0b, 0xa0, 0x03, 0x0f, 0x0e, 0x14, 0x12, 0x0f, 0x0c, 0x13, 0x20, 0x20, 0xa0, 0x20, 0x65, 0x60,
    0x20, 0x02, 0x66, 0x3a, 0x4d, 0x60, 0x3a, 0x02, 0x4e, 0x3a, 0x66, 0xa0, 0xa0, 0x60, 0x20, 0x02, 0xe9, 0xce, 0x20, 0x20,
    0xe9, 0xce, 0x60, 0x20, 0x02, 0xce, 0x60, 0xa0, 0x09, 0xcd, 0x4e, 0x60, 0x20, 0x03, 0x66, 0x55, 0x43, 0x4d, 0x3a, 0x4e,
    0x43, 0x49, 0x66, 0xa0, 0xa0, 0x60, 0x20, 0x02, 0x66, 0xa0, 0x20, 0x20, 0x66, 0xa0, 0x60, 0x20, 0x13, 0x66, 0x42, 0x51,
    0x48, 0x3a, 0x42, 0x51, 0x48, 0x66, 0xa0, 0x69, 0x60, 0x20, 0x02, 0x66, 0xa0, 0x20, 0x20, 0x66, 0xa0, 0x60, 0x20, 0x02,
    0x70, 0x60, 0x40, 0x02, 0x73, 0x0d, 0x01, 0x10, 0x6b, 0x60, 0x40, 0x02, 0x6e, 0x60, 0x20, 0x03, 0x66, 0x4a, 0x46, 0x4b,
    0x3a, 0x4a, 0x46, 0x4b, 0x66, 0xce, 0x60, 0x20, 0x03, 0x66, 0xce, 0xa0, 0xa0, 0x66, 0xa0, 0x20, 0x20, 0x0b, 0x09, 0x0c,
    0x0c, 0x20, 0x01, 0x0c, 0x0c, 0x20, 0x08, 0x15, 0x0d, 0x01, 0x0e, 0x13, 0x60, 0x20, 0x03, 0x60, 0x66, 0x06, 0xa0, 0xa0,
    0x60, 0x20, 0x03, 0x60, 0x66, 0x04, 0x69, 0x60, 0x20, 0x14, 0x66, 0x60, 0xd0, 0x04, 0x66, 0xa0, 0xa0, 0x60, 0x20, 0x05,
    0x66, 0xa0, 0x20, 0x20, 0x60, 0x43, 0x14, 0x66, 0x60, 0xd0, 0x04, 0x66, 0xa0, 0x69, 0x60, 0x43, 0x05, 0x66, 0xa0, 0x43,
    0x43, 0x60, 0x3a, 0x14, 0x60, 0x66, 0x06, 0xce, 0xa0, 0xa0, 0xce, 0x60, 0x3a, 0x03, 0x66, 0xa0, 0x60, 0x3a, 0x16, 0xe9,
    0xa0, 0xa0, 0xe7, 0xd0, 0xce, 0x60, 0xa0, 0x02, 0xce, 0xa0, 0x60, 0x3a, 0x03, 0x66, 0xa0, 0x60, 0x3a, 0x15, 0xe9, 0x60,
    0xa0, 0x03, 0xe3, 0x60, 0xa0, 0x02, 0xce, 0xa0, 0xa0, 0x60, 0x3a, 0x03, 0x66, 0xa0, 0x60, 0x3a, 0x0b, 0xe9, 0xce, 0xdf,
    0x60, 0x3a, 0x06, 0x60, 0x66, 0x08, 0xd5, 0xc0, 0xc9, 0x60, 0x3a, 0x03, 0x66, 0xce, 0xdf, 0x60, 0x3a, 0x09, 0xe9, 0xe3,
    0xcd, 0xce, 0x60, 0xa0, 0x06, 0x66, 0x51, 0x60, 0x66, 0x04, 0x51, 0x66, 0xdd, 0xce, 0xe3, 0x60, 0xa0, 0x02, 0xce, 0xa0,
    0xcd, 0xce, 0x60, 0x3a, 0x09, 0xa0, 0xd1, 0xe7, 0x60, 0x66, 0x10, 0xdd, 0x60, 0x66, 0x04, 0xa0, 0xd1, 0xe7, 0x69, 0x60,
    0x3a, 0x09, 0x5f, 0xa0, 0xce, 0x60, 0x3a, 0x07, 0x60, 0x66, 0x08, 0xca, 0xc0, 0xcb, 0x60, 0x3a, 0x02, 0x5f, 0xe4, 0x69,
    0x60, 0x3a, 0x0b, 0x66, 0xa0, 0x3a, 0xe9, 0xa0, 0xa0, 0xce, 0x3a, 0xe9, 0xa0, 0xa0, 0xce, 0xe9, 0xa0, 0xa0, 0xce, 0x66,
    0xe9, 0xa0, 0xa0, 0xce, 0xe9, 0xa0, 0xa0, 0xce, 0xe9, 0xa0, 0xa0, 0xce, 0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x60, 0x66,
    0x02, 0xce, 0xce, 0x60, 0x66, 0x02, 0xa0, 0x60, 0x66, 0x02, 0xce, 0xce, 0x60, 0x66, 0x02, 0xa0, 0x60, 0x66, 0x02, 0x69,
    0x60, 0x66, 0x02, 0x69, 0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x66, 0xce, 0xa0, 0x66, 0xce, 0x66, 0xa0, 0x66, 0xa0, 0x66,
    0xce, 0xa0, 0x66, 0xce, 0x66, 0xa0, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x3a, 0x66, 0xce, 0xa0, 0xce, 0x60, 0x3a, 0x0a, 0x66,
    0xa0, 0x3a, 0x60, 0x66, 0x02, 0xce, 0xce, 0x66, 0xa0, 0x66, 0xa0, 0x60, 0x66, 0x02, 0xce, 0xce, 0x66, 0xa0, 0x66, 0xa0,
    0x3a, 0x66, 0xa0, 0x3a, 0x60, 0x66, 0x02, 0xa0, 0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x66,
    0xce, 0x66, 0xa0, 0x66, 0xce, 0xa0, 0x66, 0x69, 0x66, 0xce, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x3a, 0xe9, 0xa0, 0x66, 0xa0,
    0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x66, 0x69, 0x3a, 0x66, 0x69, 0x60, 0x66, 0x02, 0x69, 0x60, 0x66, 0x02, 0x69, 0x3a,
    0x60, 0x66, 0x02, 0x69, 0x3a, 0x66, 0x69, 0x3a, 0x60, 0x66, 0x02, 0x69, 0x3a
};

uint8_t SCR_TEXT[] = {
    0x60, 0x20, 0x20, 0x5d, 0x17, 0x05, 0x01, 0x10, 0x0f, 0x0e, 0x60, 0x20, 0x20, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20,
    0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x6b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20,
    0x5d, 0x20, 0x09, 0x14, 0x05, 0x0d, 0x60, 0x20, 0x21, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20,
    0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x6b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20, 0x5d, 0x20, 0x0b, 0x05,
    0x19, 0x13, 0x60, 0x20, 0x21, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x6b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20,
    0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x05, 0x73, 0x09, 0x0e, 0x06, 0x0f, 0x12, 0x0d, 0x01,
    0x14, 0x09, 0x0f, 0x0e, 0x6b, 0x60, 0x40, 0x13, 0x5b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20, 0x5d, 0x08, 0x05, 0x01, 0x0c,
    0x14, 0x08, 0x60, 0x20, 0x20, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x71, 0x05
};

uint8_t SCR_ENDGAME[] = {
    0x55, 0x60, 0x40, 0x03, 0x73, 0x01, 0x14, 0x14, 0x01, 0x03, 0x0B, 0x20, 0x0F, 0x06, 0x20,
    0x14, 0x08, 0x05, 0x20, 0x10, 0x05, 0x14, 0x13, 0x03, 0x09, 0x09, 0x20, 0x12, 0x0F, 0x02,
    0x0F, 0x14, 0x13, 0x6B, 0x60, 0x40, 0x03, 0x49, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x0A, 0x13, 0x03, 0x05,
    0x0E, 0x01, 0x12, 0x09, 0x0F, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x5D, 0x60, 0x20, 0x06, 0x05, 0x0C, 0x01, 0x10, 0x13, 0x05, 0x04, 0x20, 0x14, 0x09, 0x0D,
    0x05, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x02,
    0x12, 0x0F, 0x02, 0x0F, 0x14, 0x13, 0x20, 0x12, 0x05, 0x0D, 0x01, 0x09, 0x0E, 0x09, 0x0E,
    0x07, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x20, 0x20, 0x13,
    0x05, 0x03, 0x12, 0x05, 0x14, 0x13, 0x20, 0x12, 0x05, 0x0D, 0x01, 0x09, 0x0E, 0x09, 0x0E,
    0x07, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x08,
    0x04, 0x09, 0x06, 0x06, 0x09, 0x03, 0x15, 0x0C, 0x14, 0x19, 0x3A, 0x60, 0x20, 0x11, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x4A, 0x60, 0x40, 0x25, 0x4B
};
#endif

uint8_t SCR_CUSTOM_KEYS[] = {
    0x55, 0x60, 0x40, 0x03, 0x73, 0x01, 0x14, 0x14, 0x01, 0x03, 0x0B, 0x20, 0x0F, 0x06, 0x20,
    0x14, 0x08, 0x05, 0x20, 0x10, 0x05, 0x14, 0x13, 0x03, 0x09, 0x09, 0x20, 0x12, 0x0F, 0x02,
    0x0F, 0x14, 0x13, 0x6B, 0x60, 0x40, 0x03, 0x49, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x10, 0x12, 0x05, 0x13, 0x13, 0x20, 0x14, 0x08,
    0x05, 0x20, 0x0B, 0x05, 0x19, 0x13, 0x20, 0x19, 0x0F, 0x15, 0x20, 0x17, 0x09, 0x13, 0x08,
    0x20, 0x14, 0x0F, 0x20, 0x15, 0x13, 0x05, 0x60, 0x20, 0x03, 0x5D, 0x5D, 0x60, 0x20, 0x04,
    0x06, 0x0F, 0x12, 0x20, 0x14, 0x08, 0x05, 0x20, 0x06, 0x0F, 0x0C, 0x0C, 0x0F, 0x17, 0x09,
    0x0E, 0x07, 0x20, 0x06, 0x15, 0x0E, 0x03, 0x14, 0x09, 0x0F, 0x0E, 0x13, 0x60, 0x20, 0x05,
    0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25,
    0x5D, 0x5D, 0x60, 0x20, 0x06, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x15, 0x10, 0x3A, 0x60, 0x20,
    0x16, 0x5D, 0x5D, 0x60, 0x20, 0x04, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x04, 0x0F, 0x17, 0x0E,
    0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x04, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x0C,
    0x05, 0x06, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x0D, 0x0F, 0x16,
    0x05, 0x20, 0x12, 0x09, 0x07, 0x08, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20,
    0x06, 0x06, 0x09, 0x12, 0x05, 0x20, 0x15, 0x10, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60,
    0x20, 0x04, 0x06, 0x09, 0x12, 0x05, 0x20, 0x04, 0x0F, 0x17, 0x0E, 0x3A, 0x60, 0x20, 0x16,
    0x5D, 0x5D, 0x60, 0x20, 0x04, 0x06, 0x09, 0x12, 0x05, 0x20, 0x0C, 0x05, 0x06, 0x14, 0x3A,
    0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x06, 0x09, 0x12, 0x05, 0x20, 0x12, 0x09,
    0x07, 0x08, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x20, 0x03, 0x19, 0x03, 0x0C, 0x05,
    0x20, 0x17, 0x05, 0x01, 0x10, 0x0F, 0x0E, 0x13, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60,
    0x20, 0x02, 0x03, 0x19, 0x03, 0x0C, 0x05, 0x20, 0x09, 0x14, 0x05, 0x0D, 0x13, 0x3A, 0x60,
    0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x05, 0x15, 0x13, 0x05, 0x20, 0x09, 0x14, 0x05, 0x0D,
    0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x20, 0x13, 0x05, 0x01, 0x12, 0x03, 0x08, 0x20, 0x0F,
    0x02, 0x0A, 0x05, 0x03, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x02, 0x0D,
    0x0F, 0x16, 0x05, 0x20, 0x0F, 0x02, 0x0A, 0x05, 0x03, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x4A, 0x60, 0x40, 0x25, 0x4B
};

char CINEMA_MESSAGE[] = 
    "coming soon: space balls 2 - the search for more money, "
    "attack of the paperclips: clippy's revenge, "
    "it came from planet earth, "
    "rocky 5000, all my circuits the movie, "
    "conan the librarian, and more! comin";

#ifndef PLATFORM_IMAGE_SUPPORT
uint8_t WEAPON1A[] = {
    0x2c, 0x20, 0x20, 0x20, 0x20, 0x2c
};

uint8_t WEAPON1B[] = {
    0xe2, 0xf9, 0xef, 0xe4, 0x66, 0x66
};

uint8_t WEAPON1C[] = {
    0x20, 0x20, 0x20, 0x20, 0x5f, 0xdf
};

uint8_t WEAPON1D[] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t PISTOL1A[] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t PISTOL1B[] = {
    0x20, 0x68, 0x62, 0x62, 0x62, 0x20
};

uint8_t PISTOL1C[] = {
    0x20, 0x20, 0x20, 0x5f, 0xdf, 0x20
};

uint8_t PISTOL1D[] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t TBOMB1A[] = {
    0x20, 0x20, 0x55, 0x2a, 0x20, 0x20
};

uint8_t TBOMB1B[] = {
    0x20, 0x55, 0x66, 0x49, 0x20, 0x20
};

uint8_t TBOMB1C[] = {
    0x20, 0x42, 0x20, 0x48, 0x20, 0x20
};

uint8_t TBOMB1D[] = {
    0x20, 0x4a, 0x46, 0x4b, 0x20, 0x20
};

uint8_t EMP1A[] = {
    0x20, 0x55, 0x43, 0x43, 0x49, 0x20
};

uint8_t EMP1B[] = {
    0x66, 0xdf, 0x55, 0x49, 0xe9, 0x66
};

uint8_t EMP1C[] = {
    0x66, 0x69, 0x4a, 0x4b, 0x5f, 0x66
};

uint8_t EMP1D[] = {
    0x20, 0x4a, 0x46, 0x46, 0x4b, 0x20
};

uint8_t MAG1A[] = {
    0x4d, 0x70, 0x6e, 0x70, 0x6e, 0x4e
};

uint8_t MAG1B[] = {
    0x20, 0x42, 0x42, 0x48, 0x48, 0x20
};

uint8_t MAG1C[] = {
    0x63, 0x42, 0x4a, 0x4b, 0x48, 0x63
};

uint8_t MAG1D[] = {
    0x4e, 0x4a, 0x46, 0x46, 0x4b, 0x4d
};

uint8_t MED1A[] = {
    0x20, 0x55, 0x43, 0x43, 0x49, 0x20
};

uint8_t MED1B[] = {
    0x20, 0xA0, 0xA0, 0xA0, 0xA0, 0x20
};

uint8_t MED1C[] = {
    0x20, 0xA0, 0xEB, 0xF3, 0xA0, 0x20
};

uint8_t MED1D[] = {
    0x20, 0xE4, 0xE4, 0xE4, 0xE4, 0x20
};
#endif

#ifndef PLATFORM_MODULE_BASED_AUDIO
uint8_t NOTE_FREQ[] = {
    0,  // placeholder for zero
    251, // Note 01    B   (Lowest note the PET can produce)
    // octave 4
    238, // Note 02    C
    224, // Note 03    C#
    210, // Note 04    D
    199, // Note 05    D#
    188, // Note 06    E
    177, // Note 07    F
    168, // Note 08    F#
    158, // Note 09    G
    149, // Note 10    G#
    140, // Note 11    A
    133, // Note 12    A#
    251, // Note 13    B
    // octave 5
    238, // Note 14    C
    224, // Note 15    C#
    210, // Note 16    D
    199, // Note 17    D#
    188, // Note 18    E
    177, // Note 19    F
    168, // Note 20    F#
    158, // Note 21    G
    149, // Note 22    G#
    140, // Note 23    A
    133, // Note 24    A#
    251, // Note 25    B
    // octave 6
    238, // Note 26    C
    224, // Note 27    C#
    210, // Note 28    D
    199, // Note 29    D#
    188, // Note 30    E
    177, // Note 31    F
    168, // Note 32    F#
    158, // Note 33    G
    149, // Note 34    G#
    140, // Note 35    A
    133 // Note 36    A#  (Highest note the PET can produce)
};

uint8_t NOTE_OCTAVE[] = {
    00,  // placeholder for zero
    15,  // Note 01    B   (Lowest note the PET can produce)
    // octave 4
    15,  // Note 02    C
    15,  // Note 03    C#
    15,  // Note 04    D
    15,  // Note 05    D#
    15,  // Note 06    E
    15,  // Note 07    F
    15,  // Note 08    F#
    15,  // Note 09    G
    15,  // Note 10    G#
    15,  // Note 11    A
    15,  // Note 12    A#
    51,  // Note 13    B
    // octave 5
    51,  // Note 14    C
    51,  // Note 15    C#
    51,  // Note 16    D
    51,  // Note 17    D#
    51,  // Note 18    E
    51,  // Note 19    F
    51,  // Note 20    F#
    51,  // Note 21    G
    51,  // Note 22    G#
    51,  // Note 23    A
    51,  // Note 24    A#
    85,  // Note 25    B
    // octave 6
    85,  // Note 26    C
    85,  // Note 27    C#
    85,  // Note 28    D
    85,  // Note 29    D#
    85,  // Note 30    E
    85,  // Note 31    F
    85,  // Note 32    F#
    85,  // Note 33    G
    85,  // Note 34    G#
    85,  // Note 35    A
    85  // Note 36    A#  (Highest note the PET can produce)
};

uint8_t SND_EXPLOSION[] = {
    0x27, 0x0B, 0x0D, 0x07, 0x08, 0x0B, 0x03, 0x07, 0x03, 0x05, 0x06, 0x0C, 0x0E, 0x02, 0x01,
    0x06, 0x08, 0x0D, 0x0B, 0x01, 0x05, 0x06, 0x07, 0x0A, 0x0A, 0x03, 0x06, 0x25
};

uint8_t SND_MEDKIT[] = {
    0x29, 0x09, 0x04, 0x01, 0x25
};

uint8_t SND_EMP[] = {
    0x27, 0x02, 0x07, 0x0C, 0x11, 0x16, 0x1B, 0x20, 0x1B, 0x16, 0x11, 0x0C, 0x07, 0x02, 0x25
};

uint8_t SND_MAGNET[] = {
    0x2A, 0x17, 0x0D, 0x12, 0x23, 0x0F, 0x15, 0x09, 0x17, 0x03, 0x25
};

uint8_t SND_SHOCK[] = {
    0x27, 0x26, 0x0E, 0x26, 0x0E, 0x26, 0x0E, 0x26, 0x0E, 0x26, 0x04,
    0x26, 0x04, 0x26, 0x04, 0x26, 0x04, 0x25
};

uint8_t SND_MOVE_OBJ[] = {
    0x28, 0x02, 0x26, 0x0E, 0x26, 0x25
};

uint8_t SND_PLASMA[] = {
    0x27, 0x07, 0x13, 0x06, 0x12, 0x07, 0x13, 0x06, 0x12,
    0x07, 0x13, 0x06, 0x12, 0x07, 0x13, 0x25 
};

uint8_t SND_PISTOL[] = {
    0x27, 0x24, 0x23, 0x22, 0x21, 0x20, 0x1F, 0x25
};

uint8_t SND_ITEM_FOUND[] = {
    0x28, 0x09, 0x15, 0x0B, 0x17, 0x0D, 0x19,
    0x0E, 0x02, 0x0E, 0x02, 0x0E, 0x25
};

uint8_t SND_ERROR[] = {
    0x27, 0x01, 0x26, 0x01, 0x26, 0x01, 0x26, 0x01, 0x25
};

uint8_t SND_CYCLE_WEAPON[] = {
    0x27, 0x15, 0x13, 0x11, 0x0F, 0x0D, 0x15, 0x25
};

uint8_t SND_CYCLE_ITEM[] = {
    0x27, 0x0D, 0x0C, 0x0B, 0x0A, 0x0B, 0x0C, 0x0D, 0x25
};

uint8_t SND_DOOR[] = {
    0x28, 0x0B, 0x0D, 0x0E, 0x10, 0x12, 0x13, 0x15, 0x25
};

uint8_t SND_MENU_BEEP[] = {
    0x28, 0x0E, 0x04, 0x25
};

uint8_t SND_SHORT_BEEP[] = {
    0x28, 0x11, 0x25
};

uint8_t INTRO_MUSIC[] = {
    0x2D, 0x06, 0x12, 0x0F, 0x03, 0x0F, 0x0D, 0x08, 0x12, 0x03, 0x06, 0x08, 0x0A, 0x03, 0x12,
    0x03, 0x09, 0x2D, 0x03, 0x00, 0x01, 0x01, 0x03, 0x26, 0x06, 0x26, 0x00, 0x00, 0x01, 0x26,
    0x03, 0x01, 0x00, 0x00, 0x03, 0x00, 0x01, 0x01, 0x03, 0x26, 0x06, 0x26, 0x00, 0x00, 0x01,
    0x26, 0x08, 0x06, 0x00, 0x00, 0x03, 0x00, 0x01, 0x01, 0x03, 0x26, 0x06, 0x26, 0x00, 0x00,
    0x01, 0x26, 0x03, 0x01, 0x00, 0x00, 0x08, 0x0A, 0x08, 0x00, 0x06, 0x00, 0x03, 0x01, 0x03,
    0x26, 0x06, 0x26, 0x03, 0x26, 0x00, 0x00, 0x0F, 0x00, 0x0D, 0x0F, 0x26, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x0D, 0x0F, 0x12, 0x00, 0x0F, 0x26, 0x0F, 0x00, 0x0D, 0x0F, 0x26, 0x00, 0x12,
    0x00, 0x00, 0x00, 0x0D, 0x0F, 0x14, 0x00, 0x12, 0x00, 0x0F, 0x00, 0x0D, 0x0F, 0x26, 0x00,
    0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x14, 0x00, 0x0F, 0x00, 0x14, 0x16, 0x14, 0x00, 0x12,
    0x00, 0x0F, 0x0D, 0x0F, 0x26, 0x12, 0x26, 0x0F, 0x26, 0x00, 0x00, 0x0F, 0x00, 0x0D, 0x0F,
    0x26, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x12, 0x26, 0x0F, 0x26, 0x0F, 0x00, 0x0D,
    0x0F, 0x26, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x12, 0x00, 0x14, 0x00, 0x0F, 0x00,
    0x0D, 0x0F, 0x26, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x12, 0x00, 0x0F, 0x00, 0x14,
    0x16, 0x14, 0x00, 0x12, 0x14, 0x12, 0x00, 0x0F, 0x12, 0x0D, 0x00, 0x0F, 0x08, 0x0A, 0x0B,
    0x0F, 0x04, 0x01, 0x04, 0x0D, 0x04, 0x01, 0x04, 0x0F, 0x04, 0x01, 0x04, 0x12, 0x04, 0x01,
    0x04, 0x0D, 0x04, 0x0F, 0x04, 0x12, 0x04, 0x01, 0x04, 0x0D, 0x04, 0x01, 0x04, 0x0B, 0x0A,
    0x08, 0x06, 0x08, 0x04, 0x01, 0x04, 0x0F, 0x04, 0x01, 0x04, 0x0D, 0x04, 0x01, 0x04, 0x0B,
    0x00
};

uint8_t WIN_MUSIC[] = {
    0x2C, 0x07, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x10, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x10, 0x00,
    0x11, 0x00, 0x00, 0x10, 0x00, 0x00, 0x11, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25,
};

uint8_t LOSE_MUSIC[] = {
    0x2F, 0x0D, 0x00, 0x0B, 0x00, 0x09, 0x00, 0x08, 0x00, 0x06, 0x00, 0x02, 0x00, 0x00, 0x04,
    0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25
};

uint8_t IN_GAME_MUSIC1[] = {
    0x30, 0x01, 0x03, 0x04, 0x06, 0x08, 0x00, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26,
    0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26, 0x04, 0x03, 0x04, 0x26, 0x09, 0x26, 0x08,
    0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06, 0x08, 0x00, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26,
    0x09, 0x26, 0x06, 0x04, 0x06, 0x26, 0x0F, 0x26, 0x0B, 0x26, 0x09, 0x08, 0x09, 0x00, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x14, 0x00, 0x15, 0x00, 0x0B, 0x09, 0x08, 0x00,
    0x12, 0x00, 0x14, 0x00, 0x09, 0x08, 0x06, 0x26, 0x12, 0x00, 0x14, 0x00, 0x02, 0x04, 0x08,
    0x09, 0x08, 0x06, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x14, 0x00, 0x15, 0x00, 0x0B, 0x09,
    0x08, 0x00, 0x12, 0x00, 0x14, 0x00, 0x09, 0x08, 0x06, 0x00, 0x12, 0x00, 0x14, 0x00, 0x02,
    0x06, 0x0D, 0x00, 0x0B, 0x09, 0x0B, 0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06, 0x08, 0x00,
    0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09,
    0x26, 0x04, 0x03, 0x04, 0x26, 0x09, 0x26, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06,
    0x08, 0x00, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26, 0x06, 0x04, 0x06, 0x26, 0x0F,
    0x26, 0x0B, 0x26, 0x09, 0x08, 0x09, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x26, 0x08, 0x00,
    0x13, 0x00, 0x14, 0x00, 0x16, 0x14, 0x13, 0x00, 0x05, 0x00, 0x07, 0x00, 0x14, 0x13, 0x11,
    0x00, 0x05, 0x00, 0x07, 0x00, 0x01, 0x03, 0x07, 0x08, 0x07, 0x05, 0x07, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x13, 0x00, 0x14, 0x00, 0x0A, 0x08, 0x07, 0x00, 0x11, 0x00, 0x13, 0x00, 0x08,
    0x07, 0x05, 0x00, 0x11, 0x00, 0x13, 0x00, 0x03, 0x04, 0x09, 0x00, 0x06, 0x00, 0x08, 0x00,
    0x00
};

uint8_t IN_GAME_MUSIC2[] = {
    0x2E, 0x03, 0x00, 0x26, 0x03, 0x0F, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x0D,
    0x02, 0x0E, 0x03, 0x00, 0x26, 0x03, 0x0F, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0F, 0x00, 0x01,
    0x0D, 0x02, 0x0E, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12,
    0x00, 0x0F, 0x26, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x14,
    0x00, 0x0F, 0x26, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12,
    0x00, 0x0F, 0x00, 0x0D, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x30, 0x00, 0x00, 0x26, 0x00,
    0x00, 0x2E, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12, 0x00,
    0x0F, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x14, 0x00,
    0x0F, 0x26, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12, 0x00,
    0x0F, 0x00, 0x0D, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x0D,
    0x00, 0x0C, 0x26, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x30, 0x00, 0x00, 0x00, 0x26,
    0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x11, 0x30, 0x00, 0x00, 0x00,
    0x26, 0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x13, 0x00, 0x11, 0x00, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x30, 0x00,
    0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x11, 0x30,
    0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x13,
    0x00, 0x00, 0x00, 0x14, 0x00, 0x13, 0x00, 0x11, 0x00, 0x0F, 0x00, 0x0E, 0x00, 0x0D, 0x2E,
    0x00
};

uint8_t IN_GAME_MUSIC3[] = {
    0x2C, 0x02, 0x00, 0x09, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x02, 0x00,
    0x09, 0x00, 0x0C, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x09, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0E,
    0x00, 0x09, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x09, 0x00,
    0x0C, 0x00, 0x09, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x0C, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x00,
    0x00, 0x09, 0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x0C, 0x00, 0x09,
    0x00, 0x26, 0x00, 0x09, 0x00, 0x26, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x10, 0x00,
    0x11, 0x00, 0x10, 0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x0C, 0x00,
    0x00, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0C,
    0x00, 0x00, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x05,
    0x00, 0x07, 0x00, 0x09, 0x00, 0x07, 0x00, 0x0B, 0x00, 0x2F, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x2C, 0x0E, 0x00, 0x0C, 0x00, 0x26, 0x00, 0x0C, 0x00, 0x26, 0x00, 0x0E, 0x00, 0x26, 0x00,
    0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0B, 0x00, 0x10, 0x00, 0x26, 0x00, 0x10,
    0x00, 0x12, 0x00, 0x26, 0x00, 0x17, 0x00, 0x26, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};
#endif

void convertToPETSCII(char* string)
{
    for (char* c = string; *c; c++) {
        if (*c >= 96) {
            *c -= 96;
        }
    }
}

void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    SCREEN_MEMORY[address] = value;
#ifdef PLATFORM_COLOR_SUPPORT
    platform->writeToScreenMemory(address, value, color, yOffset);
#else
    platform->writeToScreenMemory(address, value);
#endif
}

// NOTES ABOUT UNIT TYPES
// ----------------------
// 000=no unit (does not exist)
// 001=player unit
// 002=hoverbot left-to-right
// 003=hoverbot up-down
// 004=hoverbot attack mode
// 005=hoverbot chase player
// 006=
// 007=transporter
// 008=
// 009=evilbot chase player
// 010=door
// 011=small explosion
// 012=pistol fire up
// 013=pistol fire down
// 014=pistol fire left
// 015=pistol fire right
// 016=trash compactor
// 017=
// 018=
// 019=
// 020=

// NOTES ABOUT UNIT NUMBERING SCHEME
// ---------------------------------
// 0 = player unit
// 1-27 = enemy robots    (max 28 units)
// 28-31 = weapons fire
// 32-47 = doors and other units that don't have sprites (max 16 units)
// 48-63 = hidden objects to be found (max 16 units)

// NOTES ABOUT DOORS.
// -------------------
// A-0=horitzonal 1=vertical
// B-0=opening-A 1=opening-B 2=OPEN / 3=closing-A 4=closing-B 5-CLOSED
// C-0=unlocked / 1=locked spade 2=locked heart 3=locked star
// D-0=automatic / 0=manual

// HIDDEN OBJECTS
// --------------
// UNIT_TYPE:128=key UNIT_A: 0=SPADE 1=HEART 2=STAR
// UNIT_TYPE:129=time bomb
// UNIT_TYPE:130=EMP
// UNIT_TYPE:131=pistol
// UNIT_TYPE:132=charged plasma gun
// UNIT_TYPE:133=medkit
// UNIT_TYPE:134=magnet

// NOTES ABOUT TRANSPORTER
// ----------------------
// UNIT_A: 0=always active    1=only active when all robots are dead
// UNIT_B:    0=completes level 1=send to coordinates
// UNIT_C:    X-coordinate
// UNIT_D:    Y-coordinate

// Sound Effects
// ----------------------
// 0 explosion
// 1 small explosion
// 2 medkit
// 3 emp
// 4 haywire
// 5 evilbot
// 6 move
// 7 electric shock
// 8 plasma gun
// 9 fire pistol
// 10 item found
// 11 error
// 12 change weapons
// 13 change items
// 14 door
// 15 menu beep
// 16 walk
// 17 sfx (short beep)
// 18 sfx
