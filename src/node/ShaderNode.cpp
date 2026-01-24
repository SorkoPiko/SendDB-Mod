#include "ShaderNode.hpp"

#include <utils/TimeUtils.hpp>
#include <include/Shader.hpp>
#include <manager/ShaderCache.hpp>

long long ShaderNode::firstTime = 0;

GLuint createTexture(const int width, const int height) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

bool ShaderNode::init(const std::string& vertPath, const std::string& fragPath) {
    const auto cachedShader = ShaderCache::get()->getShader(vertPath, fragPath);
    if (!cachedShader) {
        log::error("Failed to load shader");
        return false;
    }

    shader = cachedShader->copy();
    glBindAttribLocation(shader.program, 0, "aPosition");

    const auto linkRes = shader.link();
    if (!linkRes) {
        log::error("failed to link shader (vert: {}, frag: {}): {}", vertPath, fragPath, linkRes.unwrapErr());
        return false;
    }

    ccGLUseProgram(shader.program);

    shaderSprites.inner()->retain();
    std::istringstream stream(fragPath);
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
    uniformScreenRect = glGetUniformLocation(shader.program, "screenRect");
    uniformTime = glGetUniformLocation(shader.program, "time");
    uniformDeltaTime = glGetUniformLocation(shader.program, "deltaTime");
    uniformFrameRate = glGetUniformLocation(shader.program, "frameRate");
    uniformFrame = glGetUniformLocation(shader.program, "frame");
    uniformCurrentPass = glGetUniformLocation(shader.program, "currentPass");

    const auto glv = CCDirector::sharedDirector()->getOpenGLView();
    const auto frSize = glv->getFrameSize() * getDisplayFactor();
    updateTextures(frSize);

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
    if (numPasses <= 0) return;

    glBindVertexArray(vao);

    ccGLUseProgram(shader.program);

    const auto glv = CCDirector::sharedDirector()->getOpenGLView();
    const auto frSize = glv->getFrameSize() * getDisplayFactor();
    const auto winSize = CCDirector::sharedDirector()->getWinSize();

    updateTextures(frSize);

    const CCPoint worldPos = convertToWorldSpace(CCPointZero);
    const CCSize contentSize = getScaledContentSize();
    const CCSize contentScaleFactor = {frSize.width / winSize.width, frSize.height / winSize.height};

    const int scissorX = worldPos.x * contentScaleFactor.width;
    const int scissorY = worldPos.y * contentScaleFactor.height;
    const int scissorW = contentSize.width * contentScaleFactor.width;
    const int scissorH = contentSize.height * contentScaleFactor.height;

    if (!onlyScissorFinalPass) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissorX, scissorY, scissorW, scissorH);
    }

    GLint currentFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);

    if (passCurrentFrame) {
        GLint currentBuffer = 0;
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &currentBuffer);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[pboIndex]);

        glReadPixels(0, 0, frSize.width, frSize.height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        const int nextPboIndex = (pboIndex + 1) % 2;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[nextPboIndex]);

        glBindTexture(GL_TEXTURE_2D, pingTexture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, frSize.width, frSize.height);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, currentBuffer);
        pboIndex = nextPboIndex;
    }

    for (size_t i = 0; i < shaderSprites.size(); ++i) {
        const auto sprite = shaderSprites[i];
        ccGLBindTexture2DN(i + 1, sprite->getTexture()->getName());
    }

    glUniform2f(uniformResolution, frSize.width, frSize.height);
    glUniform4f(uniformScreenRect, scissorX, scissorY, scissorW, scissorH);
    glUniform1f(uniformTime, time);
    glUniform1f(uniformDeltaTime, deltaTime);
    glUniform1f(uniformFrameRate, 1.f / deltaTime);
    glUniform1i(uniformFrame, frame);

    for (int pass = 0; pass < numPasses; ++pass) {
        const bool isLastPass = pass == numPasses - 1;

        const GLuint readTexture = pass % 2 == 0 ? pingTexture : pongTexture;
        const GLuint writeFBO = isLastPass ? currentFbo : pass % 2 == 0 ? pongFBO : pingFBO;

        glBindFramebuffer(GL_FRAMEBUFFER, writeFBO);
        if (passCurrentFrame) ccGLBindTexture2DN(0, readTexture);

        glUniform1i(uniformCurrentPass, pass);

        if (isLastPass && onlyScissorFinalPass) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(scissorX, scissorY, scissorW, scissorH);
        }

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    for (size_t i = 0; i <= shaderSprites.size(); ++i) {
        ccGLBindTexture2DN(i, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
    glBindVertexArray(0);
    glDisable(GL_SCISSOR_TEST);

#if !defined(GEODE_IS_MACOS) && !defined(GEODE_IS_IOS)
    CC_INCREMENT_GL_DRAWS(numPasses);
#endif
}

void ShaderNode::updateTextures(const CCSize& frSize) {
    if (frSize == lastSize) return;
    lastSize = frSize;

    if (pingTexture != 0) glDeleteTextures(1, &pingTexture);
    if (pongTexture != 0) glDeleteTextures(1, &pongTexture);
    if (pingFBO != 0) glDeleteFramebuffers(1, &pingFBO);
    if (pongFBO != 0) glDeleteFramebuffers(1, &pongFBO);
    if (pbos[0] != 0) glDeleteBuffers(2, pbos);

    pingTexture = createTexture(frSize.width, frSize.height);
    pongTexture = createTexture(frSize.width, frSize.height);

    GLint currentFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFbo);

    glGenFramebuffers(1, &pingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pingFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingTexture, 0);

    glGenFramebuffers(1, &pongFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pongFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pongTexture, 0);

    GLint currentBuffer = 0;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &currentBuffer);

    glGenBuffers(2, pbos);
    const size_t bufferSize = frSize.width * frSize.height * 4;
    for (int i = 0; i < 2; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbos[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, nullptr, GL_STREAM_READ);
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, currentBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);
}


ShaderNode* ShaderNode::create(const std::string& vertPath, const std::string& fragPath, const std::vector<CCSprite*>& sprites) {
    auto node = new ShaderNode();

    for (const auto sprite : sprites) node->shaderSprites.push_back(sprite);

    if (node->init(vertPath, fragPath)) {
        node->autorelease();
        return node;
    }
    CC_SAFE_DELETE(node);
    return nullptr;
}