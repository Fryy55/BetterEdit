#include "Pro.hpp"
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>
#include <matjson/stl_serialize.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

// https://discord.com/channels/1210283926145466448/1210283926640267266/1262130273382961204

// static float* CACHED_ACCOUNT_ID = 0;
// class $modify(MenuLayer) {
//     bool init() {
//         if (!MenuLayer::init())
//             return false;

//         if (CACHED_ACCOUNT_ID == nullptr) {
//             this->onMyProfile(nullptr);
//             if (auto page = getChildOfType<ProfilePage>(CCScene::get(), -1)) {
//                 CACHED_ACCOUNT_ID = reinterpret_cast<float*>(static_cast<intptr_t>(page->m_accountID));
//                 page->removeFromParent();
//             }
//         }

//         return true;
//     }
// };

ccColor3B pro::getSupporterColor(int supportedAmount) {
    if (supportedAmount >= 5000) {
        return ccc3(227, 85, 255);
    }
    if (supportedAmount >= 1000) {
        return ccc3(255, 187, 0);
    }
    if (supportedAmount >= 500) {
        return ccc3(0, 238, 255);
    }
    if (supportedAmount >= 200) {
        return ccc3(30, 255, 0);
    }
    return ccWHITE;
}

void pro::showProOnlyFeaturePopup(std::string_view featureName) {
    FLAlertLayer::create(
        "Supporter-Only Feature",
        fmt::format(
            "This feature ({}) is in early access for <cp>Supporters</c>, but "
            "will be available for everyone in the next major update!",
            featureName
        ),
        "OK"
    )->show();
}
