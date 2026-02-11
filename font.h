#pragma once 
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Character {
    float x, y;             // Position in the texture (pixels)
    float width, height;    // Size of the character (pixels)
    float xoffset, yoffset; // How much to shift when rendering
    float xadvance;         // How far to move the cursor for the next char
};

std::map<int, Character> fontMap;

float texW = 512.0f; // Total width of your texture atlas
float texH = 512.0f; // Total height of your texture atlas

void LoadFontMetadata(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 4) == "char") {
            Character c;
            int id;
            // Simplified parsing - in a real engine, use a more robust string splitter
            sscanf(line.c_str(), "char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f",
                   &id, &c.x, &c.y, &c.width, &c.height, &c.xoffset, &c.yoffset, &c.xadvance);
            fontMap[id] = c;
        }
    }
}

void DrawStringProportional(float x, float y, std::string text, GLuint fontTexture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glBegin(GL_QUADS);

    float cursorX = x;

    for (char charCode : text) {
        if (fontMap.find(charCode) == fontMap.end()) continue;

        Character& c = fontMap[charCode];

        // Convert pixel coordinates to 0.0 - 1.0 UV coordinates
        float u = c.x / texW;
        float v = c.y / texH;
        float uw = c.width / texW;
        float vh = c.height / texH;

        // Calculate screen positions using offsets
        float posX = cursorX + c.xoffset;
        float posY = y + c.yoffset;

        glTexCoord2f(u, v);           glVertex2f(posX, posY);
        glTexCoord2f(u + uw, v);      glVertex2f(posX + c.width, posY);
        glTexCoord2f(u + uw, v + vh); glVertex2f(posX + c.width, posY + c.height);
        glTexCoord2f(u, v + vh);      glVertex2f(posX, posY + c.height);

        // Move cursor by xadvance for the next letter
        cursorX += c.xadvance;
    }

    glEnd();
}
