//
//  model.h
//  TinyRenderer
//
//  Created by samson stannus on 2017-03-12.
//  Copyright (c) 2017 samson stannus. All rights reserved.
//

#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model
{
private:
    std::vector<Vec3f>            verts_;
    std::vector<std::vector<int>> faces_;
    
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    std::vector<int> face(int idx);
    Vec3f vert(int i);
};



#endif //__MODEL_H__
