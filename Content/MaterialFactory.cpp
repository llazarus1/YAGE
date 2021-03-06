/*
 *  MaterialFactory.cpp
 *  Mountainhome
 *
 *  Created by loch on 1/20/11.
 *  Copyright 2011 Mountainhome Project. All rights reserved.
 *
 */

#include <Base/FileSystem.h>
#include "MaterialFactory.h"
#include "ShaderManager.h"
#include "TextureManager.h"

#include "BasicMaterial.h"

MaterialFactory::MaterialFactory(ResourceGroupManager *rManager, ShaderManager *sManager, TextureManager *tManager):
    PTreeResourceFactory<Material>(rManager),
    _shaderManager(sManager),
    _textureManager(tManager)
{
    _keywords.push_back("type");
    _keywords.push_back("shader");
    _keywords.push_back("transparent");
    _keywords.push_back("depthtest");
    _keywords.push_back("wireframe");
}

MaterialFactory::~MaterialFactory() {}

bool MaterialFactory::canLoad(const std::string &name) {
    std::string ext;
    FileSystem::ExtractExtension(name, ext);
    return ext == "material";
}

template <>
ShaderParameter * MaterialFactory::parseFloat<1>(const std::vector<std::string> &tokens) {
    // We MUST malloc here so ShaderParameter can clean up after us.
    float *value = (float*)malloc(sizeof(float));
    *value = strtod(tokens[1].c_str(), (char **)NULL);

    ShaderParameter *result = new ShaderParameter();
    result->setData(value, 1, true);
    return result;
}

template <>
ShaderParameter * MaterialFactory::parseInt<1>(const std::vector<std::string> &tokens) {
    // We MUST malloc here so ShaderParameter can clean up after us.
    int *value = (int*)malloc(sizeof(int));
    *value = strtol(tokens[1].c_str(), (char **)NULL, 10);

    ShaderParameter *result = new ShaderParameter();
    result->setData(value, 1, true);
    return result;
}

ShaderParameter* MaterialFactory::parseShaderParameter(const std::string &input) {
    // Break the string into tokens.
    std::vector<std::string> tokens;
    tokenize(input, " ", tokens);

    if (tokens[0] == "float") {
        switch(tokens.size()) {
        case 2: return parseFloat<1>(tokens);
        case 3: return parseFloat<2>(tokens);
        case 4: return parseFloat<3>(tokens);
        case 5: return parseFloat<4>(tokens);
        default: THROW(InternalError, "Unhandled case value: " << tokens.size());
        }
    } else if (tokens[0] == "int") {
        switch(tokens.size()) {
        case 2: return parseInt<1>(tokens);
        case 3: return parseInt<2>(tokens);
        case 4: return parseInt<3>(tokens);
        case 5: return parseInt<4>(tokens);
        default: THROW(InternalError, "Unhandled case value: " << tokens.size());
        }
    } else if (tokens[0] == "texture") {
        ShaderParameter *result = new ShaderParameter();
        result->setData(_textureManager->getOrLoadResource(tokens[1]));
        return result;
    }

    THROW(NotImplementedError, "This data type has not been implemented for loading from .material files.");
}

void MaterialFactory::setGenericParameters(Material *mat) {
    boost::property_tree::ptree::const_iterator itr;
    std::list<std::string>::iterator keywordItr;

    for (itr = _ptree.begin(); itr != _ptree.end(); itr++) {
        bool generic = true;
        for (keywordItr = _keywords.begin(); keywordItr != _keywords.end(); keywordItr++) {
            if (itr->first == *keywordItr) {
                generic = false;
                break;
            }
        }

        if (generic) {
            std::string value = itr->second.get_value<std::string>();
            mat->setShaderParameter(itr->first, parseShaderParameter(value));
        }
    }
}

Material* MaterialFactory::load(const std::string &name) {
    Material *mat = NULL;
    // If a shader is not set, we have a BasicMaterial, otherwise a generic Material.
    if (_ptree.find("shader") == _ptree.not_found()) {
        Texture *texture = NULL;
        if (_ptree.find("texture") != _ptree.not_found()) {
            texture = _textureManager->getOrLoadResource(_ptree.get<std::string>("texture"));
        }

        BasicMaterial *basic = new BasicMaterial();
        basic->setAmbient(_ptree.get<Vector4>("ambient",Vector4(1.0f)));
        basic->setDiffuse(_ptree.get<Vector4>("diffuse",Vector4(1.0f)));
        basic->setLightingEnabled(_ptree.get<bool>("lighting", false));
        basic->setTexture(texture);

        mat = basic;
    } else {
        std::string name = _ptree.get<std::string>("shader");
        Shader *shader = _shaderManager->getOrLoadResource(name);

        if (!shader) {
            THROW(InternalError, "Could not load a shader named: " << name);
        }

        mat = new Material(name);
        mat->setShader(shader);
        setGenericParameters(mat);
    }

    // Only set the values if they're explicitely specified, otherwise go with the default.
    if (_ptree.find("transparent") != _ptree.not_found()) { mat->setTransparency(_ptree.get<int>("transparent", 0)); }
    if (_ptree.find("depthtest")   != _ptree.not_found()) { mat->setDepthTest(   _ptree.get<int>("depthtest",   0)); }
    if (_ptree.find("wireframe")   != _ptree.not_found()) { mat->setWireframe(   _ptree.get<int>("wireframe",   0)); }

    return mat;
}
