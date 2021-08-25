
static void handleReticuleMovement()
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

}