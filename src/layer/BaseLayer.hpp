#ifndef SENDDB_BASELAYER_HPP
#define SENDDB_BASELAYER_HPP

#include <Geode/Prelude.hpp>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h>
#include <node/ShaderNode.hpp>

using namespace geode::prelude;

class BaseLayer : public CCLayer {
    void keyBackClicked() override;

protected:
    ShaderNode* backgroundShader = nullptr;

public:
    bool init() override;

    void initShaderBackground(const std::string& fragPath);

    void initBackground();

    BaseLayer() = default;
    BaseLayer(const BaseLayer&) = delete;
    BaseLayer& operator=(const BaseLayer&) = delete;
    BaseLayer(BaseLayer&&) = delete;
    BaseLayer& operator=(BaseLayer&&) = delete;
};

#endif