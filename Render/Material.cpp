/*
 *  Material.cpp
 *  System
 *
 *  Created by loch on 1/6/10.
 *  Copyright 2010 Brent Wilson. All rights reserved.
 *
 */

#include "Material.h"
#include "ShaderManager.h"
#include "Shader.h"

Material::Material(ShaderManager *shaderManager): _color(1.0f, 1.0f, 1.0f, 1.0f), _materialShader(NULL), _shaderManager(shaderManager), _transparent(false) {
    int c;
    for(c=0; c < 8; c++) {
        _texture[c] = NULL;
    }
}

Material::~Material() {}

void Material::setTransparent(bool value) {
    _transparent = value;
}

void Material::setColor(Real r, Real g, Real b, Real a) {
	_color.r = r;
	_color.g = g;
	_color.b = b;
	_color.a = a;
}

void Material::setAmbient(Real r, Real g, Real b, Real a) {
	_ambient.r = r;
	_ambient.g = g;
	_ambient.g = b;
}

void Material::setDiffuse(Real r, Real g, Real b, Real a) {
	_diffuse.r = r;
	_diffuse.g = g;
	_diffuse.b = b;
}

void Material::setTexture(Texture *t, int level) {
	_texture[level] = t;
}

void Material::loadShader(std::string shader) {
    _shaderManager->getOrLoadResource(shader);
}

void Material::enableMaterial() const {
	glColor4f(_color.r, _color.g, _color.b, _color.a);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&_diffuse);

    if (_transparent) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }

	// Set up textures
	if (_texture[0]) { 
        int c;
        for(c=8; c >= 0; --c) {
            if(_texture[c]) {
                Info("Binding texture " << c);
                (_texture[c])->bindAndEnable(c);
            }
        }
    }
	else { glDisable(GL_TEXTURE_2D); }
    
    if(_materialShader) {
        _materialShader->on();
    }
}

// Likely called from MHUIElement.cpp, Entity.cpp
void Material::disableMaterial() const {
    if(_materialShader) {
        _materialShader->off();
    }
    
    if(_texture[0]) {
        int c;
        for(c=8; c >= 0; --c) {
            if(_texture[c]) {
                Info("Releasing texture " << c);
                (_texture[c])->releaseAndDisable(c);
            }
        }
    }
}

bool Material::getTransparent() const {
    return _transparent;
}

Texture* Material::getTexture(int level) const {
	return _texture[level];
}

Vector4* Material::getAmbient() const {
	return (Vector4*)&_ambient;
}

Vector4* Material::getDiffuse() const {
	return (Vector4*)&_diffuse;
}

Vector4* Material::getColor() const {
	return (Vector4*)&_color;
}
