/*
 *  Material.h
 *  System
 *
 *  Created by loch on 1/6/10.
 *  Copyright 2010 Brent Wilson. All rights reserved.
 *
 */

#ifndef _MATERIAL_H_
#define _MATERIAL_H_
#include <Base/Math3D.h>
#include <Base/Vector.h>
#include <Render/Texture.h>

///\todo Look into making a parent class for everything that needs set/get ambient/diffuse/specular
class Shader;
class ShaderManager;

class Material {
public:
    Material(ShaderManager *shaderManager = NULL);
    virtual ~Material();

    void setTransparent(bool value);
	void setColor(Real r, Real g, Real b, Real a);
    void setAmbient(Real r, Real g, Real b, Real a = 1.0f);
    void setDiffuse(Real r, Real g, Real b, Real a = 1.0f);
	void setTexture(Texture *t, int level = 0);
    void loadShader(std::string shader);
    
    void enableMaterial() const;
    void disableMaterial() const;

    bool getTransparent() const;
	Texture *getTexture(int level = 0) const;
	Vector4 *getColor() const;
	Vector4 *getAmbient() const;
	Vector4 *getDiffuse() const;
	
private:
	Vector4 _color;
	Vector4 _ambient;
	Vector4 _diffuse;
	Texture *_texture[8];
    Shader *_materialShader;
    ShaderManager *_shaderManager;

    bool _transparent;
};

#endif
