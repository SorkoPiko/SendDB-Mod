#pragma once

#include <fmod.hpp>
#include <Geode/Prelude.hpp>

#include <vector>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

class AudioManager : public CCObject {
    std::vector<CCNode*> sources;
    FMOD::Channel* channel = nullptr;
    float musicVolume;
    bool musicVolumeSet = false;

    AudioManager();

    void update(float delta) override;
    void pulseSources() const;

public:
    static AudioManager* get() {
        static auto instance = new AudioManager();
        return instance;
    }

    void registerSource(CCNode* node);
    void unregisterSource(CCNode* node);
};