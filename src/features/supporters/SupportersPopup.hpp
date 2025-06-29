#pragma once

#include <features/supporters/Pro.hpp>
#include <Geode/ui/Popup.hpp>
#include <server/Server.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

using namespace geode::prelude;

namespace pro {
    class SupportersPopup : public Popup<> {
    protected:
        EventListener<server::ServerRequest<server::Supporters>> m_reqListener;
        EventListener<server::ServerRequest<server::MySupport>> m_mySupportListener;
        CCMenuItemSpriteExtra* m_prevPageBtn;
        CCMenuItemSpriteExtra* m_nextPageBtn;
        CCLabelBMFont* m_pageLabel;
        LoadingSpinner* m_loadingCircle;
        CCLabelBMFont* m_errorLabel;
        CCNode* m_supportersList;
        size_t m_page = 0;
        size_t m_maxPage = 0;
        CCMenu* m_mySupportMenu = nullptr;

        bool setup() override;
        void loadPage(size_t page);
        void onLoadPage(server::ServerRequest<server::Supporters>::Event* event);
        void onLoadMySupport(server::ServerRequest<server::MySupport>::Event* event);
        void updatePageInfo();

        void onPage(CCObject*);
        void onSupporter(CCObject*);
        void onShowMe(CCObject*);

    public:
        static SupportersPopup* create();
    };
}
