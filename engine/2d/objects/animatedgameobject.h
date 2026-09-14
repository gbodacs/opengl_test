#pragma once

#include "gameobject.h"
#include <vector>

//-----------------------------------------------
class AnimatedGameObject : public GameObject
{
protected:
    std::vector<std::vector<AnimFrame>> animations;
    int   currentAnimation = 0;
    float currentTime = 0.0f;
    bool  isAnimating = true;

public:
    AnimatedGameObject(float x, float y, float width, float height, GLuint textureID=0)
        : GameObject(x, y, width, height, textureID)
    {
        // Constructor implementation (if needed)
    }

    virtual ~AnimatedGameObject() {
        // Destructor implementation (if needed)
    }

    void SetAnimating(bool animating)
    {
        isAnimating = animating;
    }

    bool GetAnimating() const
    {
        return isAnimating;
    }

    // Creates a new empty animation and returns its index
    int AddAnimation()
    {
        animations.emplace_back();
        return static_cast<int>(animations.size()) - 1;
    }

    void AddFrame(int animationIndex, float time, GLuint texture)
    {
        if (animationIndex < 0 || animationIndex >= (int)animations.size())
            return; // Invalid animation index

        AnimFrame frame;
        frame.time = time;
        frame.texture = texture;
        animations[animationIndex].push_back(frame);
    }

    int GetNumberOfAnimations()
    {
        return animations.size();
    }

    int GetNumberofFrames(int animationIndex)
    {
        if (animationIndex < 0 || animationIndex >= (int)animations.size())
            return 0; // Invalid animation index
        return animations[animationIndex].size();
    }

    int GetFrameTexture(int animationIndex, int frameIndex)
    {
        if (animationIndex < 0 || animationIndex >= (int)animations.size())
            return -1; // Invalid animation index
        auto& frames = animations[animationIndex];
        if (frameIndex < 0 || frameIndex >= (int)frames.size())
            return -1; // Invalid frame index
        return frames[frameIndex].texture;
    }

    float GetFrameTime(int animationIndex, int frameIndex)
    {
        if (animationIndex < 0 || animationIndex >= (int)animations.size())
            return -1.0f; // Invalid animation index
        auto& frames = animations[animationIndex];
        if (frameIndex < 0 || frameIndex >= (int)frames.size())
            return -1.0f; // Invalid frame index
        return frames[frameIndex].time;
    }

    int GetCurrentAnimation() const
    {
        return currentAnimation;
    }

    // Switches the active animation, optionally resetting playback time
    void SetCurrentAnimation(int animationIndex, bool resetTime = true)
    {
        if (animationIndex < 0 || animationIndex >= (int)animations.size())
            return; // Invalid animation index

        if (animationIndex != currentAnimation && resetTime)
            currentTime = 0.0f;

        currentAnimation = animationIndex;
    }

    virtual void Update(float deltaTime) override
    {
        if (currentAnimation < 0 || currentAnimation >= (int)animations.size())
            return;

        auto& frames = animations[currentAnimation];
        if (frames.empty())
            return;

        if (!isAnimating)
            return;

        currentTime += deltaTime;

        // Loop through the animation frames
        for (size_t i = 0; i < frames.size(); ++i)
        {
            if (currentTime < frames[i].time)
            {
                GetPlane()->SetTexture(frames[i].texture);
                break;
            }
            else if (i == frames.size() - 1)
            {
                // If we've reached the last frame, loop back to the first frame
                currentTime = 0.0f;
                GetPlane()->SetTexture(frames[0].texture);
            }
        }
    }
};