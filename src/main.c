#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "dma.h"

#include "vars.h"
#include "utils.h"
#include "entities.h"

// forward
static void handleInput();
static void joyEvent(u16 joy, u16 changed, u16 state);
static void setSpritePosition(Sprite *sprite, s16 posX, s16 posY);
static void updatePhysic();
static void updateAnim();
static void updateCameraPosition();
static void setCameraPosition(s16 x, s16 y);
static void updateMap(VDPPlane plane, Map *map, s16 xt, s16 yt);
static void updateVDPScroll();
//static void frameChanged(Sprite* sprite);

// used for alternate map update mode
u16 tilemapBuf[MAP_WIDTH_TILES * MAP_HEIGHT_TILES];
u16 bufOffset;

// sprites
// Sprite *reticuleASprite;
// Sprite *reticuleBSprite;
// Sprite *puckASprite;
// Sprite *puckBSprite;
// Sprite* ball;

int gamestate = GS_GAME;
int i;

// actors
// two reticules, two pucks, one ball
Entity actorsReticuleA;
Entity actorsReticuleB;
Entity actorsPuckA;
Entity actorsPuckB;
// Entity actorsBall;

//actorsBall = {0, 0, 32, 32, 0, 0, 0, 1, SLEEPING, ENT_TYPE_PUCK, CTRL_NONE, FALSE, "PLAYER"};
//Entity reticuleB = {0, 0, 32, 32, 0, 0, 0, 1, SLEEPING, ENT_TYPE_PUCK, CTRL_NONE, FALSE, "PLAYER"};

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

bool alternateScrollMethod;
bool paused;

// animation index table for enemies (static VRAM loading)
u16 **sprTileIndexes[2];
// BG start tile index
u16 bgBaseTileIndex[2];

#include "camera.h"
#include "input.h"
#include "map.h"
#include "reticule.h"
#include "sprites.h"

void init_actors()
{
    // ball
    // actorsBall.x = 0;
    // actorsBall.y = 0;
    // actorsBall.w = 32;
    // actorsBall.h = 32;
    // actorsBall.active = ACTIVE;
    // actorsBall.sprite = SPR_addSprite(
    //     &ballSprite,
    //     actorsBall.x >> BITSHIFT,
    //     actorsBall.y >> BITSHIFT,
    //     TILE_ATTR(PAL1, 0, FALSE, FALSE));

    // reticule A
    actorsReticuleA.x = 0;
    actorsReticuleA.y = 0;
    actorsReticuleA.w = 32;
    actorsReticuleA.h = 32;
    actorsReticuleA.active = ACTIVE;
    actorsReticuleA.sprite = SPR_addSprite(
        &reticule_a,
        actorsReticuleA.x >> BITSHIFT,
        actorsReticuleA.y >> BITSHIFT,
        TILE_ATTR(PAL1, 0, FALSE, FALSE));

    // reticule B
    actorsReticuleB.x = 0;
    actorsReticuleB.y = 0;
    actorsReticuleB.w = 32;
    actorsReticuleB.h = 32;
    actorsReticuleB.active = ACTIVE;
    actorsReticuleB.sprite = SPR_addSprite(
        &reticule_b,
        actorsReticuleB.x >> BITSHIFT,
        actorsReticuleB.y >> BITSHIFT,
        TILE_ATTR(PAL1, 0, FALSE, FALSE));

    // puck A
    actorsPuckA.x = 0;
    actorsPuckA.y = 0;
    actorsPuckA.w = 32;
    actorsPuckA.h = 32;
    actorsPuckA.active = ACTIVE;
    actorsPuckA.sprite = SPR_addSprite(
        &puck_a,
        actorsPuckA.x >> BITSHIFT,
        actorsPuckA.y >> BITSHIFT,
        TILE_ATTR(PAL1, 0, FALSE, FALSE));

    // puck B
    actorsPuckB.x = 0;
    actorsPuckB.y = 0;
    actorsPuckB.w = 32;
    actorsPuckB.h = 32;
    actorsPuckB.active = ACTIVE;
    actorsPuckB.sprite = SPR_addSprite(
        &puck_b,
        actorsPuckB.x >> BITSHIFT,
        actorsPuckB.y >> BITSHIFT,
        TILE_ATTR(PAL1, 0, FALSE, FALSE));
}

int main(u16 hard)
{
    u16 palette[64];
    u16 ind;
    //u16 numTile;

    // initialization
    VDP_setScreenWidth320();
    VDP_setScreenHeight240();

    // init sprite engine with default parameters
    SPR_init();

    // set all palette to black
    VDP_setPaletteColors(0, (u16 *)palette_black, 64);

    // load background tilesets in VRAM
    ind = TILE_USERINDEX;
    bgBaseTileIndex[0] = ind;
    VDP_loadTileSet(&arena_tileset, ind, DMA);
    ind += arena_tileset.numTile;

    // initialize variables
    bufOffset = 0;

    alternateScrollMethod = FALSE; // by default we use the easy MAP_scrollTo(..) method
    paused = FALSE;

    // BGB/BGA tile position (force refresh)
    mapMetaTilePosX[0] = -42;
    mapMetaTilePosY[0] = 0;
    // mapMetaTilePosX[1] = -42;
    // mapMetaTilePosY[1] = 0;
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

    // old sprite init
    // reticuleASprite = SPR_addSprite(&reticule_a, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
    // reticuleBSprite = SPR_addSprite(&reticule_b, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
    // puckASprite = SPR_addSprite(&puck_b, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
    // puckBSprite = SPR_addSprite(&puck_b, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
    // ballSprite = SPR_addSprite(&ball, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));

    init_actors();

    SPR_update();

    // prepare palettes
    memcpy(&palette[0], palette_arena.data, 16);
    memcpy(&palette[16], reticule_a.palette->data, 32);
    // VDP_setPalette(PAL0, palette_arena.data);
    // VDP_setPalette(PAL1, reticule_a.palette -> data);

    // fade in
    PAL_fadeIn(0, 63, palette, 30, FALSE);

    JOY_setEventHandler(joyEvent);

    // just to monitor frame CPU usage
    SYS_showFrameLoad(TRUE);

    while (TRUE)
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
    setSpritePosition(actorsReticuleA.sprite, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY);
}
