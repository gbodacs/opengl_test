#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <set>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "plane.h"
#include "gameobject.h"

class Engine 
{
public:
    GLFWwindow* window;
    const double TARGET_FPS = 60.0;
    const double FRAME_TIME = 1.0 / TARGET_FPS;
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> deltaTime = std::chrono::duration<double>::zero();
    std::set<Plane*> planes;

    Plane* CreatePlane(float x, float y, float width, float height, GLuint textureID=0) 
    {
        Plane* newPlane = new Plane{x, y, width, height, textureID};
        planes.insert(newPlane);
        return newPlane;
    }

    AnimatedGameObject* CreateAnimatedGameObject(float x, float y, float width, float height) 
    {
        AnimatedGameObject* newObj = new AnimatedGameObject{x, y, width, height};
        planes.insert(newObj);
        return newObj;
    }

    bool RemovePlane(Plane* plane) 
    {
        if (plane == nullptr)
            return false;
        planes.erase(plane);
        delete plane;
        return true;
    }

    ~Engine() 
    {
        for (Plane* p : planes) 
        {
            delete p;
        }
    }

    GLuint LoadTexture(const char* filename) 
    {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set wrapping/filtering options
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Setup Ortho Projection
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Left, Right, Bottom, Top, Near, Far
        glOrtho(0, width, height, 0, -10000, 10000); 
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        return true;
    }

    float UpdateStart() 
    {
        // 1. Clear Screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity(); // Alaphelyzetbe állítja a Modelview mátrixot
        
        auto now = std::chrono::high_resolution_clock::now();
        deltaTime = now - start;
        start = now;

        double deltaTimeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(deltaTime).count();

        return deltaTimeMs/1000.0f; // Return deltaTime in seconds
    }

    void UpdateAllPlanes(float deltaTime) 
    {
        for (Plane* p : planes) 
        {
            p->Update(deltaTime);
        }
    }

    void RenderAllPlanes() 
    {
        for (Plane* p : planes) 
        {
            DrawPlane(p);
        }
    }

    void DrawPlane(Plane* p) 
    {
        glPushMatrix(); // Elmenti az aktuális mátrix állapotot
        
        // 1. Pozicionálás
        glTranslatef(p->x, p->y, 0); // Így a glBegin-ben (0,0)-ról indulhatsz

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
            glTexCoord2f(0, 0); glVertex2f(0, 0);
            glTexCoord2f(1, 0); glVertex2f(p->width, 0);
            glTexCoord2f(1, 1); glVertex2f(p->width, p->height);
            glTexCoord2f(0, 1); glVertex2f(0, p->height);
        glEnd();

        glPopMatrix(); // Visszaállítja a mátrixot a rajzolás előtti állapotra
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
};