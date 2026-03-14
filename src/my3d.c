#include "my3d.h"
#include <stdio.h>
#include <stdint.h>
#include "constants.h"
#include "raymath.h"

Model LoadMy3D(const char *fileName) {
    char path[256];
    snprintf(path, sizeof(path), "MeshData/%s.my3d", fileName);
    
    FILE *f = fopen(path, "rb");
    if (!f) {
        TraceLog(LOG_ERROR, "LoadMy3D: '%s' NOT FOUND at '%s'", fileName, path);
        return (Model){0};
    }
    
    TraceLog(LOG_INFO, "LoadMy3D: Loading '%s'...", path);
    
    int surfaces = 0;
    if (fread(&surfaces, sizeof(int32_t), 1, f) != 1) {
        fclose(f);
        return (Model){0};
    }
    
    Model model = {0};
    model.meshCount = surfaces;
    model.meshes = (Mesh *)MemAlloc(surfaces * sizeof(Mesh));
    model.meshMaterial = (int *)MemAlloc(surfaces * sizeof(int));
    model.materialCount = 1;
    model.materials = (Material *)MemAlloc(sizeof(Material));
    model.materials[0] = LoadMaterialDefault();
    
    float scale = OSCALE;
    
    for (int s = 0; s < surfaces; s++) {
        Mesh *mesh = &model.meshes[s];
        *mesh = (Mesh){0};
        
        int vertices = 0;
        fread(&vertices, sizeof(int32_t), 1, f);
        
        mesh->vertexCount = vertices;
        mesh->vertices = (float *)MemAlloc(vertices * 3 * sizeof(float));
        mesh->normals = (float *)MemAlloc(vertices * 3 * sizeof(float));
        mesh->colors = (unsigned char *)MemAlloc(vertices * 4 * sizeof(unsigned char));
        mesh->texcoords = (float *)MemAlloc(vertices * 2 * sizeof(float));
        
        for (int v = 0; v < vertices; v++) {
            float x, y, z;
            fread(&x, sizeof(float), 1, f);
            fread(&y, sizeof(float), 1, f);
            fread(&z, sizeof(float), 1, f);
            
            float nx, ny, nz;
            fread(&nx, sizeof(float), 1, f);
            fread(&ny, sizeof(float), 1, f);
            fread(&nz, sizeof(float), 1, f);
            
            float r, g, b;
            fread(&r, sizeof(float), 1, f);
            fread(&g, sizeof(float), 1, f);
            fread(&b, sizeof(float), 1, f);
            
            float u, v_tex, w;
            fread(&u, sizeof(float), 1, f);
            fread(&v_tex, sizeof(float), 1, f);
            fread(&w, sizeof(float), 1, f);
            
            // Coordinate System Flip: Left-Handed -> Right-Handed (Z invert)
            z = -z;
            nz = -nz;
            
            // Apply scale
            x *= scale;
            y *= scale;
            z *= scale;
            
            mesh->vertices[v*3 + 0] = x;
            mesh->vertices[v*3 + 1] = y;
            mesh->vertices[v*3 + 2] = z;
            
            mesh->normals[v*3 + 0] = nx;
            mesh->normals[v*3 + 1] = ny;
            mesh->normals[v*3 + 2] = nz;
            
            // Ensure color clamps to 0-255
            mesh->colors[v*4 + 0] = (unsigned char)(r < 0.0f ? 0 : (r > 255.0f ? 255 : (int)r));
            mesh->colors[v*4 + 1] = (unsigned char)(g < 0.0f ? 0 : (g > 255.0f ? 255 : (int)g));
            mesh->colors[v*4 + 2] = (unsigned char)(b < 0.0f ? 0 : (b > 255.0f ? 255 : (int)b));
            mesh->colors[v*4 + 3] = 255;
            
            mesh->texcoords[v*2 + 0] = u;
            mesh->texcoords[v*2 + 1] = v_tex;
        }
        
        int triangles = 0;
        fread(&triangles, sizeof(int32_t), 1, f);
        
        mesh->triangleCount = triangles;
        mesh->indices = (unsigned short *)MemAlloc(triangles * 3 * sizeof(unsigned short));
        
        for (int t = 0; t < triangles; t++) {
            int v0, v1, v2;
            fread(&v0, sizeof(int32_t), 1, f);
            fread(&v1, sizeof(int32_t), 1, f);
            fread(&v2, sizeof(int32_t), 1, f);
            
            // Reverse winding for Z-flip (v0, v1, v2) -> (v0, v2, v1)
            mesh->indices[t*3 + 0] = (unsigned short)v0;
            mesh->indices[t*3 + 1] = (unsigned short)v2;
            mesh->indices[t*3 + 2] = (unsigned short)v1;
        }
        
        UploadMesh(mesh, false);
        model.meshMaterial[s] = 0;
    }
    
    float posX = 0, posY = 0;
    // End values (can be missing gracefully)
    if (fread(&posX, sizeof(float), 1, f) == 1) {
        fread(&posY, sizeof(float), 1, f);
    }
    
    fclose(f);
    
    model.transform = MatrixTranslate(posX * scale, posY * scale, 0); // Scale positional offset too
    
    return model;
}

Model LoadMy3DState(const char *fileName, int state) {
    Model m = LoadMy3D(fileName);
    if (m.meshCount == 0) return m;

    for (int s = 0; s < m.meshCount; s++) {
        Mesh *mesh = &m.meshes[s];
        
        for (int v = 0; v < mesh->vertexCount; v++) {
            unsigned char r = mesh->colors[v*4 + 0];
            unsigned char g = mesh->colors[v*4 + 1];
            unsigned char b = mesh->colors[v*4 + 2];
            
            if (state == 1) { // Infected
                // Shift towards violet/dark purple
                mesh->colors[v*4 + 0] = (unsigned char)((r + 250) * 0.5f);
                mesh->colors[v*4 + 1] = (unsigned char)((g + 100) * 0.5f);
                mesh->colors[v*4 + 2] = (unsigned char)((b + 100) * 0.5f);
            } else if (state >= 2) { // Destroyed / Destroyed+Infected
                // state 2 = destroyed regular, state 4 = destroyed infected (hypothetical, used internally below)
                float dark_r = 60.0f, dark_g = 40.0f, dark_b = 25.0f;
                if (state == 4) { // Infected version of destroyed
                    dark_r = 120.0f; dark_g = 60.0f; dark_b = 100.0f;
                }

                mesh->colors[v*4 + 0] = (unsigned char)((float)r / 255.0f * dark_r);
                mesh->colors[v*4 + 1] = (unsigned char)((float)g / 255.0f * dark_g); 
                mesh->colors[v*4 + 2] = (unsigned char)((float)b / 255.0f * dark_b);
                
                // Scale mesh down/squash
                float sx = 1.0f, sy = 1.0f, sz = 1.0f;
                int shape = (state == 4) ? 2 : state; // Infected destroyed uses implode (2) or squash (3)
                if (shape == 2) { sx = 0.5f; sy = 0.75f; sz = 0.5f; } // Implode
                if (shape == 3) { sx = 1.0f; sy = 0.5f;  sz = 1.0f; } // Squash

                mesh->vertices[v*3 + 0] *= sx;
                mesh->vertices[v*3 + 1] *= sy;
                mesh->vertices[v*3 + 2] *= sz;
            }
        }
        
        // Final upload
        UpdateMeshBuffer(*mesh, 0, mesh->vertices, mesh->vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(*mesh, 3, mesh->colors, mesh->vertexCount * 4 * sizeof(unsigned char), 0);
    }
    
    return m;
}
