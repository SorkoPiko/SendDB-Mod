#pragma once

#include <unordered_map>

#include <utils/Shader.hpp>
#include <utils/HashUtils.hpp>

struct ShaderKey {
    std::string vertexPath;
    std::string fragmentPath;

    bool operator==(const ShaderKey& other) const {
        return vertexPath == other.vertexPath && fragmentPath == other.fragmentPath;
    }
};

template <>
struct std::hash<ShaderKey> {
    size_t operator()(const ShaderKey& key) const noexcept {
        size_t seed = 0;
        hash_combine(seed, key.vertexPath);
        hash_combine(seed, key.fragmentPath);
        return seed;
    }
};

class ShaderCache {
    static ShaderCache* instance;
    std::unordered_map<ShaderKey, std::optional<Shader>> cache;

    std::unordered_map<std::string, std::string> vertexSourceCache;
    std::unordered_map<std::string, std::string> fragmentSourceCache;

    std::optional<std::string> getSource(std::unordered_map<std::string, std::string>& sourceCache, const std::string& path);
    const Shader* tryLoadShader(const ShaderKey& shaderKey);

public:
    static ShaderCache* get() {
        if (!instance) instance = new ShaderCache();
        return instance;
    }

    const Shader* getShader(const std::string& vertexPath, const std::string& fragmentPath);
};