#pragma once

#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

class BEMenuItemToggler : public CCMenuItemSpriteExtra {
public:
    std::function<bool()> m_getter;
    std::function<void(bool)> m_setter;
    CCNode* m_offNode = nullptr;
    CCNode* m_onNode = nullptr;
    bool m_toggled = false;

    static BEMenuItemToggler* create(CCNode* offNode, CCNode* onNode, auto getter, auto setter) {
        auto ret = new BEMenuItemToggler();
        if (ret && ret->init(offNode, onNode, getter, setter)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(CCNode* offNode, CCNode* onNode, auto getter, auto setter) {
        if (!CCMenuItemSpriteExtra::init(offNode, nullptr, nullptr, nullptr)) {
            return false;
        }

        m_offNode = offNode;
        m_offNode->retain();
        m_onNode = onNode;
        m_onNode->retain();
        m_getter = getter;
        m_setter = setter;

        return true;
    }

    void toggle() {
        this->toggle(m_getter());
    }
    void toggle(bool toggled) {
        m_toggled = toggled;
        this->setNormalImage(toggled ? m_onNode : m_offNode);
        this->updateSprite();
    }
    bool isToggled() const {
        return m_toggled;
    }

    void activate() override {
        toggle(!m_toggled);
        m_setter(m_toggled);
        CCMenuItemSpriteExtra::activate();
    }

    ~BEMenuItemToggler() {
        CC_SAFE_RELEASE(m_offNode);
        CC_SAFE_RELEASE(m_onNode);
    }
};
