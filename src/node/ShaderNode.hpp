#pragma once

#include <utils/Shader.hpp>

// stripped down version of https://github.com/cgytrus/MenuShaders/blob/12a8244/src/main.cpp
class ShaderNode : public CCNode {
    Shader shader;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint uniformResolution = 0;
    GLint uniformScreenRect = 0;
    GLint uniformTime = 0;
    GLint uniformDeltaTime = 0;
    GLint uniformFrameRate = 0;
    GLint uniformFrame = 0;
    GLint uniformCurrentPass = 0;
    GLint uniformPasses = 0;
    GLint uniformSprites = 0;
    GLuint pingFBO = 0;
    GLuint pongFBO = 0;
    GLuint pingTexture = 0;
    GLuint pongTexture = 0;
    float deltaTime = 0.f;
    float time = 0.f;
    GLint frame = 0;
    std::vector<Ref<CCSprite>> shaderSprites;

    bool onlyScissorFinalPass = false;
    int numPasses = 1;
    bool passCurrentFrame = false;

    CCSize lastSize = {};
    int allocatedSprites = 0;

    bool init(const std::string& vertPath, const std::string& fragPath);
    void update(float dt) override;
    void draw() override;

    void updateTextures(const CCSize& frSize);
    void updateSprites(int newSpriteCount);

public:
    static long long firstTime;

    static ShaderNode* create(const std::string& vertPath, const std::string& fragPath);

    void setOnlyScissorFinalPass(const bool flag) {
        onlyScissorFinalPass = flag;
    }

    // if true, only applies scissor test on final pass, meaning intermediate passes render full screen.
    // useful for controlling blur effects (soft edges otherwise)
    void setPasses(const int value) {
        numPasses = value;
    }

    void setPassCurrentFrame(const bool flag) {
        passCurrentFrame = flag;
    }

    void setSprites(const std::vector<Ref<CCSprite>>& sprites) {
        shaderSprites = sprites;
        updateSprites(shaderSprites.size());
    }

    void allocateSprites(const int count) {
        updateSprites(count);
    }
};