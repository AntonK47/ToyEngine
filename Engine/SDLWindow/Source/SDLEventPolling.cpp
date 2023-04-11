#include <SDLWindow.h>
#include <SDL_events.h>

using namespace toy::window;

#define KEYDOWN_ASSIGNMENT(var, sdlKey)\
if(event.key.keysym.sym == (sdlKey))\
{\
	windowIo_.keyboardState.var = io::ButtonState::pressed;\
}

#define KEYUP_ASSIGNMENT(var, sdlKey)\
if(event.key.keysym.sym == (sdlKey))\
{\
	windowIo_.keyboardState.var = io::ButtonState::unpressed;\
}

void SDLWindow::pollEventsInternal()
{
    resetPolledEventsAndIo();

    windowIo_.mouseState.wheel.x = 0.0;
    windowIo_.mouseState.wheel.y = 0.0;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            currentPolledEvents_.push_back(Event::quit);
            break;
        case SDL_KEYUP:
            KEYUP_ASSIGNMENT(a, SDLK_a)
            KEYUP_ASSIGNMENT(b, SDLK_b)
            KEYUP_ASSIGNMENT(c, SDLK_c)
            KEYUP_ASSIGNMENT(d, SDLK_d)
            KEYUP_ASSIGNMENT(e, SDLK_e)
            KEYUP_ASSIGNMENT(f, SDLK_f)
            KEYUP_ASSIGNMENT(g, SDLK_g)
            KEYUP_ASSIGNMENT(h, SDLK_h)
            KEYUP_ASSIGNMENT(i, SDLK_i)
            KEYUP_ASSIGNMENT(j, SDLK_j)
            KEYUP_ASSIGNMENT(k, SDLK_k)
            KEYUP_ASSIGNMENT(l, SDLK_l)
            KEYUP_ASSIGNMENT(m, SDLK_m)
            KEYUP_ASSIGNMENT(n, SDLK_n)
            KEYUP_ASSIGNMENT(o, SDLK_o)
            KEYUP_ASSIGNMENT(p, SDLK_p)
            KEYUP_ASSIGNMENT(q, SDLK_q)
            KEYUP_ASSIGNMENT(r, SDLK_r)
            KEYUP_ASSIGNMENT(s, SDLK_s)
            KEYUP_ASSIGNMENT(t, SDLK_t)
            KEYUP_ASSIGNMENT(u, SDLK_u)
            KEYUP_ASSIGNMENT(v, SDLK_v)
            KEYUP_ASSIGNMENT(w, SDLK_w)
            KEYUP_ASSIGNMENT(x, SDLK_x)
            KEYUP_ASSIGNMENT(y, SDLK_y)
            KEYUP_ASSIGNMENT(z, SDLK_z)
            KEYUP_ASSIGNMENT(zero, SDLK_0)
            KEYUP_ASSIGNMENT(one, SDLK_1)
            KEYUP_ASSIGNMENT(two, SDLK_2)
            KEYUP_ASSIGNMENT(three, SDLK_3)
            KEYUP_ASSIGNMENT(four, SDLK_4)
            KEYUP_ASSIGNMENT(five, SDLK_5)
            KEYUP_ASSIGNMENT(six, SDLK_6)
            KEYUP_ASSIGNMENT(seven, SDLK_7)
            KEYUP_ASSIGNMENT(eight, SDLK_8)
            KEYUP_ASSIGNMENT(nine, SDLK_9)

            KEYUP_ASSIGNMENT(space, SDLK_SPACE)
            KEYUP_ASSIGNMENT(backspace, SDLK_BACKSPACE)
            KEYUP_ASSIGNMENT(enter, SDLK_RETURN)
            KEYUP_ASSIGNMENT(shiftLeft, SDLK_LSHIFT)
            KEYUP_ASSIGNMENT(shiftRight, SDLK_RSHIFT)
            KEYUP_ASSIGNMENT(bracketLeft, SDLK_LEFTBRACKET)
            KEYUP_ASSIGNMENT(bracketRight, SDLK_RIGHTBRACKET)
            KEYUP_ASSIGNMENT(altLeft, SDLK_LALT)
            KEYUP_ASSIGNMENT(altRight, SDLK_RALT)
            KEYUP_ASSIGNMENT(semicolon, SDLK_SEMICOLON)
            KEYUP_ASSIGNMENT(apostrophe, SDLK_QUOTE)
            KEYUP_ASSIGNMENT(comma, SDLK_COMMA)
            KEYUP_ASSIGNMENT(period, SDLK_PERIOD)
            KEYUP_ASSIGNMENT(slash, SDLK_SLASH)
            KEYUP_ASSIGNMENT(graveAccent, SDLK_BACKQUOTE)
            KEYUP_ASSIGNMENT(minus, SDLK_MINUS)
            KEYUP_ASSIGNMENT(equal, SDLK_EQUALS)
            KEYUP_ASSIGNMENT(controlLeft, SDLK_LCTRL)
            KEYUP_ASSIGNMENT(controlRight, SDLK_RCTRL)
            KEYUP_ASSIGNMENT(escape, SDLK_ESCAPE)
            KEYUP_ASSIGNMENT(capsLock, SDLK_CAPSLOCK)
            KEYUP_ASSIGNMENT(tab, SDLK_TAB)

            KEYUP_ASSIGNMENT(f1, SDLK_F1)
            KEYUP_ASSIGNMENT(f2, SDLK_F2)
            KEYUP_ASSIGNMENT(f3, SDLK_F3)
            KEYUP_ASSIGNMENT(f4, SDLK_F4)
            KEYUP_ASSIGNMENT(f5, SDLK_F5)
            KEYUP_ASSIGNMENT(f6, SDLK_F6)
            KEYUP_ASSIGNMENT(f7, SDLK_F7)
            KEYUP_ASSIGNMENT(f8, SDLK_F8)
            KEYUP_ASSIGNMENT(f9, SDLK_F9)
            KEYUP_ASSIGNMENT(f10, SDLK_F10)
            KEYUP_ASSIGNMENT(f11, SDLK_F11)
            KEYUP_ASSIGNMENT(f12, SDLK_F12)
            break;
        case SDL_KEYDOWN:
            KEYDOWN_ASSIGNMENT(a, SDLK_a)
            KEYDOWN_ASSIGNMENT(b, SDLK_b)
            KEYDOWN_ASSIGNMENT(c, SDLK_c)
            KEYDOWN_ASSIGNMENT(d, SDLK_d)
            KEYDOWN_ASSIGNMENT(e, SDLK_e)
            KEYDOWN_ASSIGNMENT(f, SDLK_f)
            KEYDOWN_ASSIGNMENT(g, SDLK_g)
            KEYDOWN_ASSIGNMENT(h, SDLK_h)
            KEYDOWN_ASSIGNMENT(i, SDLK_i)
            KEYDOWN_ASSIGNMENT(j, SDLK_j)
            KEYDOWN_ASSIGNMENT(k, SDLK_k)
            KEYDOWN_ASSIGNMENT(l, SDLK_l)
            KEYDOWN_ASSIGNMENT(m, SDLK_m)
            KEYDOWN_ASSIGNMENT(n, SDLK_n)
            KEYDOWN_ASSIGNMENT(o, SDLK_o)
            KEYDOWN_ASSIGNMENT(p, SDLK_p)
            KEYDOWN_ASSIGNMENT(q, SDLK_q)
            KEYDOWN_ASSIGNMENT(r, SDLK_r)
            KEYDOWN_ASSIGNMENT(s, SDLK_s)
            KEYDOWN_ASSIGNMENT(t, SDLK_t)
            KEYDOWN_ASSIGNMENT(u, SDLK_u)
            KEYDOWN_ASSIGNMENT(v, SDLK_v)
            KEYDOWN_ASSIGNMENT(w, SDLK_w)
            KEYDOWN_ASSIGNMENT(x, SDLK_x)
            KEYDOWN_ASSIGNMENT(y, SDLK_y)
            KEYDOWN_ASSIGNMENT(z, SDLK_z)
            KEYDOWN_ASSIGNMENT(zero, SDLK_0)
            KEYDOWN_ASSIGNMENT(one, SDLK_1)
            KEYDOWN_ASSIGNMENT(two, SDLK_2)
            KEYDOWN_ASSIGNMENT(three, SDLK_3)
            KEYDOWN_ASSIGNMENT(four, SDLK_4)
            KEYDOWN_ASSIGNMENT(five, SDLK_5)
            KEYDOWN_ASSIGNMENT(six, SDLK_6)
            KEYDOWN_ASSIGNMENT(seven, SDLK_7)
            KEYDOWN_ASSIGNMENT(eight, SDLK_8)
            KEYDOWN_ASSIGNMENT(nine, SDLK_9)

            KEYDOWN_ASSIGNMENT(space, SDLK_SPACE)
            KEYDOWN_ASSIGNMENT(backspace, SDLK_BACKSPACE)
            KEYDOWN_ASSIGNMENT(enter, SDLK_RETURN)
            KEYDOWN_ASSIGNMENT(shiftLeft, SDLK_LSHIFT)
            KEYDOWN_ASSIGNMENT(shiftRight, SDLK_RSHIFT)
            KEYDOWN_ASSIGNMENT(bracketLeft, SDLK_LEFTBRACKET)
            KEYDOWN_ASSIGNMENT(bracketRight, SDLK_RIGHTBRACKET)
            KEYDOWN_ASSIGNMENT(altLeft, SDLK_LALT)
            KEYDOWN_ASSIGNMENT(altRight, SDLK_RALT)
            KEYDOWN_ASSIGNMENT(semicolon, SDLK_SEMICOLON)
            KEYDOWN_ASSIGNMENT(apostrophe, SDLK_QUOTE)
            KEYDOWN_ASSIGNMENT(comma, SDLK_COMMA)
            KEYDOWN_ASSIGNMENT(period, SDLK_PERIOD)
            KEYDOWN_ASSIGNMENT(slash, SDLK_SLASH)
            KEYDOWN_ASSIGNMENT(graveAccent, SDLK_BACKQUOTE)
            KEYDOWN_ASSIGNMENT(minus, SDLK_MINUS)
            KEYDOWN_ASSIGNMENT(equal, SDLK_EQUALS)
            KEYDOWN_ASSIGNMENT(controlLeft, SDLK_LCTRL)
            KEYDOWN_ASSIGNMENT(controlRight, SDLK_RCTRL)
            KEYDOWN_ASSIGNMENT(escape, SDLK_ESCAPE)
            KEYDOWN_ASSIGNMENT(capsLock, SDLK_CAPSLOCK)
            KEYDOWN_ASSIGNMENT(tab, SDLK_TAB)
            KEYDOWN_ASSIGNMENT(f1, SDLK_F1)
            KEYDOWN_ASSIGNMENT(f2, SDLK_F2)
            KEYDOWN_ASSIGNMENT(f3, SDLK_F3)
            KEYDOWN_ASSIGNMENT(f4, SDLK_F4)
            KEYDOWN_ASSIGNMENT(f5, SDLK_F5)
            KEYDOWN_ASSIGNMENT(f6, SDLK_F6)
            KEYDOWN_ASSIGNMENT(f7, SDLK_F7)
            KEYDOWN_ASSIGNMENT(f8, SDLK_F8)
            KEYDOWN_ASSIGNMENT(f9, SDLK_F9)
            KEYDOWN_ASSIGNMENT(f10, SDLK_F10)
            KEYDOWN_ASSIGNMENT(f11, SDLK_F11)
            KEYDOWN_ASSIGNMENT(f12, SDLK_F12)
            break;
        case SDL_MOUSEMOTION:
            windowIo_.mouseState.position.x = event.motion.x;
            windowIo_.mouseState.position.y = event.motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                windowIo_.mouseState.leftButton = io::ButtonState::pressed;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                windowIo_.mouseState.rightButton = io::ButtonState::pressed;
            }
            if (event.button.button == SDL_BUTTON_MIDDLE)
            {
                windowIo_.mouseState.middleButton = io::ButtonState::pressed;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                windowIo_.mouseState.leftButton = io::ButtonState::unpressed;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                windowIo_.mouseState.rightButton = io::ButtonState::unpressed;
            }
            if (event.button.button == SDL_BUTTON_MIDDLE)
            {
                windowIo_.mouseState.middleButton = io::ButtonState::unpressed;
            }
            break;
        case SDL_MOUSEWHEEL:
        {

            float wheelX = -event.wheel.preciseX;
            float wheelY = event.wheel.preciseY;
            windowIo_.mouseState.wheel.x = wheelX;
            windowIo_.mouseState.wheel.y = wheelY;

            break;
        }
        case SDL_TEXTINPUT:
            windowIo_.textState.text = std::string(event.text.text, strlen(event.text.text));
            break;
        default:
            break;
        }
    }
}
