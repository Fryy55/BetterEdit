#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/file.hpp>
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

    template <size_t Magic>
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
            auto signature = ssl::decodeBase64<Magic + 20>(parts[4]).value_or(ByteVector());

            return ssl::verify<Magic>(
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

#define HAS_PRO() (pro::verifyPro<__LINE__>() == __LINE__ + pro::ssl::MAGIC_OK)
