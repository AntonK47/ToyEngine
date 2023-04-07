#pragma once

#include <Core.h>

namespace toy::io
{
    enum class ButtonState
    {
        pressed,
        unpressed
    };

    struct WheelState
    {
        float x;
        float y;
    };

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

        ButtonState space; 
        ButtonState backspace;
        ButtonState enter;
        ButtonState shiftLeft;
        ButtonState shiftRight;
        ButtonState bracketLeft;
        ButtonState bracketRight;
        ButtonState backslash;

        ButtonState altLeft;
        ButtonState altRight;


        ButtonState semicolon;
        ButtonState apostrophe;
        ButtonState comma;
        ButtonState period;
        ButtonState slash;



        ButtonState graveAccent;//tilde
       
        ButtonState minus;
        ButtonState equal;
        
        ButtonState controlLeft;
        ButtonState controlRight;
        ButtonState escape;
        ButtonState capsLock;
        ButtonState tab;

        ButtonState f1;
        ButtonState f2;
        ButtonState f3;
        ButtonState f4;
        ButtonState f5;
        ButtonState f6;
        ButtonState f7;
        ButtonState f8;
        ButtonState f9;
        ButtonState f10;
        ButtonState f11;
        ButtonState f12;



        void reset();

    };
    //struct GamepadState {};

    struct WindowIo
    {
        MouseState mouseState;
        KeyboardState keyboardState;
    };
}
