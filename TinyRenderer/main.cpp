//
//  main.cpp
//  TinyRenderer
//
//  Created by samson stannus on 2017-03-10.
//  Copyright (c) 2017 samson stannus. All rights reserved.
//

#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor blue  = TGAColor(0  ,   0, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

int main(int argc, char** argv) {
    
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("../../../TinyRenderer/head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;
            line(x0, y0, x1, y1, image, blue);
        }
    }
    
    image.flip_vertically();
    std::cout << "Writing File\n";
    image.write_tga_file("../../../output.tga");
    return 0;
}

// Draws a line segment between (x0, y0) and (x1, y1)
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    // check if the change in y is greater than the change in x
    // if so, we'll interpolate the y-axis as to not have gaps
    // in the final line.
    if (std::abs(x1-x0) < std::abs(y1-y0))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    // check if the second point as actually first when scanning
    // left-to-right, if so switch the first and second points.
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    
    int dx = (x1 - x0);
    int dy = (y1 - y0);
    int derror = std::abs(dy)*2;
    int error = 0;
    int y = y0;
    for (int x = x0; x < x1; x++)
    {
        if (steep)
        {
            image.set(y, x, color); // if transposed, de-transpose.
        }
        else
        {
            image.set(x, y, color);
        }
        error += derror;
        if (error > dx)
        {
            y += (y1 > y0 ? 1 : -1);
            error -= dx*2;
        }
    }
}

