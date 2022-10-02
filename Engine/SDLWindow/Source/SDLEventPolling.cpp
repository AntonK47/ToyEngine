#include "SDLWindow.h"
#include "SDL_events.h"

using namespace toy::window;

#define KEYDOWN_ASSIGNMENT(var, sdlKey)\
if(event.key.keysym.sym == (sdlKey))\
{\
	windowIo_.keyboardState.var = io::ButtonState::pressed;\
}

void SDLWindow::pollEventsInternal()
{
    resetPolledEventsAndIo();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            currentPolledEvents_.push_back(Event::quit);
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
            KEYDOWN_ASSIGNMENT(space, SDLK_SPACE)
            KEYDOWN_ASSIGNMENT(enter, SDLK_KP_ENTER)
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
            break;
        default:
            break;
        }
    }
}
