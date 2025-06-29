#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/utils/Task.hpp>
#include <optional>
#include <chrono>

using namespace geode::prelude;

namespace pro::server {
    constexpr std::string_view BASE_URL = "https://api.betteredit.pro";

    struct CachedGDInfo final {
        int gdAccountID;
        std::string username;
        int cubeID;
        int playerColor1;
        int playerColor2;
        std::optional<int> glowColor;

        static Result<CachedGDInfo> parse(matjson::Value const& json);
        void update(SimplePlayer* player);
    };

    struct Supporter final {
        CachedGDInfo info;
        int supportedAmount;

        static Result<Supporter> parse(matjson::Value const& json);
    };
    struct Supporters final {
        std::vector<Supporter> supporters;
        size_t totalSupporterCount;
        size_t totalPublicSupporterCount;

        static Result<Supporters> parse(matjson::Value const& json);
    };

    struct MyDevice final {
        std::string deviceID;
        std::string deviceName;
        PlatformID devicePlatform = PlatformID::Windows;
        std::chrono::time_point<std::chrono::system_clock> activatedOn;

        static Result<MyDevice> parse(matjson::Value const& json);
    };
    struct MySupport final {
        bool showingPublicly;
        std::chrono::time_point<std::chrono::system_clock> supportedOn;
        int supportedAmount;
        CachedGDInfo gdInfo;
        std::vector<MyDevice> devices;
        
        static Result<MySupport> parse(matjson::Value const& json);
    };

    struct UpdateSupporter final {
        bool showPublicly;
    };
    struct UpdateDeviceInfo final {
        std::string deviceName;
    };
    struct UpdatedDeviceInfo final {
        std::string deviceName;

        static Result<UpdatedDeviceInfo> parse(matjson::Value const& json);
    };

    struct ActivatedLicense final {
        std::string token;
        
        static Result<ActivatedLicense> parse(matjson::Value const& json);
    };

    struct CreatedProductKey final {
        std::string key;
        
        static Result<CreatedProductKey> parse(matjson::Value const& json);
    };

    constexpr size_t SUPPORTERS_PER_PAGE = 10;
    constexpr size_t MAX_DEVICE_COUNT = 5;

    template <class T>
    using ServerRequest = Task<Result<T>, uint8_t>;

    std::string getUserAgent();

    ServerRequest<Supporters> getSupporters(size_t page, bool useCache = true);
    ServerRequest<MySupport> getMySupport(std::string const& token, bool useCache = true);
    ServerRequest<ActivatedLicense> activateLicense(std::string const& key);
    ServerRequest<std::monostate> deactivateLicense(std::string const& token, std::string const& deviceID);
    ServerRequest<std::string> checkLicense(std::string const& token);
    ServerRequest<CreatedProductKey> createNewLicense(std::string const& token);
    ServerRequest<std::monostate> updateCachedInfo(std::string const& token, CachedGDInfo const& info);
    ServerRequest<std::monostate> updateSupporter(std::string const& token, UpdateSupporter const& info);
    ServerRequest<UpdatedDeviceInfo> updateDeviceInfo(std::string const& token, std::string const& deviceID, UpdateDeviceInfo const& info);

    void clearCaches();
}
