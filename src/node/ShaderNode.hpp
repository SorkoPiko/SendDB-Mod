#ifndef SENDDB_SHADERNODE_HPP
#define SENDDB_SHADERNODE_HPP

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
    GLuint pingFBO = 0;
    GLuint pongFBO = 0;
    GLuint pingTexture = 0;
    GLuint pongTexture = 0;
    float deltaTime = 0.f;
    float time = 0.f;
    GLint frame = 0;
    CCArrayExt<CCSprite*> shaderSprites;

    GLuint pbos[2] = {0, 0};
    int pboIndex = 0;

    bool onlyScissorFinalPass = false;
    int numPasses = 1;
    bool passCurrentFrame = false;

    CCSize lastSize = {};

    bool init(const std::string& vertPath, const std::string& fragPath);
    void update(float dt) override;
    void draw() override;

    void updateTextures(const CCSize& frSize);

public:
    static long long firstTime;

    static ShaderNode* create(const std::string& vertPath, const std::string& fragPath, const std::vector<CCSprite*>& sprites = {});

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
};

#endif