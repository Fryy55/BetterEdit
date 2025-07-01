#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include "server/Server.hpp"
#include "server/OpenSSL.hpp"

using namespace geode::prelude;

class AboutBEPopup;

namespace pro {
    inline std::filesystem::path getKeyPath() {
        return dirs::getSaveDir() / ".betteredit-key-please-do-not-delete";
    }

    inline Result<> saveProKey(std::string const& key) {
        return file::writeString(getKeyPath(), key);
    }
    inline std::optional<std::string> getProKey(bool update = false) {
        static std::string cache = std::string();
        if (update || cache.empty()) {
            cache = file::readString(getKeyPath()).unwrapOrDefault();
            Mod::get()->setSavedValue("has-pro", !cache.empty());
        }
        return cache.empty() ? std::nullopt : std::optional(cache);
    }
    inline std::optional<std::string> getDeviceSalt() {
        auto key = getProKey();
        if (!key) return std::nullopt;
        auto parts = string::split(*key, ":");
        if (parts.size() != 5) {
            return std::nullopt;
        }
        return parts[3];
    }

    inline size_t verifyPro() {
        if (auto key = getProKey()) {
            auto parts = string::split(*key, ":");
            if (parts.size() != 5) {
                return 0;
            }
            auto gdAccountID = GJAccountManager::get()->m_accountID;
            auto supporterID = parts[1];
            auto platform = GEODE_PLATFORM_SHORT_IDENTIFIER;
            auto salt = parts[3];
            auto signature = base64::decode(parts[4], base64::Base64Variant::Normal).unwrapOrDefault();

            return ssl::verify(
                fmt::format("{}:{}:{}:{}", gdAccountID, supporterID, platform, salt),
                signature
            );
        }
        return 0;
    }

    // Interfaces for free

    void addAboutPopupStuff(AboutBEPopup* popup);
    
    ccColor3B getSupporterColor(int supportedAmount);
    
    void showProOnlyFeaturePopup(std::string_view featureName);
}

#define HAS_PRO() (pro::verifyPro())
