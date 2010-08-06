/*
 *  ChunkedTerrainGroup.cpp
 *  Mountainhome
 *
 *  Created by loch on 7/12/10.
 *  Copyright 2010 Mountainhome Project. All rights reserved.
 *
 */

#include "ChunkedTerrainGroup.h"
#include "BlockChunkedTerrainModel.h"

#include "TileGrid.h"
#include "OctreeSceneManager.h"

#include <Render/MaterialManager.h>
#include <Render/Entity.h>

#define GET_CHUNK_INDEX(x, y, z) ((((x) / ChunkSize) << (BitsPerDim * 2)) | (((y) / ChunkSize) << BitsPerDim) | ((z) / ChunkSize))

ChunkedTerrainGroup::ChunkedTerrainGroup(TileType type, TileGrid *grid,
OctreeSceneManager *scene, Material *material): _type(type), _grid(grid),
_sceneManager(scene), _material(material) {}

ChunkedTerrainGroup::~ChunkedTerrainGroup() {}

// Assumptions: The counting system only works if this method is only called on a change!!
void ChunkedTerrainGroup::update(int x, int y, int z, bool doPolyReduction) {
    // Create and update the chunk for loc x, y, z.
    createChunkIfNeeded(x, y, z);
    updateIfExists(x, y, z, doPolyReduction);

    // Lastly, update the surrounding chunks as needed.
    if (x % ChunkSize == 0            ) { updateIfExists(x - 1, y, z, doPolyReduction); }
    if (x % ChunkSize == ChunkSize - 1) { updateIfExists(x + 1, y, z, doPolyReduction); }
    if (y % ChunkSize == 0            ) { updateIfExists(x, y - 1, z, doPolyReduction); }
    if (y % ChunkSize == ChunkSize - 1) { updateIfExists(x, y + 1, z, doPolyReduction); }
    if (z % ChunkSize == 0            ) { updateIfExists(x, y, z - 1, doPolyReduction); }
    if (z % ChunkSize == ChunkSize - 1) { updateIfExists(x, y, z + 1, doPolyReduction); }
}

void ChunkedTerrainGroup::createChunkIfNeeded(int x, int y, int z) {
    // Don't create a chunk for the far outer edge of the grid. This only exists to
    // give the chunk it borders context and to mesh correctly with terrain generation.
    if (x == _grid->getWidth()  - 1 ||
        y == _grid->getHeight() - 1 ||
        z == _grid->getDepth()  - 1)
    {
        return;
    }

    IndexType chunkIndex = GET_CHUNK_INDEX(x, y, z);

    // Instantiate a new ChunkedTerrainModel if it doesn't already exist.
    if (_chunks.find(chunkIndex) == _chunks.end()) {
        // Create a new terrain model.
        ChunkedTerrainModel *model = new BlockChunkedTerrainModel(_grid, _type,
            x / ChunkSize, y / ChunkSize, z / ChunkSize);

        // Create an entity in the scene manager for the model and assign a texture.
        Entity *entity = _sceneManager->create<Entity>(model->getName());
        entity->setMaterial(_material);
        entity->setModel(model);

        // Save the model in the chunks map.
        _chunks[chunkIndex] = model;
    }
}

void ChunkedTerrainGroup::updateAll(bool doPolyReduction) {
    for (int x = 0; x < _grid->getWidth(); x+=ChunkSize) {
        for (int y = 0; y < _grid->getHeight(); y+=ChunkSize) {
            for (int z = 0; z < _grid->getDepth(); z+=ChunkSize) {
                createChunkIfNeeded(x, y, z);
                updateIfExists(x, y, z, doPolyReduction);
            }
        }
    }
}

void ChunkedTerrainGroup::clear() {
    ChunkLookupMap::iterator itr;
    for (itr = _chunks.begin(); itr != _chunks.end(); itr++) {
        removeChunk(itr);
    }
}

void ChunkedTerrainGroup::removeChunk(ChunkLookupMap::iterator itr) {
    _sceneManager->destroy<Entity>(itr->second->getName());
    delete itr->second;
    _chunks.erase(itr);
}

void ChunkedTerrainGroup::updateIfExists(int x, int y, int z, bool doPolyReduction) {
    // Verify the x/y/z set is within the bounds of the grid.
    if (!_grid->isOutOfBounds(x, y, z)) {
        IndexType chunkIndex = GET_CHUNK_INDEX(x, y, z);
        ChunkLookupMap::iterator chunkItr = _chunks.find(chunkIndex);
        if (chunkItr != _chunks.end()) {
            if (chunkItr->second->update(doPolyReduction) == 0) {
                removeChunk(chunkItr);
            }
        }
    }
}
