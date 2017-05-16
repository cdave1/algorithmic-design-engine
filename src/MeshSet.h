#ifndef _MESHSET_H_
#define _MESHSET_H_

#include <vector>
#include "Mesh.h"

class MeshSet {
public:

    std::vector<Mesh> meshes;

public:

    static MeshSet * LoadFromSVG(const std::string &path);

    void AddMesh(const Mesh &mesh);

    void Clear();

};

#endif
