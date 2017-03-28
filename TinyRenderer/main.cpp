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
void triangle(Vec3f *pts, float *zBuffer, TGAImage &image, TGAColor color);
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);
Vec3f worldToScreen(Vec3f v);
void render(Model *model, TGAImage &image, TGAColor color);
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void renderWireFrame(Model *model, TGAImage &image, TGAColor color);
void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]);


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
    
    
    

    render(model, image, white);
    
//    renderWireFrame(model, image, blue);
    
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


void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            if (zbuffer[int(P.x+P.y*width)] < P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

Vec3f worldToScreen(Vec3f v) {
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

void render(Model *model, TGAImage &image, TGAColor color)
{
    Vec3f light_dir(0, 0, -1);
    float *zBuffer = new float[width * height];
    for (int i = width * height; i--; zBuffer[i] = -std::numeric_limits<float>::max());
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f pts[3];
        Vec3f worldCoords[3];
        for (int i = 0; i < 3; i++)
        {
            Vec3f v = model->vert(face[i]);
            pts[i] = worldToScreen(v);
            worldCoords[i] = v;
        }
        Vec3f n = cross(worldCoords[2] - worldCoords[0], worldCoords[1] - worldCoords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0)
        {
            triangle(pts, zBuffer, image, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255));
        }
    }
}

//// Draws a triangle
//void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
//{
//    if (t0.y == t1.y && t1.y == t2.y) return;
//    // Sort the points, t0, t1, t2, lower-to-upper (bubble sort)
//    if (t0.y > t1.y) std::swap(t0, t1);
//    if (t0.y > t2.y) std::swap(t0, t2);
//    if (t1.y > t2.y) std::swap(t1, t2);
//    int triangleHeight = t2.y - t0.y;
//    for (int i = 0; i < triangleHeight; i++)
//    {
//        bool secondHalf = i > t1.y - t0.y || t1.y == t0.y;
//        int segmentHeight = secondHalf ? t2.y - t1.y : t1.y - t0.y;
//        float alpha = (float) i / triangleHeight;
//        float beta  = (float) (i - (secondHalf ? t1.y - t0.y : 0)) / segmentHeight;
//        Vec2i A =              t0 + (t2 - t0) * alpha;
//        Vec2i B = secondHalf ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
//        if (A.x > B.x) std::swap(A, B);
//        for (int j = A.x; j <= B.x; j++)
//        {
//            image.set(j, t0.y + i, color);
//        }
//
//    }
//}

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

//void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[])
//{
//    if (p0.x > p1.x) std::swap(p0, p1);
//    for (int x = p0.x; x < p1.x; x++)
//    {
//        float t = (x - p0.x)/(float)(p1.x - p0.x);
//        int y = p0.y * (1. - t) + p1.y * t;
//        if (yBuffer[x] < y)
//        {
//            yBuffer[x] = y;
//            for (int i = 0; i < 16; i++) {
//                image.set(x, i, color);
//            }
//
//        }
//    }
//}

