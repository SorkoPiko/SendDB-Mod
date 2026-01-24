#include "ShaderCache.hpp"

ShaderCache* ShaderCache::instance = nullptr;

std::optional<std::string> ShaderCache::getSource(std::unordered_map<std::string, std::string>& sourceCache, const std::string& path) {
    if (const auto it = sourceCache.find(path); it != sourceCache.end()) {
        return it->second;
    }
    return std::nullopt;
}

const Shader* ShaderCache::tryLoadShader(const ShaderKey& shaderKey) {
    std::string vertexSource = "";
    if (const auto cachedVertexSource = getSource(vertexSourceCache, shaderKey.vertexPath)) {
        vertexSource = *cachedVertexSource;
    } else {
        const std::filesystem::path vertexPath = Mod::get()->getResourcesDir() / shaderKey.vertexPath;
        const auto vertexSourceFs = file::readString(vertexPath);
        if (!vertexSourceFs) {
            log::error("failed to read vertex shader at path {}: {}", vertexPath.string(), vertexSourceFs.unwrapErr());
            return nullptr;
        }
        vertexSource = *vertexSourceFs;
        vertexSourceCache[shaderKey.vertexPath] = vertexSource;
    }

    std::string fragmentSource = "";
    if (const auto cachedFragmentSource = getSource(fragmentSourceCache, shaderKey.fragmentPath)) {
        fragmentSource = *cachedFragmentSource;
    } else {
        const std::filesystem::path fragmentPath = Mod::get()->getResourcesDir() / shaderKey.fragmentPath;
        const auto fragmentSourceFs = file::readString(fragmentPath);
        if (!fragmentSourceFs) {
            log::error("failed to read fragment shader at path {}: {}", fragmentPath.string(), fragmentSourceFs.unwrapErr());
            return nullptr;
        }
        fragmentSource = *fragmentSourceFs;
        fragmentSourceCache[shaderKey.fragmentPath] = fragmentSource;
    }

    const auto compileRes = Shader::compile(vertexSource, fragmentSource);
    if (!compileRes) {
        log::error("failed to compile shader (vert: {}, frag: {}): {}", shaderKey.vertexPath, shaderKey.fragmentPath, compileRes.unwrapErr());
        return nullptr;
    }

    cache[shaderKey] = std::move(compileRes.unwrap());

    return &cache[shaderKey].value();
}

const Shader* ShaderCache::getShader(const std::string& vertexPath, const std::string& fragmentPath) {
    const auto key = ShaderKey(vertexPath, fragmentPath);

    if (const auto it = cache.find(key); it != cache.end()) {
        if (auto val = it->second) {
            if (val) return &val.value();
            return nullptr;
        }
    }

    return tryLoadShader(key);
}