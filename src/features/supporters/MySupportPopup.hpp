#pragma once

#include <Geode/DefaultInclude.hpp>
#include <utils/PopupWithCorners.hpp>
#include <Geode/ui/LoadingSpinner.hpp>
#include <server/Server.hpp>

using namespace geode::prelude;

class MySupportPopup : public PopupWithCorners<> {
protected:
    EventListener<pro::server::ServerRequest<pro::server::MySupport>> m_mySupportListener;
    CCNode* m_playerInfo;
    SimplePlayer* m_playerIcon;
    CCLabelBMFont* m_playerName;
    LoadingSpinner* m_playerInfoLoading;
    CCMenuItemToggler* m_showMeToggle;
    LoadingSpinner* m_showMeToggleLoading;
    CCLabelBMFont* m_supporterSince;
    LoadingSpinner* m_supporterSinceLoading;
    CCNode* m_devicesList;
    LoadingSpinner* m_devicesLoading;
    CCLabelBMFont* m_errorLabel;

    bool setup() override;

    void reloadData();
    void onLoadData(pro::server::ServerRequest<pro::server::MySupport>::Event* event);

    void onReload(CCObject*);
    void onShowMe(CCObject* sender);
    void onDevicesInfo(CCObject*);
    void onDeleteDevice(CCObject* sender);
    void onActivateNewDevice(CCObject*);
    void onUpdateDeviceName(CCObject* sender);
    void setError(std::string const& error);

public:
    static MySupportPopup* create();

    void onClose(CCObject* sender) override;
};
