#include "plane.h"
#include <vector>

struct AnimFrame
{
public:
    float time;
    GLuint texture;
};

//-----------------------------------------------
class GameObject: public Plane
{
    public:
    GameObject(float x, float y, float width, float height, GLuint textureID=0) 
        : Plane(x, y, width, height, textureID) 
    {
        // Constructor implementation (if needed)
    }

    virtual void Update(float deltaTime) override
    {
        // Default implementation does nothing
    }

    virtual ~GameObject() {
        // Destructor implementation (if needed)
    }

};

//-----------------------------------------------
class AnimatedGameObject : public GameObject
{
protected:
    std::vector<AnimFrame> animationFrames;
    float currentTime = 0.0f;
    bool  isAnimating = true;

public:
    AnimatedGameObject(float x, float y, float width, float height) 
        : GameObject(x, y, width, height, 0) 
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

    void AddFrame(float time, GLuint texture)
    {
        AnimFrame frame;
        frame.time = time;
        frame.texture = texture;
        animationFrames.push_back(frame);
    }

    int GetNumberofFrames()
    {
        return animationFrames.size();
    }

    int GetFrameTexture(int index)
    {
        if (index < 0 || index >= animationFrames.size())
            return -1; // Invalid index
        return animationFrames[index].texture;
    }

    float GetFrameTime(int index)
    {
        if (index < 0 || index >= animationFrames.size())
            return -1.0f; // Invalid index
        return animationFrames[index].time;
    }

    virtual void Update(float deltaTime)
    {
        if (animationFrames.empty())
            return;

        if (!isAnimating)
            return;

        currentTime += deltaTime;

        // Loop through the animation frames
        for (size_t i = 0; i < animationFrames.size(); ++i)
        {
            if (currentTime < animationFrames[i].time)
            {
                // Set the texture of the GameObject to the current frame's texture
                // Assuming you have a method to set the texture, e.g., SetTexture()
                SetTexture(animationFrames[i].texture);
                break;
            }
            else if (i == animationFrames.size() - 1)
            {
                // If we've reached the last frame, loop back to the first frame
                currentTime = 0.0f;
                SetTexture(animationFrames[0].texture);
            }
        }
    }
};

//-----------------------------------------------
class Player : public GameObject
{
public:
    

};


//-----------------------------------------------
class Enemy : public GameObject
{
public:
    void Move(float deltaX, float deltaY)
    {
        x += deltaX;
        y += deltaY;
    }

};