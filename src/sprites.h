

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

// static void frameChanged(Sprite* sprite)
// {
//     // get enemy index (stored in data field)
//     u16 enemyIndex = sprite->data;
//     // get VRAM tile index for this animation of this sprite
//     u16 tileIndex = sprTileIndexes[enemyIndex][sprite->animInd][sprite->frameInd];
//     // manually set tile index for the current frame (preloaded in VRAM)
//     SPR_setVRAMTileIndex(sprite, tileIndex);
// }