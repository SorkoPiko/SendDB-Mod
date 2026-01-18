#ifndef SENDDB_SENDINFOBOX_HPP
#define SENDDB_SENDINFOBOX_HPP

#include <Geode/cocos/base_nodes/CCNode.h>
#include <Geode/cocos/label_nodes/CCLabelBMFont.h>
#include <model/APIResponse.hpp>

using namespace geode::prelude;

class SendInfoBox : public CCNode {
    std::optional<Send> sendData;
    std::optional<Rate> rateData;
    int sendIndex = 0;

    CCLabelBMFont* infoLabel = nullptr;
    CCLabelBMFont* timeLabel = nullptr;
    CCLabelBMFont* timeLabel2 = nullptr;

    bool init() override;
    void update(float delta) override;

    void updateSend() const;
    void updateRate() const;

    void prepareSend();

    void prepareRate();

public:
    static SendInfoBox* create();

    void setSendData(const std::optional<Send>& send, int index = 0);
    void setRateData(const std::optional<Rate>& rate);
    void clearData();
};

#endif