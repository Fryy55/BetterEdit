#pragma once

#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

//// Free-standing helpers
namespace be {
    /**
     * Get all selected objects
     */
    std::vector<GameObject*> getSelectedObjects(EditorUI* ui);
    /**
     * Forces touch priority of a text input contained within `GJBaseGameLayer`
     */
    void evilForceTouchPrio(EditorUI* ui, CCTextInputNode* input);

    /**
     * Create a view-only editor, i.e. one where no objects can be placed nor 
     * edited
     */
    LevelEditorLayer* createViewOnlyEditor(GJGameLevel* level, std::function<CCLayer*()> returnTo);
    /**
     * Check if the given editor is view-only
     */
    bool isViewOnlyEditor(LevelEditorLayer* lel);
    /**
     * Color an object like selecting (without changing its actual color)
     */
    void tintObject(GameObject* obj, std::optional<ccColor3B> const& color);

    void enableButton(CCMenuItemSpriteExtra* btn, bool enabled, bool visualOnly = false);
    void enableToggle(CCMenuItemToggler* toggle, bool enabled, bool visualOnly = false);

    CCArray* getObjectsFromGroupDict(CCDictionary* groupDict, int groupID);

    enum class SlotType : uint8_t {
        Generic,
        Spawn,
        Center,
        Transform,
    };
    struct InputSlots final {
        std::optional<SlotType> targetGroupID;

        CCPoint calculateSlotPosition(EffectGameObject* trigger, size_t slotIndex) const;
    };
    struct OutputSlots final {
        std::optional<SlotType> targetGroupID;
        std::optional<SlotType> centerGroupID;

        CCPoint calculateSlotPosition(EffectGameObject* trigger, size_t slotIndex) const;
    };
    std::pair<InputSlots, OutputSlots> getTriggerSlots(EffectGameObject* trigger);
    std::vector<int> getTriggerTargetedGroups(EffectGameObject* trigger);
}

//// More EditCommand options
struct EditCommandExt {
    static constexpr auto QuarterLeft  = static_cast<EditCommand>(0x400);
    static constexpr auto QuarterRight = static_cast<EditCommand>(0x401);
    static constexpr auto QuarterUp    = static_cast<EditCommand>(0x402);
    static constexpr auto QuarterDown  = static_cast<EditCommand>(0x403);
    
    static constexpr auto EighthLeft   = static_cast<EditCommand>(0x404);
    static constexpr auto EighthRight  = static_cast<EditCommand>(0x405);
    static constexpr auto EighthUp     = static_cast<EditCommand>(0x406);
    static constexpr auto EighthDown   = static_cast<EditCommand>(0x407);

    static constexpr auto UnitLeft     = static_cast<EditCommand>(0x408);
    static constexpr auto UnitRight    = static_cast<EditCommand>(0x409);
    static constexpr auto UnitUp       = static_cast<EditCommand>(0x40a);
    static constexpr auto UnitDown     = static_cast<EditCommand>(0x40b);
};

//// Editor exit events - used for standardizing detecting when the editor is closed
struct EditorExitEvent : public Event {};

//// UI events - used for standard management of when the editor UI visibility is toggled
struct UIShowEvent : public Event {
    EditorUI* ui;
    bool show;

    UIShowEvent(EditorUI* ui, bool show);
};
class UIShowFilter : public EventFilter<UIShowEvent> {
protected:
    EditorUI* m_ui = nullptr;

public:
    using Callback = void(UIShowEvent*);

    UIShowFilter() = default;
    UIShowFilter(EditorUI* ui);

    ListenerResult handle(std::function<Callback> fn, UIShowEvent* ev);
};
using OnUIHide = EventListener<UIShowFilter>;
