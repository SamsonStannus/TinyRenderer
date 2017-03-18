//
//  main.cpp
//  TinyRenderer
//
//  Created by samson stannus on 2017-03-10.
//  Copyright (c) 2017 samson stannus. All rights reserved.
//

#include <vector>
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
void triangle(Vec2i *pts, TGAImage &image, TGAColor color);
void renderWireFrame(Model *model, TGAImage &image, TGAColor color);
void render(Model *model, TGAImage &image, TGAColor color);
Vec3f cross(Vec3f v1, Vec3f v2);
Vec3f barycentric(Vec2i *pts, Vec2i P);

int main(int argc, char** argv) {
    
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("../../../obj/african_head/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);
    
//    renderWireFrame(model, image, white);
    render(model, image, blue);
    
    
//    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
//    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
//    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
//    triangle(t0[0], t0[1], t0[2], image, red);
//    triangle(t1[0], t1[1], t1[2], image, white);
//    triangle(t2[0], t2[1], t2[2], image, green);
    
//    Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)};
//    triangle(pts, image, green);
    
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

void triangle(Vec2i *pts, TGAImage &image, TGAColor color)
{
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin.raw[j] = std::max(0,            std::min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = std::min(clamp.raw[j], std::max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    Vec2i P;
    for (P.x = bboxmin.x; P.x < bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y < bboxmax.y; P.y++)
        {
            Vec3f bcScreen = barycentric(pts, P);
            if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0) continue;
            std::cout << P << std::endl;
            image.set(P.x, P.y, color);
        }
    }
    
    
}

// Renders a Model using lines
void renderWireFrame(Model *model, TGAImage &image, TGAColor color)
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

void render(Model *model, TGAImage &image, TGAColor color)
{
    Vec3f lightDir(0,0,-1);
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screenCoords[3];
        Vec3f worldCoords[3];
        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            screenCoords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
            worldCoords[j] = v;
        }
        Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * lightDir;
        if (intensity > 0 )
        {
            triangle(screenCoords[0], screenCoords[1], screenCoords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }
}

Vec3f cross(Vec3f v1, Vec3f v2)
{
    return Vec3f(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

Vec3f barycentric(Vec2i *pts, Vec2i P)
{
    Vec3f x = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x);
    Vec3f y = Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
    Vec3f u = cross(x, y);
    if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1); // triangle is degenerate return a negative point.
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

