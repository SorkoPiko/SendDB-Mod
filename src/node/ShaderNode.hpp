#ifndef SENDDB_SHADERNODE_HPP
#define SENDDB_SHADERNODE_HPP

#include <utils/Shader.hpp>

// stripped down version of https://github.com/cgytrus/MenuShaders/blob/12a8244/src/main.cpp
class ShaderNode : public CCNode {
    Shader shader;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLint uniformResolution = 0;
    GLint uniformTime = 0;
    GLint uniformDeltaTime = 0;
    GLint uniformFrameRate = 0;
    GLint uniformFrame = 0;
    float deltaTime = 0.f;
    float time = 0.f;
    GLint frame = 0;
    CCArrayExt<CCSprite*> shaderSprites;

    bool init(const std::string& vert, const std::string& frag);
    void update(float dt) override;
    void draw() override;

public:
    static long long firstTime;

    static ShaderNode* create(const std::string& vert, const std::string& frag, const std::vector<CCSprite*>& sprites = {});
    static Result<ShaderNode*> createFromPath(const std::string& vertPath, const std::string& fragPath, const std::vector<CCSprite*>& sprites = {});
};

#endif