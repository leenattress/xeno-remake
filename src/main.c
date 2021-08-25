#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "dma.h"

#define RETICULE_READY      0
#define RETICULE_CHARGING   1

#define RETICULE_ACCEL      FIX32(1L)
#define RETICULE_SPEED_MAX  FIX32(5L)
#define RETICULE_BRAKING    FIX32(1L)

#define BALL_SPEED_MAX      FIX32(16L)

#define BALL_FRICTION       FIX32(6L)
#define PUCK_FRICTION       FIX32(2L)

#define MAP_TILE_SIZE       8
#define MAP_WIDTH           1360
#define MAP_HEIGHT          384
#define MAP_WIDTH_TILES     (MAP_WIDTH / MAP_TILE_SIZE)
#define MAP_HEIGHT_TILES    (MAP_HEIGHT / MAP_TILE_SIZE)

#define MIN_POSX            FIX32(32L)
#define MAX_POSX            FIX32(MAP_WIDTH - 32)
#define MIN_POSY            FIX32(32L)
#define MAX_POSY            FIX32(MAP_HEIGHT - 32)


// forward
static void handleInput();
static void joyEvent(u16 joy, u16 changed, u16 state);

static void setSpritePosition(Sprite* sprite, s16 posX, s16 posY);

static void updatePhysic();
static void updateAnim();

static void updateCameraPosition();
static void setCameraPosition(s16 x, s16 y);

static void updateMap(VDPPlane plane, Map* map, s16 xt, s16 yt);
static void updateVDPScroll();

//static void frameChanged(Sprite* sprite);

// used for alternate map update mode
u16 tilemapBuf[MAP_WIDTH_TILES * MAP_HEIGHT_TILES];
u16 bufOffset;

// sprites
Sprite* reticuleA;
// Sprite* reticule2;
// Sprite* puck1;
// Sprite* puck2;
// Sprite* ball;

// maps (BGA and BGB) position (tile) for alternate method
s16 mapMetaTilePosX[2];
s16 mapMetaTilePosY[2];
// maps (BGA and BGB)
Map *arenaMap;

// absolute camera position (pixel)
s16 camPosX;
s16 camPosY;
// require scroll update
bool scrollNeedUpdate;

// position and movement variables
fix32 posX;
fix32 posY;
fix32 movX;
fix32 movY;
s16 xOrder;
s16 yOrder;

// maintain X button to use alternate MAP update mode
bool alternateScrollMethod;
bool paused;

// animation index table for enemies (static VRAM loading)
u16** sprTileIndexes[2];
// BG start tile index
u16 bgBaseTileIndex[2];

#include "camera.h"
#include "input.h"
#include "map.h"
#include "reticule.h"
#include "sprites.h"

int main(u16 hard)
{
    u16 palette[64];
    u16 ind;
    //u16 numTile;

    // initialization
    VDP_setScreenWidth320();

    // init sprite engine with default parameters
    SPR_init();

    // set all palette to black
    VDP_setPaletteColors(0, (u16*) palette_black, 64);

    // load background tilesets in VRAM
    ind = TILE_USERINDEX;
    bgBaseTileIndex[0] = ind;
    VDP_loadTileSet(&arena_tileset, ind, DMA);
    ind += arena_tileset.numTile;

    // initialize variables
    bufOffset = 0;

    alternateScrollMethod = FALSE;          // by default we use the easy MAP_scrollTo(..) method
    paused = FALSE;

    // BGB/BGA tile position (force refresh)
    mapMetaTilePosX[0] = -42;
    mapMetaTilePosY[0] = 0;
    mapMetaTilePosX[1] = -42;
    mapMetaTilePosY[1] = 0;
    // camera position (force refresh)
    camPosX = -1;
    camPosY = -1;
    scrollNeedUpdate = FALSE;

    // set main sprite position (camera position may be adjusted depending it)
    posX = FIX32(32L);
    posY = FIX32(32L);
    movX = FIX32(0);
    movY = FIX32(0);
    xOrder = 0;
    yOrder = 0;

    // init backgrounds
    arenaMap = MAP_create(&arena_map, BG_A, TILE_ATTR_FULL(0, FALSE, FALSE, FALSE, bgBaseTileIndex[0]));

    // need to increase a bit DMA buffer size to init both plan tilemap
    DMA_setBufferSize(9000);
    // init scrolling
    updateCameraPosition();

    if (scrollNeedUpdate)
    {
        updateVDPScroll();
        scrollNeedUpdate = FALSE;
    }

    // update camera position
    SYS_doVBlankProcess();
    // reset tilemap buffer position after update
    bufOffset = 0;

    // can restore default DMA buffer size
    DMA_setBufferSizeToDefault();

    // init sonic sprite
    reticuleA = SPR_addSprite(&reticule, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));

    SPR_update();

    // prepare palettes
    memcpy(&palette[0], palette_arena.data, 16);
    memcpy(&palette[16], palette_players.data, 16);

    // fade in
    PAL_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    JOY_setEventHandler(joyEvent);

    // just to monitor frame CPU usage
    SYS_showFrameLoad(TRUE);

    while(TRUE)
    {
        handleInput();

        if (!paused)
        {
            // update internal sprite position
            updatePhysic();
            updateAnim();
        }

        // update sprites
        SPR_update();

        // sync frame and do vblank process
        SYS_doVBlankProcess();
        // reset tilemap buffer position after update
        bufOffset = 0;

        // needed only for alternate MAP update method
        if (scrollNeedUpdate)
        {
            updateVDPScroll();
            scrollNeedUpdate = FALSE;
        }

        // KLog_U1("CPU usage = ", SYS_getCPULoad());
    }

    // release maps
    MEM_free(arenaMap);

    return 0;
}

static void updatePhysic()
{

    // update position of reticule
    handleReticuleMovement();

    // update camera position (*after* player sprite position has been updated)
    updateCameraPosition();

    // set sprites position
    setSpritePosition(reticuleA, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY);
}

