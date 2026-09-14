#pragma once

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include <GL/glew.h>

struct Character
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float xoffset = 0.0f;
    float yoffset = 0.0f;
    float xadvance = 0.0f;
};

class BitmapFont
{
public:
    std::map<int, Character> glyphs;
    GLuint textureID = 0;
    float atlasWidth = 512.0f;
    float atlasHeight = 512.0f;
    float lineHeight = 32.0f;

    bool LoadMetadata(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            return false;
        }

        glyphs.clear();

        std::string line;
        while (std::getline(file, line))
        {
            if (line.rfind("common", 0) == 0)
            {
                ParseCommonLine(line);
            }
            else if (line.rfind("char ", 0) == 0)
            {
                ParseCharLine(line);
            }
        }

        return !glyphs.empty();
    }

    void SetTexture(GLuint texture)
    {
        textureID = texture;
    }

    const Character* GetCharacter(int codepoint) const
    {
        std::map<int, Character>::const_iterator it = glyphs.find(codepoint);
        if (it == glyphs.end())
        {
            return nullptr;
        }
        return &it->second;
    }

private:
    static bool ExtractValue(const std::string& token, const char* key, float& value)
    {
        const std::string prefix = std::string(key) + "=";
        if (token.rfind(prefix, 0) != 0)
        {
            return false;
        }
        value = std::stof(token.substr(prefix.size()));
        return true;
    }

    static bool ExtractIntValue(const std::string& token, const char* key, int& value)
    {
        const std::string prefix = std::string(key) + "=";
        if (token.rfind(prefix, 0) != 0)
        {
            return false;
        }
        value = std::stoi(token.substr(prefix.size()));
        return true;
    }

    void ParseCommonLine(const std::string& line)
    {
        std::istringstream ss(line);
        std::string token;
        while (ss >> token)
        {
            float value = 0.0f;
            if (ExtractValue(token, "lineHeight", value))
            {
                lineHeight = value;
            }
            else if (ExtractValue(token, "scaleW", value))
            {
                atlasWidth = value;
            }
            else if (ExtractValue(token, "scaleH", value))
            {
                atlasHeight = value;
            }
        }
    }

    void ParseCharLine(const std::string& line)
    {
        std::istringstream ss(line);
        std::string token;
        Character ch;
        int id = -1;

        while (ss >> token)
        {
            ExtractIntValue(token, "id", id);
            ExtractValue(token, "x", ch.x);
            ExtractValue(token, "y", ch.y);
            ExtractValue(token, "width", ch.width);
            ExtractValue(token, "height", ch.height);
            ExtractValue(token, "xoffset", ch.xoffset);
            ExtractValue(token, "yoffset", ch.yoffset);
            ExtractValue(token, "xadvance", ch.xadvance);
        }

        if (id >= 0)
        {
            glyphs[id] = ch;
        }
    }
};
