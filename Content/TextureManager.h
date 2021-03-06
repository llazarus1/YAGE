/*
 *  TextureManager.h
 *  Engine
 *
 *  Created by Brent Wilson on 4/22/07.
 *  Copyright 2007 Brent Wilson. All rights reserved.
 *
 */

#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_
#include "ResourceManager.h"
#include "Singleton.h"
#include "Texture.h"

/*! \brief Provides the functionality for Texture caching and Loading. */
class TextureManager : public ResourceManager<Texture> {
public:
    TextureManager(ResourceGroupManager *manager);
    ~TextureManager();

    Texture *createTexture(const std::string &name, int frames = 1);
    Texture *createBlankTexture(const std::string &name, GLenum internal, int w, int h = 1, int d = 1);
    Texture *createRandomTexture(const std::string &name, int w, int h = 1, int d = 1);

};

#endif
