#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>
#include <set>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "plane.h"
#include "objects/animatedgameobject.h"
#include "font.h"
#include "../3d/mesh.h"
#include "../3d/objloader.h"

class Engine 
{
public:
    GLFWwindow* window;
    const double TARGET_FPS = 60.0;
    const double FRAME_TIME = 1.0 / TARGET_FPS;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> deltaTime = std::chrono::duration<double>::zero();
    std::set<Plane*> planes;
    std::set<Mesh3D*> meshes;
    int viewportWidth = 0;
    int viewportHeight = 0;
    float cameraDistance = 5.0f;
    float cameraFovDegrees = 60.0f;

    Plane* CreatePlane(float x, float y, float width, float height, GLuint textureID=0) 
    {
        Plane* newPlane = new Plane{x, y, width, height, textureID};
        planes.insert(newPlane);
        return newPlane;
    }

    bool AddPlane(Plane* plane) 
    {
        if (plane == nullptr)
            return false;
        planes.insert(plane);
        return true;
    }

    bool AddMesh(Mesh3D* mesh)
    {
        if (mesh == nullptr)
            return false;
        meshes.insert(mesh);
        return true;
    }

    bool RemovePlane(Plane* plane) 
    {
        if (plane == nullptr)
            return false;
        planes.erase(plane);
        delete plane;
        return true;
    }

    bool RemoveMesh(Mesh3D* mesh)
    {
        if (mesh == nullptr)
            return false;
        meshes.erase(mesh);
        delete mesh;
        return true;
    }

    Mesh3D* LoadMesh(const char* filename)
    {
        Mesh3D* mesh = new Mesh3D();
        std::string diffuseTexturePath;
        if (!LoadOBJMesh(filename, *mesh, &diffuseTexturePath))
        {
            delete mesh;
            return nullptr;
        }

        if (!diffuseTexturePath.empty())
        {
            const GLuint textureID = LoadTexture(diffuseTexturePath.c_str());
            if (textureID == 0)
            {
                delete mesh;
                return nullptr;
            }
            mesh->SetTexture(textureID);
        }

        meshes.insert(mesh);
        return mesh;
    }

    ~Engine() 
    {
        for (Plane* p : planes) 
        {
            delete p;
        }

        for (Mesh3D* mesh : meshes)
        {
            delete mesh;
        }
    }

    GLuint LoadTexture(const char* filename) 
    {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
        if (data) 
        {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            glDeleteTextures(1, &textureID);
            return 0;
        }

        return textureID;
    }

    bool Init(int width, int height, const char* title) 
    {
        if (!glfwInit()) 
        {
            return false;
        }

        window = glfwCreateWindow(width, height, title, NULL, NULL);

        if (!window) 
        {
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window);
        glewInit();
        viewportWidth = width;
        viewportHeight = height;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glViewport(0, 0, width, height);
        Begin2DPass();
        
        return true;
    }

    float UpdateStart() 
    {
        // 1. Clear Screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Begin2DPass();
        
        auto now = std::chrono::high_resolution_clock::now();
        deltaTime = now - start;
        start = now;

        double deltaTimeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(deltaTime).count();

        return deltaTimeMs/1000.0f; // Return deltaTime in seconds
    }

    void RenderAllPlanes() 
    {
        for (Plane* p : planes) 
        {
            DrawPlane(p);
        }
    }

    void RenderAllMeshes()
    {
        if (meshes.empty())
        {
            return;
        }

        Begin3DPass();
        glTranslatef(0.0f, 0.0f, -cameraDistance);

        for (Mesh3D* mesh : meshes)
        {
            DrawMesh(mesh);
        }

        Begin2DPass();
    }

    void DrawPlane(Plane* p) 
    {
        glPushMatrix(); // Elmenti az aktuális mátrix állapotot
        
        // 1. Pozicionálás
        glTranslatef(p->x, p->y, 0); // glBegin-ben (0,0)-ról indulhatsz

        // 2. Textúra kezelés
        if (p->textureID != 0) 
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, p->textureID);
        } else {
            glDisable(GL_TEXTURE_2D);
        }

        // 3. Szín alaphelyzetbe (hogy a textúra ne legyen sötét)
        glColor3f(p->red, p->green, p->blue);

        // 4. Maga a rajzolás
        if (p->mirrored) 
        {
            glScalef(-1.0f, 1.0f, 1.0f); // Tükrözés X tengely mentén
            glTranslatef(-p->width, 0, 0); // Visszaállítja a pozíciót a tükrözés után
        } else {
            glScalef(1.0f, 1.0f, 1.0f); // Normál méretezés
        }

        glBegin(GL_QUADS);
            glTexCoord2f(p->u0, p->v0); glVertex2f(0, 0);
            glTexCoord2f(p->u1, p->v0); glVertex2f(p->width, 0);
            glTexCoord2f(p->u1, p->v1); glVertex2f(p->width, p->height);
            glTexCoord2f(p->u0, p->v1); glVertex2f(0, p->height);
        glEnd();

        glPopMatrix(); // Visszaállítja a mátrixot a rajzolás előtti állapotra
    }

    void DrawMesh(Mesh3D* mesh)
    {
        if (mesh == nullptr || !mesh->IsValid())
        {
            return;
        }

        glPushMatrix();
        glTranslatef(mesh->x, mesh->y, mesh->z);
        glRotatef(mesh->rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(mesh->rotationY, 0.0f, 1.0f, 0.0f);
        glRotatef(mesh->rotationZ, 0.0f, 0.0f, 1.0f);
        glScalef(mesh->scaleX, mesh->scaleY, mesh->scaleZ);

        if (mesh->textureID != 0)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, mesh->textureID);
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
        }

        glColor3f(mesh->red, mesh->green, mesh->blue);

        glBegin(GL_TRIANGLES);
        for (unsigned int index : mesh->indices)
        {
            const MeshVertex& vertex = mesh->vertices[index];
            glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
            glTexCoord2f(vertex.uv.x, vertex.uv.y);
            glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
        }
        glEnd();

        glPopMatrix();
    }

    void PrintText(const BitmapFont& font, float x, float y, const std::string& text,
                   float scale = 1.0f, float red = 1.0f, float green = 1.0f, float blue = 1.0f)
    {
        if (font.textureID == 0 || font.atlasWidth <= 0.0f || font.atlasHeight <= 0.0f)
        {
            return;
        }

        float cursorX = x;
        float cursorY = y;

        for (char c : text)
        {
            if (c == '\n')
            {
                cursorX = x;
                cursorY += font.lineHeight * scale;
                continue;
            }

            const Character* glyph = font.GetCharacter(static_cast<unsigned char>(c));
            if (glyph == nullptr)
            {
                glyph = font.GetCharacter('?');
                if (glyph == nullptr)
                {
                    continue;
                }
            }

            Plane glyphPlane(
                cursorX + glyph->xoffset * scale,
                cursorY + glyph->yoffset * scale,
                glyph->width * scale,
                glyph->height * scale,
                font.textureID
            );
            glyphPlane.SetColor(red, green, blue);

            const float u0 = glyph->x / font.atlasWidth;
            const float v0 = glyph->y / font.atlasHeight;
            const float u1 = (glyph->x + glyph->width) / font.atlasWidth;
            const float v1 = (glyph->y + glyph->height) / font.atlasHeight;
            glyphPlane.SetUV(u0, v0, u1, v1);

            DrawPlane(&glyphPlane);
            cursorX += glyph->xadvance * scale;
        }
    }

    void UpdateEnd() 
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // 3. FPS Limiter
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        if (elapsed.count() < FRAME_TIME) 
        {
            auto sleepTime = std::chrono::duration<double>(FRAME_TIME - elapsed.count());
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(sleepTime));
        }
    }

    void Done() 
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

private:
    void Begin2DPass()
    {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, viewportWidth, viewportHeight, 0, -10000, 10000);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void Begin3DPass()
    {
        glEnable(GL_DEPTH_TEST);

        const float aspectRatio = (viewportHeight > 0)
            ? static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight)
            : 1.0f;
        const float nearPlane = 0.1f;
        const float farPlane = 100.0f;
        const float pi = 3.14159265358979323846f;
        const float tangent = std::tan((cameraFovDegrees * 0.5f) * pi / 180.0f);
        const float top = nearPlane * tangent;
        const float right = top * aspectRatio;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-right, right, -top, top, nearPlane, farPlane);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};