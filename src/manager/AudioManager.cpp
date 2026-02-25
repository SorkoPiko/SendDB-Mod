#include "AudioManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/cocos/CCScheduler.h>
#include <Geode/utils/cocos.hpp>

constexpr float bpm = 145.0f;
constexpr float maxAudioDistance = 75.0f;
constexpr float maxPulseDistance = 150.0f;

FMOD::Channel* create() {
    FMOD::Sound* sound;
    FMOD::Channel* channel;
    FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

    system->createSound(
        string::pathToString(Mod::get()->getResourcesDir() / "club.mp3").c_str(),
        FMOD_DEFAULT | FMOD_LOOP_NORMAL | FMOD_2D,
        nullptr,
        &sound
    );
    system->playSound(sound, nullptr, false, &channel);
    channel->setVolume(0.0f);
    return channel;
}

AudioManager::AudioManager() {
    CCScheduler::get()->scheduleSelector(schedule_selector(AudioManager::update), this, 0.0f, false);
    channel = create();
}

void AudioManager::update(const float delta) {
    if (!channel) return;

    bool isPlaying = false;
    channel->isPlaying(&isPlaying);
    if (!isPlaying) channel = create();

    if (sources.empty()) {
        channel->setVolume(0.0f);
        return;
    }

    pulseSources();

    if (Mod::get()->getSettingValue<bool>("removeAnimations")) {
        channel->setVolume(0.0f);
        return;
    }

    const CCPoint pos = getMousePos();
    if (pos.equals(CCPointZero)) {
        channel->setVolume(0.0f);
        return;
    }

    std::vector<CCPoint> distanceSorted;
    for (const auto& source : sources) {
        if (!source) continue;
        distanceSorted.push_back(source->convertToWorldSpace(source->getContentSize() * source->getAnchorPoint()));
    }
    std::ranges::sort(distanceSorted, [&pos](const CCPoint& a, const CCPoint& b) {
        return (a - pos).getLengthSq() < (b - pos).getLengthSq();
    });

    const CCPoint closestPoint = distanceSorted.front();
    const float distance = (closestPoint - pos).getLength();

    const float volume = 1.0f - std::pow(std::min(distance / maxAudioDistance, 1.0f), 3);
    channel->setVolume(volume * 2.0f);
}

void AudioManager::pulseSources() const {
    if (!channel || sources.empty()) return;

    if (Mod::get()->getSettingValue<bool>("removeAnimations")) {
        for (const auto& source : sources) {
            if (!source) continue;
            source->setScale(1.0f);
        }
        return;
    }

    const CCPoint pos = getMousePos();

    unsigned int position = 0;
    channel->getPosition(&position, FMOD_TIMEUNIT_MS);
    const float timeInSeconds = position / 1000.0f;

    constexpr float beatDuration = 60.0f / bpm;
    const float beatProgress = std::fmod(timeInSeconds, beatDuration) / beatDuration;

    const float pulse = pow(beatProgress / 2.0f - 1, 2);

    for (const auto& source : sources) {
        if (!source) continue;

        const CCPoint sourcePos = source->convertToWorldSpace(source->getContentSize() * source->getAnchorPoint());
        const float distance = (sourcePos - pos).getLength();
        const float mult = 1.0f - std::min(distance / maxPulseDistance, 1.0f) * 0.7f;

        const float scale = 1.0f + pulse * 0.25f * mult;
        source->setScale(scale);
    }
}

void AudioManager::registerSource(CCNode* node) {
    sources.emplace_back(node);
}

void AudioManager::unregisterSource(CCNode* node) {
    std::erase_if(sources, [node](const CCNode* source) {
        return source == node;
    });
}