#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/DrawGridLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GJGroundLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <utils/Editor.hpp>
#include <utils/HolyUB.hpp>
#include <utils/Pro.hpp>

#ifdef GEODE_IS_DESKTOP
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;
#endif

using namespace geode::prelude;

// EditButtonBar expects CCMenuItemSpriteExtra, CCMenuItemToggler crashes the game
class BEMenuItemToggler : public CCMenuItemSpriteExtra {
protected:
    std::function<bool()> m_getter;
    std::function<void(bool)> m_setter;
    CCNode* m_offNode = nullptr;
    CCNode* m_onNode = nullptr;
    bool m_toggled = false;

public:
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

class $modify(GameObjectExtra, GameObject) {
    void updateVisibility() {
        this->setVisible(m_bVisible || !this->shouldHide());
    }
    $override
    void setVisible(bool visible) {
        if (this->shouldHide()) {
            visible = false;
        }
        GameObject::setVisible(visible);
    }
    bool shouldHide() const {
        return
            m_isHighDetail &&
            // Only in the editor
            EditorUI::get() && 
            Mod::get()->template getSavedValue<bool>("hide-ldm", false);
    }
};

struct $modify(ViewTabUI, EditorUI) {
    struct Fields {
        CCNode* viewModeBtn;
        OnUIHide onUIHide;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorUI::selectObject",  3000);
        (void)self.setHookPriority("EditorUI::selectObjects", 3000);
    }

    void updateModeSprite(CCNode* node, int tag, const char* spr) {
        // resetUI resets the button sprites so we may have to change them back 
        // to squarish ones again
        if (node) {
            auto bg = m_selectedMode == tag ? "GJ_button_02.png" : "GJ_button_01.png";
            auto btn = static_cast<CCMenuItemSpriteExtra*>(node);
            if (auto bspr = typeinfo_cast<ButtonSprite*>(btn->getNormalImage())) {
                // remove GD's texture
                bspr->setTexture(nullptr);
                bspr->setTextureRect({ 0, 0, 0, 0 });
                bspr->updateBGImage(bg);
            }
            else {
                btn->setNormalImage(ButtonSprite::create(
                    CCSprite::createWithSpriteFrameName(spr), 50, true, 50, bg, .6f
                ));
            }
        }
    }
    void updateModeSprites() {
        this->updateModeSprite(m_buildModeBtn, 2, "tab-create.png"_spr);
        this->updateModeSprite(m_deleteModeBtn, 1, "tab-delete.png"_spr);
        this->updateModeSprite(m_editModeBtn, 3, "tab-edit.png"_spr);
        this->updateModeSprite(m_fields->viewModeBtn, 4, "tab-view.png"_spr);
    }

    ButtonSprite* createViewToggleSpr(const char* frame, bool selected) {
        auto top = CCSprite::createWithSpriteFrameName(frame);
        return ButtonSprite::create(
            top, 50, 0, 50, .8f, true, (selected ? "GJ_button_02.png" : "GJ_button_01.png"), true
        );
    }
    BEMenuItemToggler* createViewToggle(const char* frame, auto get, auto set) {
        auto off = createViewToggleSpr(frame, false);
        auto on  = createViewToggleSpr(frame, true);
        auto toggler = BEMenuItemToggler::create(off, on, get, set);
        return toggler;
    }
    BEMenuItemToggler* createViewToggleGV(const char* frame, const char* gv, std::function<void(bool)> postSet = nullptr) {
        auto off = createViewToggleSpr(frame, false);
        auto on  = createViewToggleSpr(frame, true);
        auto toggler = BEMenuItemToggler::create(off, on, [gv]() {
            return GameManager::get()->getGameVariable(gv);
        }, [this, gv, postSet](bool enabled) {
            GameManager::get()->setGameVariable(gv, enabled);
            postSet ? postSet(enabled) : m_editorLayer->updateOptions();
        });
        return toggler;
    }
    BEMenuItemToggler* createViewToggleMSV(
        const char* frame, const char* modSavedValue,
        bool defaultValue = false, std::function<void(bool)> postSet = nullptr
    ) {
        auto off = createViewToggleSpr(frame, false);
        auto on  = createViewToggleSpr(frame, true);
        auto toggler = BEMenuItemToggler::create(off, on, [modSavedValue, defaultValue]() {
            return Mod::get()->template getSavedValue<bool>(modSavedValue, defaultValue);
        }, [modSavedValue, postSet](bool enabled) {
            Mod::get()->setSavedValue(modSavedValue, enabled);
            if (postSet) {
                postSet(enabled);
            }
        });
        return toggler;
    }

    void updateViewTab() {
        if (auto bbar = static_cast<EditButtonBar*>(this->getChildByID("view-tab"_spr))) {
            for (auto toggle : CCArrayExt<BEMenuItemToggler*>(bbar->m_buttonArray)) {
                toggle->toggle();
            }
        }
    }

    $override
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel))
            return false;
        
        if (!Mod::get()->template getSettingValue<bool>("view-menu")) {
            return true;
        }
        
        auto winSize = CCDirector::get()->getWinSize();
        
        // Make a bit space for new style menu since the old one is a tiny bit cramped
        if (auto left = this->getChildByType<CCSprite>(1), right = this->getChildByType<CCSprite>(2); left && right) {
            left->setPositionX(winSize.width - right->getPositionX());
        }
        for (auto& child : CCArrayExt<CCNode*>(m_pChildren)) {
            if (auto bar = typeinfo_cast<EditButtonBar*>(child)) {
                bar->setPositionX(bar->getPositionX() + 5);
            }
        }

        if (auto menu = this->getChildByID("toolbar-categories-menu")) {
            m_fields->viewModeBtn = CCMenuItemSpriteExtra::create(
                CCNode::create(), this, menu_selector(EditorUI::toggleMode)
            );
            m_fields->viewModeBtn->setID("view-button"_spr);
            m_fields->viewModeBtn->setTag(4);
            menu->addChild(m_fields->viewModeBtn);

            this->updateModeSprites();

            menu->setContentSize({ 90, 90 });
            if (auto otherSide = this->getChildByID("toolbar-toggles-menu")) {
                menu->setPositionX(winSize.width - otherSide->getPositionX());
            }
            menu->setLayout(RowLayout::create()
                ->setCrossAxisOverflow(false)
                ->setGrowCrossAxis(true)
            );
        }

    #ifdef GEODE_IS_DESKTOP
        this->template addEventListener<InvokeBindFilter>([=, this](InvokeBindEvent* event) {
            if (event->isDown() && m_editorLayer->m_playbackMode == PlaybackMode::Not) {
                this->toggleMode(m_fields->viewModeBtn);
            }
            return ListenerResult::Propagate;
        }, "view-mode"_spr);
    #endif

        // Create buttons
        auto btns = CCArray::create();

        // For some reason using a class that inherits from CCMenuItemToggler 
        // for the view toggles was causing super weird crashes, so will have 
        // to make due with this :/

        btns->addObject(this->createViewToggle(
            "v-rotation.png"_spr,
            [] { return GameManager::get()->getGameVariable("0118"); },
            [this](bool) {
                fakeEditorPauseLayer(m_editorLayer)->togglePreviewAnim(nullptr);
            }
        ));
        btns->addObject(this->createViewToggleGV("v-particles.png"_spr, "0117", [this](bool) {
            m_editorLayer->updatePreviewParticles();
        }));
        btns->addObject(this->createViewToggleGV("v-shaders.png"_spr, "0158"));
        btns->addObject(this->createViewToggleMSV("v-ldm.png"_spr, "hide-ldm", false, [this](bool) {
            for (auto obj : CCArrayExt<GameObjectExtra*>(m_editorLayer->m_objects)) {
                obj->updateVisibility();
            }
        }));
        btns->addObject(this->createViewToggleGV("v-preview-mode.png"_spr, "0036", [this](bool) {
            // Let's not be funny and ruin everyone's levels
            if (m_editorLayer->m_playbackMode != PlaybackMode::Not) {
                // Why was this being called separately? `onStopPlaytest` already calls it
                // m_editorLayer->resetMovingObjects();
                this->onStopPlaytest(m_playtestBtn);
            }
            m_editorLayer->updateEditorMode();
        }));
        btns->addObject(this->createViewToggle(
            "v-bpm-lines.png"_spr,
            [] { return GameManager::get()->m_showSongMarkers; },
            [](bool enable) {
                GameManager::get()->m_showSongMarkers = enable;
            }
        ));
        btns->addObject(this->createViewToggleMSV("v-position-line.png"_spr, "pos-line"));
        btns->addObject(this->createViewToggleGV("v-duration-lines.png"_spr, "0058"));
        btns->addObject(this->createViewToggleGV("v-effect-lines.png"_spr, "0043"));
        btns->addObject(this->createViewToggleGV("v-ground.png"_spr, "0037", [this](bool enable) {
            m_editorLayer->m_groundLayer->setVisible(enable);
        }));
        btns->addObject(this->createViewToggleGV("v-grid.png"_spr, "0038"));
        btns->addObject(this->createViewToggleMSV("v-dash-lines.png"_spr, "show-dash-lines"));

    #ifdef BETTEREDIT_PRO
        auto ttt = this->createViewToggleMSV(
            "v-indicators-trigger-to-trigger.png"_spr,
            "trigger-indicators-trigger-to-trigger"
        );
        auto clusterOutlines = this->createViewToggleMSV(
            "v-indicators-cluster-outline.png"_spr,
            "trigger-indicators-cluster-outlines"
        );
        // todo: show this one as disabled if there are too many objects
        auto showAll = this->createViewToggleMSV(
            "v-indicators-all.png"_spr,
            "trigger-indicators-show-all", HAS_PRO(),
            [clusterOutlines](bool enabled) {
                be::enableToggle(clusterOutlines, enabled);
            }
        );
        auto blocky = this->createViewToggleMSV(
            "v-indicators-blocky.png"_spr,
            "trigger-indicators-blocky"
        );
        std::array<CCMenuItemToggler*, 4> indToggles { ttt, showAll, clusterOutlines, blocky };
        auto indToggle = this->createViewToggleMSV(
            "v-indicators.png"_spr, "show-trigger-indicators", HAS_PRO(),
            [indToggles, clusterOutlines, showAll](bool enabled) {
                for (auto toggle : indToggles) {
                    be::enableToggle(toggle, enabled);
                }
                // todo: come up with some more robust system for this
                be::enableToggle(clusterOutlines, showAll->isToggled());
            }
        );
        btns->addObject(indToggle);
        for (auto toggle : indToggles) {
            btns->addObject(toggle);
        }

        if (!HAS_PRO()) {
            be::enableToggle(indToggle, false, true);
            indToggle->setUserObject(CCString::create("<cj>Trigger Indicators</c>"));
            indToggle->setTarget(this, menu_selector(ViewTabUI::onProOnlyFeature));
            for (auto toggle : indToggles) {
                be::enableToggle(toggle, false, true);
                toggle->setUserObject(CCString::create("<cj>Trigger Indicators</c>"));
                toggle->setTarget(this, menu_selector(ViewTabUI::onProOnlyFeature));
            }
        }
    #endif

        auto buttonBar = EditButtonBar::create(
            btns,
            ccp(winSize.width / 2 - 5, CCDirector::get()->getScreenBottom() + m_toolbarHeight - 6),
            m_tabsArray->count(), false,
            GameManager::get()->getIntGameVariable("0049"),
            GameManager::get()->getIntGameVariable("0050")
        );
        buttonBar->setID("view-tab"_spr);
        // Need to set a tag too so GD doesn't accidentally grab this tab 
        // when doing getChildByTag for its tabs
        buttonBar->setTag(-1);
        buttonBar->setVisible(m_selectedMode == 4);
        this->addChild(buttonBar, 10);

        this->updateViewTab();

        m_fields->onUIHide.setFilter(this);
        m_fields->onUIHide.bind([this](UIShowEvent* ev) {
            m_fields->viewModeBtn->setVisible(ev->show);
            this->getChildByID("view-tab"_spr)->setVisible(ev->show && m_selectedMode == 4);
            m_buildModeBtn->getParent()->getChildByTag(4)->setVisible(ev->show);
        });

        return true;
    }

    void toggleMode(CCObject* sender) {
        EditorUI::toggleMode(sender);
        if (auto viewBtnBar = this->getChildByID("view-tab"_spr)) {
            // this->resetUI();
            this->updateModeSprites();
            
            viewBtnBar->setVisible(m_selectedMode == 4);
        }
    }


    #ifdef GEODE_IS_MACOS // toggleMode is inlined into onPlaytest on macOS
    $override
    void onPlaytest(CCObject* sender) {
        auto playbackMode = m_editorLayer->m_playbackMode;
        EditorUI::onPlaytest(sender);
        if (!m_isPaused && playbackMode != PlaybackMode::Playing) {
            if (auto viewBtnBar = this->getChildByID("view-tab"_spr)) {
                this->updateModeSprites();
                viewBtnBar->setVisible(m_selectedMode == 4);
            }
        }
    }
    #endif

    $override
    void selectObject(GameObject* obj, bool filter) {
        if (!static_cast<GameObjectExtra*>(obj)->shouldHide()) {
            EditorUI::selectObject(obj, filter);
        }
    }
    $override
    void selectObjects(CCArray* objs, bool ignoreFilters) {
        // filter out LDM objects
        if (objs) {
            unsigned int objCount = objs->count();
            for (unsigned int i = 0; i < objCount; i++) {
                auto obj = static_cast<GameObjectExtra*>(objs->objectAtIndex(i));
                if (obj->shouldHide()) {
                    objs->removeObjectAtIndex(i, false);
                }
                i -= 1;
                objCount -= 1;
            }
        }
        EditorUI::selectObjects(objs, ignoreFilters);
    }

    void onProOnlyFeature(CCObject* sender) {
    #ifdef BETTEREDIT_PRO
        pro::showProOnlyFeaturePopup(
            static_cast<CCString*>(static_cast<CCNode*>(sender)->getUserObject())->getCString()
        );
    #endif
    }
};

class $modify(DrawGridLayer) {
    $override
    void draw() {
        bool origBool = m_editorLayer->m_previewMode;
        m_editorLayer->m_previewMode = Mod::get()->template getSavedValue<bool>("pos-line");
        DrawGridLayer::draw();
        m_editorLayer->m_previewMode = origBool;
    }
};

class $modify(EditorPauseLayer) {
    struct Fields {
        LevelEditorLayer* m_editorLayer;
        ~Fields() {
            if (m_editorLayer && m_editorLayer->m_editorUI) {
                static_cast<ViewTabUI*>(m_editorLayer->m_editorUI)->updateViewTab();
            }
        }
    };

    bool init(LevelEditorLayer* lel) {
        if (!EditorPauseLayer::init(lel))
            return false;

        m_fields->m_editorLayer = lel;

        return true;
    }
};

#ifdef GEODE_IS_DESKTOP
$execute {
    BindManager::get()->registerBindable(BindableAction(
        "view-mode"_spr,
        "View Mode",
        "Toggle the View Tab",
        { Keybind::create(KEY_Four) },
        Category::EDITOR_UI
    ), "robtop.geometry-dash/delete-mode");
}
#endif
