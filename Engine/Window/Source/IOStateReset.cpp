#include "WindowIO.h"

using namespace toy::io;

void MouseState::reset()
{
    leftButton = ButtonState::unpressed;
    middleButton = ButtonState::unpressed;
    rightButton = ButtonState::unpressed;

    wheel = 0.0f;
    //TODO: mouse position shouldn't reset, it should stay at a preview values
    //position.x = 0;
    //position.y = 0;
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
    space = ButtonState::unpressed;
    shiftLeft = ButtonState::unpressed;
    enter = ButtonState::unpressed;
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
}
