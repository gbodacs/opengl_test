#pragma once

#include "../platform/gl_platform.h"

#if defined(__ANDROID__)
    #include <EGL/egl.h>
    #include <android/native_window.h>
#endif

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

// Builds a column-major orthographic projection matrix (glOrtho equivalent),
// usable with glLoadMatrixf on both desktop GL and GLES 1.1 (which has no glOrtho).
inline void BuildOrthoMatrix(float left, float right, float bottom, float top, float nearZ, float farZ, GLfloat* m)
{
    for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (farZ - nearZ);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(farZ + nearZ) / (farZ - nearZ);
    m[15] = 1.0f;
}

// Builds a column-major perspective (frustum) projection matrix, usable with
// glLoadMatrixf on both desktop GL and GLES 1.1 (which has no glFrustum).
inline void BuildFrustumMatrix(float left, float right, float bottom, float top, float nearZ, float farZ, GLfloat* m)
{
    for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    m[0] = (2.0f * nearZ) / (right - left);
    m[5] = (2.0f * nearZ) / (top - bottom);
    m[8] = (right + left) / (right - left);
    m[9] = (top + bottom) / (top - bottom);
    m[10] = -(farZ + nearZ) / (farZ - nearZ);
    m[11] = -1.0f;
    m[14] = -(2.0f * farZ * nearZ) / (farZ - nearZ);
}

class Engine 
{
public:
#if defined(__ANDROID__)
    ANativeWindow* nativeWindow = nullptr;
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglContext = EGL_NO_CONTEXT;
    bool closeRequested = false;
#else
    GLFWwindow* window;
#endif
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

#if defined(__ANDROID__)
    // Creates the EGL surface/context for a live ANativeWindow. Safe to call
    // again after DestroySurface() when Android recreates the window
    // (e.g. after being backgrounded).
    bool InitAndroid(ANativeWindow* win)
    {
        nativeWindow = win;

        if (eglDisplay == EGL_NO_DISPLAY)
        {
            eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (eglDisplay == EGL_NO_DISPLAY || !eglInitialize(eglDisplay, nullptr, nullptr))
                return false;
        }

        const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };

        EGLConfig config;
        EGLint numConfigs = 0;
        if (!eglChooseConfig(eglDisplay, attribs, &config, 1, &numConfigs) || numConfigs == 0)
            return false;

        EGLint format = 0;
        eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
        ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);

        eglSurface = eglCreateWindowSurface(eglDisplay, config, nativeWindow, nullptr);
        if (eglSurface == EGL_NO_SURFACE)
            return false;

        if (eglContext == EGL_NO_CONTEXT)
        {
            eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, nullptr);
            if (eglContext == EGL_NO_CONTEXT)
                return false;
        }

        if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) == EGL_FALSE)
            return false;

        EGLint w = 0, h = 0;
        eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &w);
        eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &h);
        viewportWidth = w;
        viewportHeight = h;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glViewport(0, 0, viewportWidth, viewportHeight);
        Begin2DPass();
        return true;
    }

    // Tears down just the window surface (called on APP_CMD_TERM_WINDOW); the
    // EGL display/context are kept so InitAndroid() can cheaply resume.
    void DestroySurface()
    {
        if (eglDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (eglSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(eglDisplay, eglSurface);
                eglSurface = EGL_NO_SURFACE;
            }
        }
        nativeWindow = nullptr;
    }

    bool ShouldClose() const { return closeRequested; }
    void RequestClose() { closeRequested = true; }
#else
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

    bool ShouldClose() const { return glfwWindowShouldClose(window); }
    void RequestClose() { glfwSetWindowShouldClose(window, true); }
#endif

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
        glColor4f(p->red, p->green, p->blue, 1.0f);

        // 4. Maga a rajzolás
        if (p->mirrored) 
        {
            glScalef(-1.0f, 1.0f, 1.0f); // Tükrözés X tengely mentén
            glTranslatef(-p->width, 0, 0); // Visszaállítja a pozíciót a tükrözés után
        } else {
            glScalef(1.0f, 1.0f, 1.0f); // Normál méretezés
        }

        // GLES has no glBegin/glEnd immediate mode, so the quad is drawn as a
        // triangle fan from vertex arrays - unchanged on desktop's compat profile.
        const GLfloat vertices[8] = {
            0.0f, 0.0f,
            p->width, 0.0f,
            p->width, p->height,
            0.0f, p->height
        };
        const GLfloat texCoords[8] = {
            p->u0, p->v0,
            p->u1, p->v0,
            p->u1, p->v1,
            p->u0, p->v1
        };

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertices);

        if (p->textureID != 0)
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
        }

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

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

        glColor4f(mesh->red, mesh->green, mesh->blue, 1.0f);

        // GLES 1.1 only guarantees GL_UNSIGNED_SHORT indices, so cache a 16-bit
        // copy the first time this mesh is drawn (meshes here are small/static).
        if (mesh->indices16.size() != mesh->indices.size())
        {
            mesh->indices16.resize(mesh->indices.size());
            for (size_t i = 0; i < mesh->indices.size(); ++i)
            {
                mesh->indices16[i] = static_cast<unsigned short>(mesh->indices[i]);
            }
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        const MeshVertex* base = mesh->vertices.data();
        glVertexPointer(3, GL_FLOAT, sizeof(MeshVertex), &base->position);
        glTexCoordPointer(2, GL_FLOAT, sizeof(MeshVertex), &base->uv);
        glNormalPointer(GL_FLOAT, sizeof(MeshVertex), &base->normal);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices16.size()), GL_UNSIGNED_SHORT, mesh->indices16.data());

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

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
#if defined(__ANDROID__)
        if (eglDisplay != EGL_NO_DISPLAY && eglSurface != EGL_NO_SURFACE)
        {
            eglSwapBuffers(eglDisplay, eglSurface);
        }
#else
        glfwSwapBuffers(window);
        glfwPollEvents();
#endif
        
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
#if defined(__ANDROID__)
        DestroySurface();
        if (eglDisplay != EGL_NO_DISPLAY)
        {
            if (eglContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(eglDisplay, eglContext);
                eglContext = EGL_NO_CONTEXT;
            }
            eglTerminate(eglDisplay);
            eglDisplay = EGL_NO_DISPLAY;
        }
#else
        glfwDestroyWindow(window);
        glfwTerminate();
#endif
    }

private:
    void Begin2DPass()
    {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        GLfloat ortho[16];
        BuildOrthoMatrix(0.0f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), 0.0f, -10000.0f, 10000.0f, ortho);
        glLoadMatrixf(ortho);
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
        GLfloat frustum[16];
        BuildFrustumMatrix(-right, right, -top, top, nearPlane, farPlane, frustum);
        glLoadMatrixf(frustum);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};