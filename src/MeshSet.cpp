#include "MeshSet.h"


void MeshSet::AddMesh(const Mesh &mesh) {
    meshes.push_back(mesh);
}


void MeshSet::Clear() {
    meshes.clear();
}