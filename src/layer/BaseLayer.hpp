#ifndef SENDDB_BASELAYER_HPP
#define SENDDB_BASELAYER_HPP

#include <Geode/Prelude.hpp>
#include <Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h>

using namespace geode::prelude;

class BaseLayer : public CCLayer {
    void keyBackClicked() override;

public:
    bool init() override;

    void initShaderBackground(const std::string& fragPath);

    BaseLayer() = default;
    BaseLayer(const BaseLayer&) = delete;
    BaseLayer& operator=(const BaseLayer&) = delete;
    BaseLayer(BaseLayer&&) = delete;
    BaseLayer& operator=(BaseLayer&&) = delete;
};

#endif