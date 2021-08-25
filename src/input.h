
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
