#pragma once

#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

namespace object_ids {
    static constexpr int MOVE_TRIGGER = 901;
    static constexpr int STOP_TRIGGER = 1616;
    static constexpr int PULSE_TRIGGER = 1006;
    static constexpr int ALPHA_TRIGGER = 1007;
    static constexpr int TOGGLE_TRIGGER = 1049;
    static constexpr int SPAWN_TRIGGER = 1268;
    static constexpr int ROTATE_TRIGGER = 1346;
    static constexpr int SCALE_TRIGGER = 2067;
    static constexpr int FOLLOW_TRIGGER = 1347;
    static constexpr int ANIMATE_TRIGGER = 1585;
    static constexpr int FOLLOW_PLAYER_Y_TRIGGER = 1814;
    static constexpr int ADVANCED_FOLLOW_TRIGGER = 3016;
    static constexpr int EDIT_ADVANCED_FOLLOW_TRIGGER = 3660;
    static constexpr int RE_TARGET_ADVANCED_FOLLOW_TRIGGER = 3661;
    static constexpr int AREA_MOVE_TRIGGER = 3006;
    static constexpr int AREA_ROTATE_TRIGGER = 3007;
    static constexpr int AREA_SCALE_TRIGGER = 3008;
    static constexpr int AREA_FADE_TRIGGER = 3009;
    static constexpr int AREA_TINT_TRIGGER = 3010;
    static constexpr int EDIT_AREA_MOVE_TRIGGER = 3011;
    static constexpr int EDIT_AREA_ROTATE_TRIGGER = 3012;
    static constexpr int EDIT_AREA_SCALE_TRIGGER = 3013;
    static constexpr int EDIT_AREA_FADE_TRIGGER = 3014;
    static constexpr int EDIT_AREA_TINT_TRIGGER = 3015;
    static constexpr int CHANGE_BG_TRIGGER = 3029;
    static constexpr int CHANGE_GROUND_TRIGGER = 3030;
    static constexpr int CHANGE_MIDGROUND_TRIGGER = 3031;
    static constexpr int TOUCH_TRIGGER = 1595;
    static constexpr int COUNT_TRIGGER = 1611;
    static constexpr int INSTANT_COUNT_TRIGGER = 1811;
    static constexpr int PICKUP_TRIGGER = 1817;
    static constexpr int RANDOM_TRIGGER = 1912;
    static constexpr int ADVANCED_RANDOM_TRIGGER = 2068;
    static constexpr int SEQUENCE_TRIGGER = 3607;
    static constexpr int SPAWN_PARTICLE_TRIGGER = 3608;
    static constexpr int RESET_TRIGGER = 3618;
    static constexpr int CAMERA_ZOOM_TRIGGER = 1913;
    static constexpr int CAMERA_STATIC_TRIGGER = 1914;
    static constexpr int CAMERA_OFFSET_TRIGGER = 1916;
    static constexpr int CAMERA_GP_OFFSET_TRIGGER = 2901;
    static constexpr int CAMERA_ROTATE_TRIGGER = 2015;
    static constexpr int CAMERA_EDGE_TRIGGER = 2062;
    static constexpr int CAMERA_MODE_TRIGGER = 2925;
    static constexpr int COLLISION_TRIGGER = 1815;
    static constexpr int TIME_WARP_TRIGGER = 1935;
    static constexpr int MIDGROUND_SETUP_TRIGGER = 2999;
    static constexpr int BG_SPEED_TRIGGER = 3606;
    static constexpr int MIDGROUND_SPEED_TRIGGER = 3612;
    static constexpr int INSTANT_COLLISION_TRIGGER = 3609;
    static constexpr int ON_DEATH_TRIGGER = 1812;
    static constexpr int PLAYER_CONTROL_TRIGGER = 1932;

    static constexpr bool isTriggerID(int id) {
        switch (id) {
            case MOVE_TRIGGER: return true;
            case STOP_TRIGGER: return true;
            case PULSE_TRIGGER: return true;
            case ALPHA_TRIGGER: return true;
            case TOGGLE_TRIGGER: return true;
            case SPAWN_TRIGGER: return true;
            case ROTATE_TRIGGER: return true;
            case SCALE_TRIGGER: return true;
            case FOLLOW_TRIGGER: return true;
            case ANIMATE_TRIGGER: return true;
            case FOLLOW_PLAYER_Y_TRIGGER: return true;
            case ADVANCED_FOLLOW_TRIGGER: return true;
            case EDIT_ADVANCED_FOLLOW_TRIGGER: return true;
            case RE_TARGET_ADVANCED_FOLLOW_TRIGGER: return true;
            case AREA_MOVE_TRIGGER: return true;
            case AREA_ROTATE_TRIGGER: return true;
            case AREA_SCALE_TRIGGER: return true;
            case AREA_FADE_TRIGGER: return true;
            case AREA_TINT_TRIGGER: return true;
            case EDIT_AREA_MOVE_TRIGGER: return true;
            case EDIT_AREA_ROTATE_TRIGGER: return true;
            case EDIT_AREA_SCALE_TRIGGER: return true;
            case EDIT_AREA_FADE_TRIGGER: return true;
            case EDIT_AREA_TINT_TRIGGER: return true;
            case CHANGE_BG_TRIGGER: return true;
            case CHANGE_GROUND_TRIGGER: return true;
            case CHANGE_MIDGROUND_TRIGGER: return true;
            case TOUCH_TRIGGER: return true;
            case COUNT_TRIGGER: return true;
            case INSTANT_COUNT_TRIGGER: return true;
            case PICKUP_TRIGGER: return true;
            case RANDOM_TRIGGER: return true;
            case ADVANCED_RANDOM_TRIGGER: return true;
            case SEQUENCE_TRIGGER: return true;
            case SPAWN_PARTICLE_TRIGGER: return true;
            case RESET_TRIGGER: return true;
            case CAMERA_ZOOM_TRIGGER: return true;
            case CAMERA_STATIC_TRIGGER: return true;
            case CAMERA_OFFSET_TRIGGER: return true;
            case CAMERA_GP_OFFSET_TRIGGER: return true;
            case CAMERA_ROTATE_TRIGGER: return true;
            case CAMERA_EDGE_TRIGGER: return true;
            case CAMERA_MODE_TRIGGER: return true;
            case COLLISION_TRIGGER: return true;
            case TIME_WARP_TRIGGER: return true;
            case MIDGROUND_SETUP_TRIGGER: return true;
            case BG_SPEED_TRIGGER: return true;
            case MIDGROUND_SPEED_TRIGGER: return true;
            case INSTANT_COLLISION_TRIGGER: return true;
            case ON_DEATH_TRIGGER: return true;
            case PLAYER_CONTROL_TRIGGER: return true;
            default: return false;
        }
    }
}
