#include "ShaderNode.hpp"

#include <utils/TimeUtils.hpp>

long long ShaderNode::firstTime = 0;

bool ShaderNode::init(const std::string& vert, const std::string& frag) {
    auto res = shader.compile(vert, frag);
    if (!res) {
        log::error("{}", res.unwrapErr());
        return false;
    }

    glBindAttribLocation(shader.program, 0, "aPosition");

    res = shader.link();
    if (!res) {
        log::error("{}", res.unwrapErr());
        return false;
    }

    ccGLUseProgram(shader.program);

    shaderSprites.inner()->retain();
    std::istringstream stream(frag);
    std::string line;

    constexpr GLfloat vertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f, -1.0f,
        1.0f,  1.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), static_cast<void*>(nullptr));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    uniformResolution = glGetUniformLocation(shader.program, "resolution");
    uniformTime = glGetUniformLocation(shader.program, "time");
    uniformDeltaTime = glGetUniformLocation(shader.program, "deltaTime");
    uniformFrameRate = glGetUniformLocation(shader.program, "frameRate");
    uniformFrame = glGetUniformLocation(shader.program, "frame");

    for (size_t i = 0; i < shaderSprites.size(); ++i) {
        const auto uniform = glGetUniformLocation(shader.program, ("sprite" + std::to_string(i)).c_str());
        glUniform1i(uniform, static_cast<GLint>(i));
    }

    scheduleUpdate();
    return true;
}

void ShaderNode::update(const float dt) {
    if (firstTime == 0) firstTime = TimeUtils::getCurrentTimestamp();

    if (time == 0.0f || frame == 0) {
        const long long currentTime = TimeUtils::getCurrentTimestamp();
        if (time == 0.0f) time = (currentTime - firstTime) / 1000.0;
        if (frame == 0) frame = static_cast<int>(time / dt);
    }

    deltaTime = dt;
    time += dt;
    frame++;
}

void ShaderNode::draw() {
    glBindVertexArray(vao);

    ccGLUseProgram(shader.program);

    const auto glv = CCDirector::sharedDirector()->getOpenGLView();
    const auto frSize = glv->getFrameSize() * getDisplayFactor();

    glUniform2f(uniformResolution, frSize.width, frSize.height);

    for (size_t i = 0; i < shaderSprites.size(); ++i) {
        const auto sprite = shaderSprites[i];
        ccGLBindTexture2DN(i, sprite->getTexture()->getName());
    }

    glUniform1f(uniformTime, time);
    glUniform1f(uniformDeltaTime, deltaTime);
    glUniform1f(uniformFrameRate, 1.f / deltaTime);
    glUniform1i(uniformFrame, frame);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

#if !defined(GEODE_IS_MACOS) && !defined(GEODE_IS_IOS)
    CC_INCREMENT_GL_DRAWS(1);
#endif
}

ShaderNode* ShaderNode::create(const std::string& vert, const std::string& frag, const std::vector<CCSprite*>& sprites) {
    auto node = new ShaderNode();

    for (const auto sprite : sprites) {
        node->shaderSprites.push_back(sprite);
    }

    if (node->init(vert, frag)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}

Result<ShaderNode*> ShaderNode::createFromPath(const std::string& vertPath, const std::string& fragPath, const std::vector<CCSprite*>& sprites) {
    const std::string _vertPath = vertPath.empty() ? "generic.vsh" : vertPath;

    const std::filesystem::path vertexPath = Mod::get()->getResourcesDir() / _vertPath;
    const std::filesystem::path fragmentPath = Mod::get()->getResourcesDir() / fragPath;

    auto vertexSource = file::readString(vertexPath);
    if (!vertexSource) return Err("failed to read vertex shader at path {}: {}", vertexPath.string(), vertexSource.unwrapErr());

    auto fragmentSourceRes = file::readString(fragmentPath);
    if (!fragmentSourceRes) return Err("failed to read fragment shader at path {}: {}", fragmentPath.string(), fragmentSourceRes.unwrapErr());

    auto shader = create(vertexSource.unwrap(), fragmentSourceRes.unwrap(), sprites);
    if (!shader) return Err("failed to create shader node");

    return Ok(shader);
}