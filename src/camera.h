
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
        //MAP_scrollTo(arenaMap, x, y);

        // method 2 
        updateMap(BG_A, arenaMap, x >> 4, y >> 4);
        scrollNeedUpdate = TRUE;

        // always store it to avoid full map update on method change
        mapMetaTilePosX[BG_A] = x >> 4;
        mapMetaTilePosY[BG_A] = y >> 4;
    }
}