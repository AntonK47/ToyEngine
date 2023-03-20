#include "WindowIO.h"

using namespace toy::io;

void MouseState::reset()
{
    leftButton = ButtonState::unpressed;
    middleButton = ButtonState::unpressed;
    rightButton = ButtonState::unpressed;

    wheel.x = 0.0f;
    wheel.y = 0.0f;
}

void KeyboardState::reset()
{
    a = ButtonState::unpressed;
    b = ButtonState::unpressed;
    c = ButtonState::unpressed;
    d = ButtonState::unpressed;
    e = ButtonState::unpressed;
    f = ButtonState::unpressed;
    g = ButtonState::unpressed;
    h = ButtonState::unpressed;
    i = ButtonState::unpressed;
    j = ButtonState::unpressed;
    k = ButtonState::unpressed;
    l = ButtonState::unpressed;
    m = ButtonState::unpressed;
    n = ButtonState::unpressed;
    o = ButtonState::unpressed;
    p = ButtonState::unpressed;
    q = ButtonState::unpressed;
    r = ButtonState::unpressed;
    s = ButtonState::unpressed;
    t = ButtonState::unpressed;
    u = ButtonState::unpressed;
    v = ButtonState::unpressed;
    w = ButtonState::unpressed;
    x = ButtonState::unpressed;
    y = ButtonState::unpressed;
    z = ButtonState::unpressed;
    zero = ButtonState::unpressed;
    one = ButtonState::unpressed;
    two = ButtonState::unpressed;
    three = ButtonState::unpressed;
    four = ButtonState::unpressed;
    five = ButtonState::unpressed;
    six = ButtonState::unpressed;
    seven = ButtonState::unpressed;
    eight = ButtonState::unpressed;
    nine = ButtonState::unpressed;

    space = ButtonState::unpressed;
    backspace = ButtonState::unpressed;
    enter = ButtonState::unpressed;
    shiftLeft = ButtonState::unpressed;
    shiftRight = ButtonState::unpressed;
    bracketLeft = ButtonState::unpressed;
    bracketRight = ButtonState::unpressed;
    backslash = ButtonState::unpressed;

    altLeft = ButtonState::unpressed;
    altRight = ButtonState::unpressed;

    semicolon = ButtonState::unpressed;
    apostroph = ButtonState::unpressed;
    comma = ButtonState::unpressed;
    period = ButtonState::unpressed;
    slash = ButtonState::unpressed;

    graveAccent = ButtonState::unpressed;

    minus = ButtonState::unpressed;
    equel = ButtonState::unpressed;

    controlLeft = ButtonState::unpressed;
    controlRight = ButtonState::unpressed;
    escape = ButtonState::unpressed;
    capsLock = ButtonState::unpressed;
    tab = ButtonState::unpressed;

    f1 = ButtonState::unpressed;
    f2 = ButtonState::unpressed;
    f3 = ButtonState::unpressed;
    f4 = ButtonState::unpressed;
    f5 = ButtonState::unpressed;
    f6 = ButtonState::unpressed;
    f7 = ButtonState::unpressed;
    f8 = ButtonState::unpressed;
    f9 = ButtonState::unpressed;
    f10 = ButtonState::unpressed;
    f11 = ButtonState::unpressed;
    f12 = ButtonState::unpressed;
}
