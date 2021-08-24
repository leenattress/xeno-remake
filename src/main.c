#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sound.h"
#include "dma.h"

#define RETICULE_READY      0
#define RETICULE_CHARGING   1

#define RETICULE_ACCEL      FIX32(1L)
#define RETICULE_SPEED_MAX  FIX32(4L)

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


// 42 * 32 = complete tilemap update; * 2 as we have 2 full plans to update potentially
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

int main(u16 hard)
{
    u16 palette[64];
    u16 ind;
    //u16 numTile;

    // initialization
    VDP_setScreenWidth320();

    // init SFX
    // XGM_setPCM(SFX_JUMP, sonic_jump_sfx, sizeof(sonic_jump_sfx));

    // start music
    // XGM_startPlay(sonic_music);

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

    // prepare palettes (BGB image contains the 4 palettes data)
    memcpy(&palette[0], palette_arena.data, 16);
    memcpy(&palette[16], palette_players.data, 16);
    // memcpy(&palette[16], bga_image.palette->data, 16 * 2);
    // memcpy(&palette[32], sonic_sprite.palette->data, 16 * 2);
    // memcpy(&palette[48], enemies_sprite.palette->data, 16 * 2);

    // fade in
    PAL_fadeIn(0, (4 * 16) - 1, palette, 20, FALSE);

    JOY_setEventHandler(joyEvent);

    // just to monitor frame CPU usage
    SYS_showFrameLoad(TRUE);

//    reseted = FALSE;

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
    // update position left/right
    if (xOrder > 0)
    {
        movX += RETICULE_ACCEL;
        if (movX >= RETICULE_SPEED_MAX) movX = RETICULE_SPEED_MAX;
    }
    else if (xOrder < 0)
    {
        movX -= RETICULE_ACCEL;
        if (movX <= -RETICULE_SPEED_MAX) movX = -RETICULE_SPEED_MAX;
    }
    else
    {
        if ((movX < FIX32(0.1)) && (movX > FIX32(-0.1)))
            movX = 0;
        else if ((movX < FIX32(0.3)) && (movX > FIX32(-0.3)))
            movX -= movX >> 2;
        else if ((movX < FIX32(1)) && (movX > FIX32(-1)))
            movX -= movX >> 3;
        else
            movX -= movX >> 4;
    }

    // update position up/down
    if (yOrder > 0)
    {
        movY += RETICULE_ACCEL;
        if (movY >= RETICULE_SPEED_MAX) movY = RETICULE_SPEED_MAX;
    }
    else if (yOrder < 0)
    {
        movY -= RETICULE_ACCEL;
        if (movY <= -RETICULE_SPEED_MAX) movY = -RETICULE_SPEED_MAX;
    }
    else
    {
        if ((movY < FIX32(0.1)) && (movY > FIX32(-0.1)))
            movY = 0;
        else if ((movX < FIX32(0.3)) && (movY > FIX32(-0.3)))
            movY -= movY >> 2;
        else if ((movY < FIX32(1)) && (movY > FIX32(-1)))
            movY -= movY >> 3;
        else
            movY -= movY >> 4;
    }    

    // update position
    posX += movX;
    posY += movY;

    // check if we are out of the map
    if (posX >= MAX_POSX)
    {
        posX = MAX_POSX;
        movX = 0;
    }
    else if (posX <= MIN_POSX)
    {
        posX = MIN_POSX;
        movX = 0;
    }

    if (posY >= MAX_POSY)
    {
        posY = MAX_POSY;
        movY = 0;
    }
    else if (posY <= MIN_POSY)
    {
        posY = MIN_POSY;
        movY = 0;
    }    

    // update camera position (*after* player sprite position has been updated)
    updateCameraPosition();

    // set sprites position
    setSpritePosition(reticuleA, fix32ToInt(posX) - camPosX, fix32ToInt(posY) - camPosY);
}

static void setSpritePosition(Sprite* sprite, s16 x, s16 y)
{
    // clip out of screen sprites
    if ((x < -100) || (x > 320) || (y < -100) || (y > 240)) SPR_setVisibility(sprite, HIDDEN);
    else
    {
        SPR_setVisibility(sprite, VISIBLE);
        SPR_setPosition(sprite, x, y);
    }
}

static void updateAnim()
{
    SPR_setAnim(reticuleA, RETICULE_READY);

    // jumping
    // if (movY) SPR_setAnim(player, ANIM_ROLL);
    // else
    // {
    //     if (((movX >= BRAKE_SPEED) && (xOrder < 0)) || ((movX <= -BRAKE_SPEED) && (xOrder > 0)))
    //     {
    //         if (player->animInd != ANIM_BRAKE)
    //         {
    //             SND_startPlayPCM_XGM(SFX_STOP, 1, SOUND_PCM_CH2);
    //             SPR_setAnim(player, ANIM_BRAKE);
    //         }
    //     }
    //     else if ((movX >= RUN_SPEED) || (movX <= -RUN_SPEED))
    //         SPR_setAnim(player, ANIM_RUN);
    //     else if (movX != 0)
    //         SPR_setAnim(player, ANIM_WALK);
    //     else
    //     {
    //         if (yOrder < 0)
    //             SPR_setAnim(player, ANIM_UP);
    //         else if (yOrder > 0)
    //             SPR_setAnim(player, ANIM_CROUNCH);
    //         else
    //             SPR_setAnim(player, ANIM_STAND);
    //     }
    // }

    // if (movX > 0) SPR_setHFlip(player, FALSE);
    // else if (movX < 0) SPR_setHFlip(player, TRUE);

}

static void updateCameraPosition()
{
    // get player position (pixel)
    s16 px = fix32ToInt(posX);
    s16 py = fix32ToInt(posY);
    // current sprite position on screen
    s16 px_scr = px - camPosX;
    s16 py_scr = py - camPosY;

    s16 npx_cam, npy_cam;

    // adjust new camera position
    if (px_scr > 240) npx_cam = px - 240;
    else if (px_scr < 40) npx_cam = px - 40;
    else npx_cam = camPosX;
    if (py_scr > 140) npy_cam = py - 140;
    else if (py_scr < 60) npy_cam = py - 60;
    else npy_cam = camPosY;

    // clip camera position
    if (npx_cam < 0) npx_cam = 0;
    else if (npx_cam > (MAP_WIDTH - 320)) npx_cam = (MAP_WIDTH - 320);
    if (npy_cam < 0) npy_cam = 0;
    else if (npy_cam > (MAP_HEIGHT - 224)) npy_cam = (MAP_HEIGHT - 224);

    // set new camera position
    setCameraPosition(npx_cam, npy_cam);
}

static void setCameraPosition(s16 x, s16 y)
{
    if ((x != camPosX) || (y != camPosY))
    {
        camPosX = x;
        camPosY = y;

        // method 1 
        MAP_scrollTo(arenaMap, x, y);

        // method 2 
        //updateMap(BG_A, arenaMap, x >> 4, y >> 4);
        //scrollNeedUpdate = TRUE;

        // always store it to avoid full map update on method change
        mapMetaTilePosX[BG_A] = x >> 4;
        mapMetaTilePosY[BG_A] = y >> 4;
    }
}

// this is just to show how use the MAP_getTilemapRect(..) method
// if we weed to actually access tilemap data and do manual tilemap update to VDP
static void updateMap(VDPPlane plane, Map* map, s16 xmt, s16 ymt)
{
    // BGA = 0; BGB = 1
    s16 cxmt = mapMetaTilePosX[plane];
    s16 cymt = mapMetaTilePosY[plane];
    s16 deltaX = xmt - cxmt;
    s16 deltaY = ymt - cymt;

    // no update --> exit
    if ((deltaX == 0) && (deltaY == 0)) return;

    // clip to 21 metatiles column max (full screen update)
    if (deltaX > 21)
    {
        cxmt += deltaX - 21;
        deltaX = 21;
        deltaY = 0;
    }
    // clip to 21 metatiles column max (full screen update)
    else if (deltaX < -21)
    {
        cxmt += deltaX + 21;
        deltaX = -21;
        deltaY = 0;
    }
    // clip to 16 metatiles row max (full screen update)
    else if (deltaY > 16)
    {
        cymt += deltaY - 16;
        deltaY = 16;
        deltaX = 0;
    }
    // clip to 16 metatiles row max (full screen update)
    else if (deltaY < -16)
    {
        cymt += deltaY + 16;
        deltaY = -16;
        deltaX = 0;
    }

    if (deltaX > 0)
    {
        // update on right
        cxmt += 21;

        // need to update map column on right
        while(deltaX--)
        {
            MAP_getTilemapRect(map, cxmt, ymt, 1, 16, TRUE, tilemapBuf + bufOffset);
            VDP_setTileMapDataColumnFast(plane, tilemapBuf + bufOffset, (cxmt * 2) + 0, ymt * 2, 16 * 2, DMA_QUEUE);
            // next column
            bufOffset += 16 * 2;
            VDP_setTileMapDataColumnFast(plane, tilemapBuf + bufOffset, (cxmt * 2) + 1, ymt * 2, 16 * 2, DMA_QUEUE);
            // next column
            bufOffset += 16 * 2;
            cxmt++;
        }
    }
    else
    {
        // need to update map column on left
        while(deltaX++)
        {
            cxmt--;
            MAP_getTilemapRect(map, cxmt, ymt, 1, 16, TRUE, tilemapBuf + bufOffset);
            VDP_setTileMapDataColumnFast(plane, tilemapBuf + bufOffset, (cxmt * 2) + 0, ymt * 2, 16 * 2, DMA_QUEUE);
            // next column
            bufOffset += 16 * 2;
            VDP_setTileMapDataColumnFast(plane, tilemapBuf + bufOffset, (cxmt * 2) + 1, ymt * 2, 16 * 2, DMA_QUEUE);
            // next column
            bufOffset += 16 * 2;
        }
    }

    if (deltaY > 0)
    {
        // update at bottom
        cymt += 16;

        // need to update map row on bottom
        while(deltaY--)
        {
            MAP_getTilemapRect(map, xmt, cymt, 21, 1, FALSE, tilemapBuf + bufOffset);
            VDP_setTileMapDataRow(plane, tilemapBuf + bufOffset, (cymt * 2) + 0, (xmt * 2), 21 * 2, DMA_QUEUE);
            // next row
            bufOffset += 21 * 2;
            VDP_setTileMapDataRow(plane, tilemapBuf + bufOffset, (cymt * 2) + 1, (xmt * 2), 21 * 2, DMA_QUEUE);
            // next row
            bufOffset += 21 * 2;
            cymt++;
        }
    }
    else
    {
        // need to update map row on top
        while(deltaY++)
        {
            cymt--;
            MAP_getTilemapRect(map, xmt, cymt, 21, 1, FALSE, tilemapBuf + bufOffset);
            VDP_setTileMapDataRow(plane, tilemapBuf + bufOffset, (cymt * 2) + 0, (xmt * 2), 21 * 2, DMA_QUEUE);
            // next row
            bufOffset += 21 * 2;
            VDP_setTileMapDataRow(plane, tilemapBuf + bufOffset, (cymt * 2) + 1, (xmt * 2), 21 * 2, DMA_QUEUE);
            // next row
            bufOffset += 21 * 2;
        }
    }

    mapMetaTilePosX[plane] = xmt;
    mapMetaTilePosY[plane] = ymt;
}

static void updateVDPScroll()
{
    VDP_setHorizontalScroll(BG_A, -camPosX);
    //VDP_setHorizontalScroll(BG_B, (-camPosX) >> 3);
    VDP_setVerticalScroll(BG_A, camPosY);
    // VDP_setVerticalScroll(BG_B, camPosY >> 5);
}

// static void frameChanged(Sprite* sprite)
// {
//     // get enemy index (stored in data field)
//     u16 enemyIndex = sprite->data;
//     // get VRAM tile index for this animation of this sprite
//     u16 tileIndex = sprTileIndexes[enemyIndex][sprite->animInd][sprite->frameInd];
//     // manually set tile index for the current frame (preloaded in VRAM)
//     SPR_setVRAMTileIndex(sprite, tileIndex);
// }

static void handleInput()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (paused)
    {
    // menu things
    }
    // can affect gameplay
    else
    {
        if (value & BUTTON_UP) yOrder = -1;
        else if (value & BUTTON_DOWN) yOrder = +1;
        else yOrder = 0;

        if (value & BUTTON_LEFT) xOrder = -1;
        else if (value & BUTTON_RIGHT) xOrder = +1;
        else xOrder = 0;

        // if (value & BUTTON_X) alternateScrollMethod = TRUE;
        // else alternateScrollMethod = FALSE;
    }
}

static void joyEvent(u16 joy, u16 changed, u16 state)
{
    // START button state changed --> pause / unpause
    if (changed & state & BUTTON_START)
    {
        paused = !paused;
//        // change scroll method when pressing pause
//        if (paused)
//            alternateScrollMethod = !alternateScrollMethod;

    }

    // can't do more in paused state
    if (paused) return;

    if (changed & state & (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_X | BUTTON_Y | BUTTON_Z))
    {
        // handle buttons here
    }
}
