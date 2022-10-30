#pragma once

#include "CommonTypes.h"

namespace toy::io
{
    enum class ButtonState
    {
        pressed,
        unpressed
    };

    using WheelState = float;

    struct Point
    {
        core::i32 x;
        core::i32 y;
    };

    struct MouseState
    {
        ButtonState leftButton;
        ButtonState middleButton;
        ButtonState rightButton;
        WheelState wheel;
        Point position;

        void reset();
    };
    struct KeyboardState
    {
        ButtonState a;
        ButtonState b;
        ButtonState c;
        ButtonState d;
        ButtonState e;
        ButtonState f;
        ButtonState g;
        ButtonState h;
        ButtonState i;
        ButtonState j;
        ButtonState k;
        ButtonState l;
        ButtonState m;
        ButtonState n;
        ButtonState o;
        ButtonState p;
        ButtonState q;
        ButtonState r;
        ButtonState s;
        ButtonState t;
        ButtonState u;
        ButtonState v;
        ButtonState w;
        ButtonState x;
        ButtonState y;
        ButtonState z;
        ButtonState space;
        ButtonState shiftLeft;
        ButtonState enter;
        ButtonState zero;
        ButtonState one;
        ButtonState two;
        ButtonState three;
        ButtonState four;
        ButtonState five;
        ButtonState six;
        ButtonState seven;
        ButtonState eight;
        ButtonState nine;

        void reset();

    };
    //struct GamepadState {};

    struct WindowIo
    {
        MouseState mouseState;
        KeyboardState keyboardState;
    };
}
