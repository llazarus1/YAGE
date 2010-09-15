/*
 *  ModelFBX.h
 *  Engine
 *
 *  Created by Andrew Jean on on 9/12/10.
 *
 */

#ifndef _MODELFACTORYFBX_H_
#define _MODELFACTORYFBX_H_
#include <Base/ResourceManager.h>
#include "Model.h"
#include <fbxsdk.h>

class ModelFBX : public Model {
public:
    class Factory : public ResourceFactory<Model> {
    public:
        Factory(ResourceGroupManager *manager);
        virtual ~Factory();
        bool canLoad(const std::string &args);
        Model* load(const std::string &args);

        // Extracts relevant data from the imported scene
        bool parseSceneNode(KFbxNode *node, ModelFBX *model);

        // Converts a KFbxMesh into a ModelMeshPart and queues it up
        bool parseMesh(KFbxMesh *mesh, ModelFBX *model);

        // Converts KFbxSurfaceMaterials into Materials and returns them in a vector
        bool parseMaterials(KFbxNode *node, ModelFBX *model, std::vector<Material*> *matList);

    private:
        KFbxSdkManager* _sdkManager;
        KFbxImporter* _importer;
        KFbxScene* _scene;
    };

protected:
    ModelFBX();
    virtual ~ModelFBX() {}

    void addMeshPart(std::vector<Vector3> *verts, std::vector<Vector3> *norms, std::vector<Vector2> *texCoords, std::vector<unsigned int> *indices);
    void internVectors();

private:
    std::vector <Vector3> _mutableVerts;
    std::vector <Vector3> _mutableNorms;
    std::vector <Vector2> _mutableTexCoords;

    std::vector <unsigned int> _mutableIndices;
    std::vector <ModelMeshPart> _mutableMeshParts;
};

#endif
