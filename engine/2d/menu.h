#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <vector>

#include "engine.h"
#include "font.h"
#include "input.h"

// A single entry in a Menu. Leaf items run onSelect; items with a non-empty
// submenu open that submenu instead. iconTexture is left at 0 until graphics
// are provided - Engine::DrawPlane already renders a plain colored quad in that case.
struct MenuItem
{
    std::string             label;
    std::vector<MenuItem>   submenu;
    std::function<void()>   onSelect;
    GLuint                  iconTexture = 0;
    bool                    enabled = true;
    bool                    closesMenu = false; // e.g. "Resume"/"Exit" leaves

    bool HasSubmenu() const { return !submenu.empty(); }
};

// Keyboard-driven menu with nested submenus and a flashing selection cursor.
class Menu
{
public:
    explicit Menu(std::vector<MenuItem> rootItems)
    {
        root = std::move(rootItems);
        levels.push_back(&root);
        selection.push_back(FirstEnabledIndex(root, 0, 1));
    }

    void Update(float deltaTime)
    {
        flashTimer += deltaTime;
        if (flashTimer >= flashInterval)
        {
            flashTimer -= flashInterval;
            flashOn = !flashOn;
        }
    }

    // Consumes an already edge-detected input snapshot (see Input::Update) and
    // updates selection/navigation. Returns true when the menu should be closed
    // (top-level Back, or a leaf item marked closesMenu was activated).
    bool HandleInput(const InputSnapshot& input)
    {
        bool close = false;

        if (input.w.pressed)
        {
            MoveSelection(-1);
        }
        if (input.s.pressed)
        {
            MoveSelection(1);
        }
        if (input.space.pressed)
        {
            close = ActivateSelected();
        }
        if (input.esc.pressed)
        {
            close = GoBack();
        }

        return close;
    }

    void Render(Engine& engine, const BitmapFont& font, float x, float y, float lineSpacing = 28.0f, float scale = 1.0f)
    {
        std::vector<MenuItem>& current = *levels.back();
        int selectedIndex = selection.back();

        float cursorY = y;
        for (int i = 0; i < static_cast<int>(current.size()); ++i)
        {
            const MenuItem& item = current[i];
            float r = 1.0f, g = 1.0f, b = 1.0f;
            std::string prefix = "  ";

            if (!item.enabled)
            {
                r = g = b = 0.4f;
            }
            else if (i == selectedIndex)
            {
                prefix = "> ";
                if (flashOn)
                {
                    r = 1.0f; g = 1.0f; b = 0.2f; // bright highlight
                }
                else
                {
                    r = 0.6f; g = 0.6f; b = 0.1f; // dimmed highlight
                }
            }

            engine.PrintText(font, x, cursorY, prefix + item.label, scale, r, g, b);
            cursorY += lineSpacing * scale;
        }
    }

    bool IsAtRoot() const { return levels.size() == 1; }

private:
    std::vector<MenuItem>                  root;
    std::vector<std::vector<MenuItem>*>    levels;
    std::vector<int>                       selection;

    float flashTimer = 0.0f;
    float flashInterval = 0.4f;
    bool  flashOn = true;

    static int FirstEnabledIndex(const std::vector<MenuItem>& items, int start, int dir)
    {
        int count = static_cast<int>(items.size());
        if (count == 0)
        {
            return 0;
        }
        int i = start;
        for (int tries = 0; tries < count; ++tries)
        {
            if (items[i].enabled)
            {
                return i;
            }
            i = (i + dir + count) % count;
        }
        return start;
    }

    void MoveSelection(int dir)
    {
        std::vector<MenuItem>& current = *levels.back();
        int count = static_cast<int>(current.size());
        if (count == 0)
        {
            return;
        }

        int& index = selection.back();
        int next = index;
        for (int tries = 0; tries < count; ++tries)
        {
            next = (next + dir + count) % count;
            if (current[next].enabled)
            {
                break;
            }
        }
        index = next;
        flashOn = true;
        flashTimer = 0.0f;
    }

    // Returns true if this activation should close the whole menu.
    bool ActivateSelected()
    {
        std::vector<MenuItem>& current = *levels.back();
        int index = selection.back();
        if (index < 0 || index >= static_cast<int>(current.size()))
        {
            return false;
        }

        MenuItem& item = current[index];
        if (!item.enabled)
        {
            return false;
        }

        if (item.HasSubmenu())
        {
            levels.push_back(&item.submenu);
            selection.push_back(FirstEnabledIndex(item.submenu, 0, 1));
            flashOn = true;
            flashTimer = 0.0f;
            return false;
        }

        if (item.onSelect)
        {
            item.onSelect();
        }
        return item.closesMenu;
    }

    // Returns true if Back was pressed at the root level (request to close menu).
    bool GoBack()
    {
        if (levels.size() > 1)
        {
            levels.pop_back();
            selection.pop_back();
            flashOn = true;
            flashTimer = 0.0f;
            return false;
        }
        return true;
    }
};
