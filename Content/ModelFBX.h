/*
 *  ModelFBX.h
 *  Engine
 *
 *  Created by Andrew Jean on on 9/12/10.
 *
 */

#ifndef _MODELFACTORYFBX_H_
#define _MODELFACTORYFBX_H_
#include "ResourceManager.h"
#include "Model.h"

#pragma GCC diagnostic ignored "-Wall"
#include <fbxsdk.h>
#pragma GCC diagnostic warning "-Wall"

class TextureManager;
class VertexArray;

class ModelFBXFactory : public ResourceFactory<Model> {
public:
    ModelFBXFactory(ResourceGroupManager *manager, TextureManager *tManager);
    virtual ~ModelFBXFactory();
    bool canLoad(const std::string &args);
    Model* load(const std::string &args);

private:
    /*! Extracts relevant data from the imported scene and builds a set of ModelMeshes,
     *  returning the new objects in a vector. */
    void buildModelMeshesFromScene(const std::string &name, KFbxNode *node, std::vector<ModelMesh *> &meshes);

    /*! Converts a KFbxMesh into a ModelMeshPart, translating appropriate attribures using
     *  by the affine transformation details gathered from its node. */
    ModelMesh * fbxMeshToModelMesh(const std::string &name, KFbxMesh *mesh);

    /*! Converts KFbxSurfaceMaterials into Materials and returns them in a vector */
    void parseMaterialsFromNode(const std::string &name, KFbxNode *node, std::vector<Material*> &matList);

    /*! Converts an FBX matrix into a Mountainhome Matrix */
    void convertMatrix(KFbxXMatrix *matrix, Matrix &mhMatrix);

    VertexArray * buildVertexArray();

private:
    TextureManager *_textureManager;
    KFbxSdkManager *_sdkManager;
    KFbxImporter *_importer;
    KFbxScene *_scene;

};

#endif
