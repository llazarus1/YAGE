/*
 *  Mouse.cpp
 *  Base
 *
 *  Created by Brent Wilson on 11/11/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#include <Render/SDL_Helper.h>
#include "Mouse.h"

Mouse::Mouse() {}
Mouse::~Mouse() {}

bool Mouse::isButtonDown(int key) {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(key);
}

void Mouse::pressButton(int button, int x, int y) {
    SDL_Event *event = new SDL_Event;
    SDL_MouseButtonEvent *mouse = &event->button;
    mouse->type = SDL_MOUSEBUTTONDOWN;
    mouse->state = SDL_PRESSED;
    mouse->button = button;
    mouse->x = x;
    mouse->y = y;
    SDL_PushEvent(event);

    mouse->type = SDL_MOUSEBUTTONUP;
    mouse->state = SDL_RELEASED;
    SDL_PushEvent(event);

    delete event;
}

void Mouse::getMousePos(int &x, int &y) {
    SDL_GetMouseState(&x, &y);
}

void Mouse::setMousePos(int x, int y) {
    SDL_WarpMouse(x, y);
}
