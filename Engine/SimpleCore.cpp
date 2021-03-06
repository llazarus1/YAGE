/*
 *  SimpleCore.cpp
 *  Base
 *
 *  Created by Brent Wilson on 11/12/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#include <Engine/Camera.h>
#include <Render/RenderContext.h>
#include <Render/Viewport.h>

#include "SimpleCore.h"
#include "EventPump.h"
#include "FrameListener.h"
#include "Window.h"


SimpleCore::SimpleCore(int width, int height, const std::string &name):
    AbstractCore(width, height, name),
    _mainCamera(NULL)
{
    _mainCamera = new Camera("Main Camera");
}

SimpleCore::~SimpleCore() {
    delete _mainCamera;
}

void SimpleCore::innerLoop(int elapsed) {
    update(elapsed);

    _renderContext->resetCounts();

    display(elapsed);

    setPostText();

    _mainWindow->swapBuffers();
}
