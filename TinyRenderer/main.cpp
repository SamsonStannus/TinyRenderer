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
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(0  , 255,   0, 255);
const TGAColor blue  = TGAColor(0  ,   0, 255, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color);
void line(int x0, int y0, int x1, int y1,TGAImage &image, TGAColor color);
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void renderModel(Model *model, TGAImage &image, TGAColor color);

int main(int argc, char** argv) {
    
//    if (2 == argc)
//    {
//        model = new Model(argv[1]);
//    }
//    else
//    {
//        model = new Model("../../../obj/diablo3_pose/diablo3_pose.obj");
//    }
    TGAImage image(width, height, TGAImage::RGB);
    
//    renderModel(model, image, white);
    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);
    
    image.flip_vertically();
    std::cout << "Writing File\n";
    image.write_tga_file("../../../output.tga");
    return 0;
}

// Draws a line segment between (x0, y0) and (x1, y1)
void line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    // check if the change in y is greater than the change in x
    // if so, we'll interpolate the y-axis as to not have gaps
    // in the final line.
    if (std::abs(v1.x - v0.x) < std::abs(v1.y - v0.y))
    {
        std::swap(v0.x, v0.y);
        std::swap(v1.x, v1.y);
        steep = true;
    }
    // check if the second point is actually first when scanning
    // left-to-right, if so switch the first and second points.
    if (v1.x < v0.x)
    {
        std::swap(v0, v1);
    }
    
    int dx = (v1.x - v0.x);
    int dy = (v1.y - v0.y);
    int derror = std::abs(dy)*2;
    int error = 0;
    int y = v0.y;
    for (int x = v0.x; x < v1.x; x++)
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
            y += (v1.y > v0.y ? 1 : -1);
            error -= dx*2;
        }
    }
}

void line(int x0, int y0, int x1, int y1,TGAImage &image, TGAColor color)
{
    Vec2i v0 {x0, y0};
    Vec2i v1 {x1, y1};
    line(v0, v1, image, color);
}

// Draws a triangle
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t0.y == t1.y && t1.y == t2.y) return;
    // Sort the points, t0, t1, t2, lower-to-upper (bubble sort)
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);
    int triangleHeight = t2.y - t0.y;
    for (int i = 0; i < triangleHeight; i++)
    {
        bool secondHalf = i > t1.y - t0.y || t1.y == t0.y;
        int segmentHeight = secondHalf ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float) i / triangleHeight;
        float beta  = (float) (i - (secondHalf ? t1.y - t0.y : 0)) / segmentHeight;
        Vec2i A =              t0 + (t2 - t0) * alpha;
        Vec2i B = secondHalf ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int j = A.x; j <= B.x; j++)
        {
            image.set(j, t0.y + i, color);
        }
        
    }
}

// Renders a Model using lines
void renderModel(Model *model, TGAImage &image, TGAColor color)
{
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
            line(x0, y0, x1, y1, image, color);
        }
    }
}

