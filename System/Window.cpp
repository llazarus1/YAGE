/*
 *  Window.cpp
 *  Engine
 *
 *  Created by Brent Wilson on 4/4/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#include <Base/Exception.h>
#include <Render/SDL_Helper.h>
#include <Render/GL_Helper.h>
#include "Window.h"

Window::Window(int width, int height, bool fullscreen, const std::string &caption)
:RenderTarget(width, height), _videoFlags(0), _framebuffer(NULL), _caption("Engine"),
_iconPath(""), _postCaption("") {
    initSDL(width, height, fullscreen);
    setCaption(caption, "");
}

Window::~Window() {
    if (_framebuffer) {
        SDL_FreeSurface((SDL_Surface*)_framebuffer);
    }

    SDL_Quit();
}

void Window::enable() {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // \todo If we ever want to allow multiple windows, we'll need to worry about setting
    // the active context. As far as that GL call goes, I kinda feel dirty having it here.
    // Seems that things should clean up after themselves.... Also, it's weird having a
    // lone GL call here...
}

void Window::initSDL(int width, int height, bool fullscreen) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        THROW(InvalidStateError, "SDL initialization failed: " << SDL_GetError());
    }

    _videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL
    _videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering
    _videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware
    _videoFlags |= SDL_HWSURFACE;       // Use HW surfaces if possible
    _videoFlags |= SDL_HWACCEL;         // Use HW accelerated blits if possible

    if (fullscreen) {
        Info("Using fullscreen mode.");
        _videoFlags |= SDL_FULLSCREEN;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Take control of the mouse and hide the cursor
    SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_GrabInput(SDL_GRAB_ON);

    resize(width, height);
    printVideoInfo();
}

void Window::setSampleCount(int samples) {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, samples);
    resize(_width, _height);
}

void Window::printVideoInfo() {
    const SDL_VideoInfo *videoInfo;
    if (!(videoInfo = SDL_GetVideoInfo())) {
        THROW(InternalError, "Video query failed: " << SDL_GetError());
    }

    Info("Video state information:");
    LogStream::IncrementIndent();
    Info("SDL accel:     " << videoInfo->hw_available);
    Info("OpenGL accel:  " << GetSDLGLAttribute(SDL_GL_ACCELERATED_VISUAL));
    Info("Resolution:    " << videoInfo->current_w << "x" << videoInfo->current_h);
    Info("FB size:       " << GetSDLGLAttribute(SDL_GL_BUFFER_SIZE));
    Info("Depth size:    " << GetSDLGLAttribute(SDL_GL_DEPTH_SIZE));
    Info("Doublebuffer:  " << GetSDLGLAttribute(SDL_GL_DOUBLEBUFFER));
    Info("Vertical sync: " << GetSDLGLAttribute(SDL_GL_SWAP_CONTROL));
    Info("FSAA samples:  " << GetSDLGLAttribute(SDL_GL_MULTISAMPLESAMPLES));
    LogStream::DecrementIndent();
}

void Window::resize(int width, int height) {
    if (_framebuffer) {
        SDL_FreeSurface((SDL_Surface*)_framebuffer);
    }

    if (!(_framebuffer = SDL_SetVideoMode(width, height, 32, _videoFlags))) {
        Error("Window: Video mode set failed: " << SDL_GetError());
    }

    RenderTarget::resize(width, height);
}

void Window::toggleFullscreen() {
    if (SDL_WM_ToggleFullScreen((SDL_Surface*)_framebuffer) != 1) {
        Error("Unable to toggle fullscreen: " << SDL_GetError());
    }
}

void Window::updateCaption() const {
    std::string text = _caption;
    if (_postCaption.size() > 0) {
        text += " (" + _postCaption + ")";
    }

    SDL_WM_SetCaption(text.c_str(), _iconPath.c_str());
}

void Window::setPostCaption(const std::string &caption) {
    _postCaption = caption;
    updateCaption();
}

void Window::setCaption(const string &caption, const string &iconPath) {
    _caption = caption;
    _iconPath = iconPath;
    updateCaption();
}

void Window::swapBuffers() const {
    SDL_GL_SwapBuffers();
}

