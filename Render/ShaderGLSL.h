/*
 *  ShaderGLSL.h
 *  Mountainhome
 *
 *  Created by loch on 3/21/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#ifndef _SHADERGLSL_H_
#define _SHADERGLSL_H_
#include <Base/PTreeResourceFactory.h>
#include "GL_Helper.h"
#include "Shader.h"

class ShaderGLSL : public Shader {
public:
    class Factory : public PTreeResourceFactory<Shader> {
    public:
        Factory();
        virtual ~Factory();
        bool canLoad(const std::string &args);
        Shader* load(const std::string &args);
    };

protected:
    friend class ShaderManager;
    template <typename Resource> friend class ResourceManager;
    
    ShaderGLSL(const std::string &vertexString, const std::string &fragmentString);
    virtual ~ShaderGLSL();
    
    void setVertex(GLhandleARB program);
    void setFragment(GLhandleARB program);
    void setProgram(GLhandleARB program);
    void unload();    

public:
    static Shader* Load(const std::string &vertFilename, const std::string &fragFilename);
    
    //Functions    
    void on();
    void off();
    bool hasVertexShader();
    bool hasFragmentShader();
    
    //Shader Accessors
    void setTexture(const std::string &strVariable, int newValue);

    void setInt(const std::string &strVariable, int newValue);
    void setIntArray(const std::string &strVariable, int size, int *newValue);

    void setFloat(const std::string &strVariable, float newValue);
    void setFloatArray(const std::string &strVariable, int size, float *newValue);

private:
    GLhandleARB _vertexShader;
    GLhandleARB _fragmentShader;
    GLhandleARB _programHandle;
    
    int getVariable(const std::string &strVariable) const;

};

#endif
