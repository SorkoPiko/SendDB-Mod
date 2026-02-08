#pragma once

#include <Geode/Geode.hpp>

#include <ctre.hpp>

using namespace geode::prelude;

struct Shader {
    GLuint vertex = 0;
    GLuint fragment = 0;
    GLuint program = 0;

    static Result<Shader> compile(
        std::string vertexSource,
        std::string fragmentSource
    ) {
        Shader shader;

        vertexSource = string::trim(vertexSource);
        if (const auto match = ctre::multiline_search<"^#version [0-9]+( core| compatibility|)$">(vertexSource)) {
            vertexSource.erase(match.get<0>().begin(), match.get<0>().end());
            log::warn("For shader developers: #version is unsupported! Always forced to 120 on Windows and undefined on macOS and mobile.");
        }
        if (const auto match = ctre::multiline_search<"precision [a-zA-Z]+ [a-zA-Z]+;">(vertexSource)) {
            vertexSource.erase(match.get<0>().begin(), match.get<0>().end());
            log::warn("For shader developers: precision is unsupported! Always forced to undefined on desktop and highp on mobile.");
        }

        fragmentSource = string::trim(fragmentSource);
        if (auto match = ctre::multiline_search<"^#version [0-9]+( core| compatibility|)$">(fragmentSource)) {
            fragmentSource.erase(match.get<0>().begin(), match.get<0>().end());
            log::warn("For shader developers: #version is unsupported! Always forced to 120 on Windows and undefined on macOS and mobile.");
        }
        if (auto match = ctre::multiline_search<"precision [a-zA-Z]+ [a-zA-Z]+;">(fragmentSource)) {
            fragmentSource.erase(match.get<0>().begin(), match.get<0>().end());
            log::warn("For shader developers: precision is unsupported! Always forced to undefined on desktop and highp on mobile.");
        }

        auto getShaderLog = [](const GLuint id) -> std::string {
            GLint length, written;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            if (length <= 0) return "";
            const auto stuff = new char[length + 1];
            glGetShaderInfoLog(id, length, &written, stuff);
            std::string result(stuff);
            delete[] stuff;
            return result;
        };
        GLint res;

        shader.vertex = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSources[] = {
#ifdef GEODE_IS_WINDOWS
            "#version 120\n",
#elifdef GEODE_IS_MOBILE
            "precision highp float;\n",
#endif
            vertexSource.c_str()
        };
        glShaderSource(shader.vertex, sizeof(vertexSources) / sizeof(char*), vertexSources, nullptr);
        glCompileShader(shader.vertex);
        auto vertexLog = string::trim(getShaderLog(shader.vertex));

        glGetShaderiv(shader.vertex, GL_COMPILE_STATUS, &res);
        if (!res) {
            glDeleteShader(shader.vertex);
            shader.vertex = 0;
            return Err("vertex shader compilation failed:\n{}", vertexLog);
        }

        if (vertexLog.empty()) log::debug("vertex shader compilation successful");
        else log::debug("vertex shader compilation successful:\n{}", vertexLog);

        shader.fragment = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSources[] = {
#ifdef GEODE_IS_WINDOWS
            "#version 120\n",
#elifdef GEODE_IS_MOBILE
            "precision highp float;\n",
#endif
            fragmentSource.c_str()
        };
        glShaderSource(shader.fragment, sizeof(fragmentSources) / sizeof(char*), fragmentSources, nullptr);
        glCompileShader(shader.fragment);
        auto fragmentLog = string::trim(getShaderLog(shader.fragment));

        glGetShaderiv(shader.fragment, GL_COMPILE_STATUS, &res);
        if (!res) {
            glDeleteShader(shader.vertex);
            glDeleteShader(shader.fragment);
            shader.vertex = 0;
            shader.fragment = 0;
            return Err("fragment shader compilation failed:\n{}", fragmentLog);
        }

        if (fragmentLog.empty()) log::debug("fragment shader compilation successful");
        else log::debug("fragment shader compilation successful:\n{}", fragmentLog);

        return Ok(std::move(shader));
    }

    Result<> link() {
        if (!vertex) return Err("vertex shader not compiled");
        if (!fragment) return Err("fragment shader not compiled");

        auto getProgramLog = [](const GLuint id) -> std::string {
            GLint length, written;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
            if (length <= 0) return "";
            const auto log = new char[length + 1];
            glGetProgramInfoLog(id, length, &written, log);
            std::string result(log);
            delete[] log;
            return result;
        };
        GLint res;


        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        const auto programLog = string::trim(getProgramLog(program));

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        vertex = 0;
        fragment = 0;

        glGetProgramiv(program, GL_LINK_STATUS, &res);
        if (!res) {
            glDeleteProgram(program);
            program = 0;
            return Err("shader link failed:\n{}", programLog);
        }

        if (programLog.empty()) log::debug("shader link successful");
        else log::debug("shader link successful:\n{}", programLog);

        return Ok();
    }

    Shader copy() const {
        Shader newShader;
        newShader.vertex = vertex;
        newShader.fragment = fragment;
        newShader.program = program;
        return newShader;
    }

    void cleanup() {
        if (program) glDeleteProgram(program);
        program = 0;
    }
};