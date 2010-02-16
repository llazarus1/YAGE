/*
 *  TextureLoaderSDL.cpp
 *  Base
 *
 *  Created by Brent Wilson on 11/15/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 *  TextureLoaderSDL is an image loader wrapper class for Texture
 *
 */

#include "SDL_Helper.h"
#include "TextureLoaderSDL.h"
#include "TextureManager.h"
#include "Texture.h"

#include <Base/FileSystem.h>
#include <Base/Assertion.h>
#include <Base/Logger.h>

SDL_Surface *readTextureLoaderSDL(const std::string &name, TextureManager::PixelData *data) {
    SDL_Surface *surface;
    std::string fullName;
    if (!FileSystem::GetReadName(name, fullName)) {
        Error("TextureManager: Could not find the given file.");
        Error("  Given name : " << name);
        Error("  Search name: " << fullName);
        return NULL;
    }

    if (!(surface = IMG_Load(fullName.c_str()))) {
        Error("TextureManager: Could not open: " << fullName);
        return NULL;
    }
    
    if (surface->format->palette) {
        //TODO add support for loading palette images. It could be neat.
        Error("TextureManager: " << fullName << " is an indexed image");
        SDL_FreeSurface(surface);
        return NULL;
    }

    if (data) {
        data->pixels = surface->pixels;
        data->type = GL_UNSIGNED_BYTE;
        if (surface->format->BitsPerPixel == 24) { data->format = GL_RGB; }
        else if(surface->format->BitsPerPixel == 32) { data->format = GL_RGBA; }
        else {
            SDL_FreeSurface(surface);
            Error("TextureManager: Unknown format");
            return NULL;
        }

        data->internal = data->format;
    }

    return surface;
}

bool TextureLoaderSDL::CanLoad(const std::string &name) {
    std::string ext;
    FileSystem::ExtractExtension(name, ext);
    return ext == "bmp" || ext == "pnm" || ext == "xpm" || ext == "lbm" || ext == "gif" ||
           ext == "gif" || ext == "png" || ext == "tga" || ext == "jpg" || ext == "jpeg"||
           ext == "tiff";
}

Texture *TextureLoaderSDL::Load(const std::string &name) {
    TextureManager::PixelData data;
    SDL_Surface *surface = readTextureLoaderSDL(name, &data);
    FlipSDLPixels(surface);

    Texture *result = NULL;
    if (surface) {
        if (surface->h == 1) {
            result = TextureManager::Get()->init1D(name, surface->w);
            TextureManager::Get()->tex1D(result, data);
        } else {
            result = TextureManager::Get()->init2D(name, surface->w, surface->h);
            TextureManager::Get()->tex2D(result, data);
        }

        SDL_FreeSurface(surface);
    }

    return result;
}

Texture* TextureLoaderSDL::LoadCubeMap(const std::string &name,
                                 const std::string files[6]) {
    if (!CanLoad(name)) {
        Error("Texture format unsupported for cube maps: " << name);
        return NULL;
    }

    TextureManager::PixelData data;
    Texture *result = NULL;

    for (int i = 0; i < 6; i++) {
        SDL_Surface *surface = readTextureLoaderSDL(files[i], &data);
        if (result = NULL) {
            result = TextureManager::Get()->initCube(name, surface->w, surface->h);        
        }

        TextureManager::Get()->texCube(result, data, i + GL_TEXTURE_CUBE_MAP_NEGATIVE_X, -1, 1);
        SDL_FreeSurface(surface);
    }

    return result;
}

Texture* TextureLoaderSDL::LoadAnimated(const std::string &name, int n,
                                  const std::string *files) {
    if (!CanLoad(name)) {
        Error("Texture format unsupported for animated textures: " << name);
        return NULL;
    }

    TextureManager::PixelData data;
    Texture *result = NULL;

    for (int i = 0; i < 6; i++) {
        SDL_Surface *surface = readTextureLoaderSDL(files[i], &data);
        if (result = NULL) {
            result = TextureManager::Get()->init2D(name, n, surface->w, surface->h);
        }

        TextureManager::Get()->tex2D(result, data, -1, n);
        SDL_FreeSurface(surface);
    }

    return result;
}
