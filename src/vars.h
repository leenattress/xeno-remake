// ---------------------------------------------------------------------------
// Basics
// ---------------------------------------------------------------------------
#define BITSHIFT 4
#define TRUE 1
#define FALSE 0
#define ACTIVE 1
#define SLEEPING 0

// ---------------------------------------------------------------------------
// Controls
// ---------------------------------------------------------------------------
#define CTRL_NONE 0
#define CTRL_UP 1
#define CTRL_DOWN 2
#define CTRL_LEFT 3
#define CTRL_RIGHT 4

// ---------------------------------------------------------------------------
// Global variables
// ---------------------------------------------------------------------------
#define RETICULE_READY 0
#define RETICULE_CHARGING 1
#define RETICULE_ACCEL FIX32(1L)
#define RETICULE_SPEED_MAX FIX32(5L)
#define RETICULE_BRAKING FIX32(1L)
#define BALL_SPEED_MAX FIX32(16L)
#define BALL_FRICTION FIX32(6L)
#define PUCK_FRICTION FIX32(2L)

#define MAP_TILE_SIZE 8
#define MAP_WIDTH 1360
#define MAP_HEIGHT 384
#define MAP_WIDTH_TILES (MAP_WIDTH / MAP_TILE_SIZE)
#define MAP_HEIGHT_TILES (MAP_HEIGHT / MAP_TILE_SIZE)

#define MIN_POSX FIX32(32L)
#define MAX_POSX FIX32(MAP_WIDTH - 32)
#define MIN_POSY FIX32(32L)
#define MAX_POSY FIX32(MAP_HEIGHT - 32)

// Game states
#define GS_MENU 1
#define GS_GAME 2
#define GS_PAUSED 3
#define GS_GAMEOVER 4
#define GS_HIGHSCORES 5

// ---------------------------------------------------------------------------
// Player 1 Variables
// ---------------------------------------------------------------------------
#define ENT_TYPE_PUCK 0
#define ENT_TYPE_BALL 1
#define ENT_TYPE_PARTICLE 2

// ---------------------------------------------------------------------------
// Player 1 Variables
// ---------------------------------------------------------------------------
#define PUCK_A_X FIX32(28L)
#define PUCK_A_Y FIX32(32L)
#define RET_A_X FIX32(32L)
#define RET_A_Y FIX32(32L)

// ---------------------------------------------------------------------------
// Player 2 Variables
// ---------------------------------------------------------------------------
#define PUCK_B_START_X FIX32(36L)
#define PUCK_B_START_Y FIX32(32L)
#define RET_B_START_X FIX32(34L)
#define RET_B_START_Y FIX32(32L)