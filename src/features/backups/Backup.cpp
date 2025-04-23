#include "Backup.hpp"
#include <Geode/loader/Dirs.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include "GMD.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <matjson/stl_serialize.hpp>
#include <fmt/chrono.h>

// todo: list backups associated with deleted / lost levels

std::filesystem::path getBackupsDir(GJGameLevel* level) {
    return dirs::getSaveDir() / "betteredit-level-backups" / std::to_string(EditorIDs::getID(level));
}

struct BackupMetadata final {
	typename Backup::TimePoint createTime = Backup::Clock::now();
    bool automated = false;
};

template <>
struct matjson::Serialize<BackupMetadata> {
    static matjson::Value toJson(BackupMetadata const& meta) {
		return matjson::makeObject({
			{ "create-time", std::chrono::duration_cast<Backup::TimeUnit>(meta.createTime.time_since_epoch()).count() },
            { "automated", meta.automated },
		});
	}
    static Result<BackupMetadata> fromJson(matjson::Value const& value) {
		auto meta = BackupMetadata();
		auto obj = checkJson(value, "BackupMetadata");
        int createTime;
        obj.needs("create-time").into(createTime);
        meta.createTime = Backup::TimePoint(Backup::TimeUnit(createTime));
        obj.has("automated").into(meta.automated);
		return obj.ok(meta);
	}
};

GJGameLevel* Backup::getLevel() const {
    return m_level;
}
GJGameLevel* Backup::getOriginalLevel() const {
    return m_forLevel;
}
typename Backup::TimePoint Backup::getCreateTime() const {
    return m_createTime;
}
bool Backup::isAutomated() const {
    return m_automated;
}

Result<> Backup::restoreThis() {
    // Add changes to memory
    // They should be saved on game close
    m_forLevel->m_levelString = m_level->m_levelString;
    m_forLevel->m_levelName = m_level->m_levelName;
    m_forLevel->m_levelDesc = m_level->m_levelDesc;
    m_forLevel->m_levelLength = m_level->m_levelLength;
    m_forLevel->m_objectCount = m_level->m_objectCount;
    m_forLevel->m_audioTrack = m_level->m_audioTrack;
    m_forLevel->m_songID = m_level->m_songID;
    m_forLevel->m_songIDs = m_level->m_songIDs;
    m_forLevel->m_sfxIDs = m_level->m_sfxIDs;
    m_forLevel->m_twoPlayerMode = m_level->m_twoPlayerMode;
    return Ok();
}
Result<> Backup::deleteThis() {
    std::error_code ec;
    std::filesystem::remove_all(m_directory, ec);
    if (ec) {
        return Err("Unable to delete backup directory: {} (code {})", ec.message(), ec.value());
    }
    return Ok();
}
Result<> Backup::preserveAutomated() {
    m_automated = false;
    auto metadata = BackupMetadata {
        .createTime = m_createTime,
        .automated = false,
    };
    GEODE_UNWRAP(file::writeToJson(m_directory / "meta.json", metadata).mapErr([](auto error) {
        return fmt::format("Unable to save metadata: {}", error);
    }));
    return Ok();
}

Result<std::shared_ptr<Backup>> Backup::load(std::filesystem::path const& dir, GJGameLevel* forLevel) {
    GEODE_UNWRAP_INTO(auto level, gmd::importGmdAsLevel(dir / "level.gmd").mapErr([](auto error) {
        return fmt::format("Unable to read level file: {}", error);
    }));
    auto meta = file::readFromJson<BackupMetadata>(dir / "meta.json").unwrapOrDefault();

    auto backup = std::make_shared<Backup>();
    backup->m_level = level;
    backup->m_forLevel = forLevel;
    backup->m_directory = dir;
    backup->m_createTime = meta.createTime;
    backup->m_automated = meta.automated;
    return Ok(backup);
}
std::vector<std::shared_ptr<Backup>> Backup::load(GJGameLevel* level) {
    std::vector<std::shared_ptr<Backup>> res;
    for (auto folder : file::readDirectory(getBackupsDir(level)).unwrapOrDefault()) {
        if (!std::filesystem::is_directory(folder)) {
            continue;
        }
        auto b = Backup::load(folder, level);
        if (!b) {
            log::error("Unable to load backup at {}: {}", folder, b.unwrapErr());
            continue;
        }
        res.push_back(*b);
    }

    // This takes advantage of the fact that directories are sorted 
    // alphabetically and the date format is oldest-newest when sorted that way 
    // Might want to consider a proper sort-by-newest-first
    std::reverse(res.begin(), res.end());

    return res;
}
Result<> Backup::create(GJGameLevel* level, bool automated) {
    if (level->m_levelType != GJLevelType::Editor) {
        return Err("Can not backup a non-editor level");
    }
    auto time = Clock::now();

    auto dir = getBackupsDir(level) / fmt::format("{:%Y-%m-%d_%H-%M}", time);
    if (std::filesystem::exists(dir)) {
        return Err("Level was already backed up less than a minute ago");
    }

    GEODE_UNWRAP(file::createDirectoryAll(dir).mapErr([](auto error) {
        return fmt::format("Failed to create directory: {}", error);
    }));

    GEODE_UNWRAP(gmd::exportLevelAsGmd(level, dir / "level.gmd").mapErr([](auto error) {
        return fmt::format("Unable to save level: {}", error);
    }));

    auto metadata = BackupMetadata {
        .createTime = time,
        .automated = automated,
    };
    GEODE_UNWRAP(file::writeToJson(dir / "meta.json", metadata).mapErr([](auto error) {
        return fmt::format("Unable to save metadata: {}", error);
    }));

    return Ok();
}
Result<> Backup::cleanAutomated(GJGameLevel* level) {
    std::vector<std::shared_ptr<Backup>> automated;
    for (auto folder : file::readDirectory(getBackupsDir(level)).unwrapOrDefault()) {
        if (!std::filesystem::is_directory(folder)) {
            continue;
        }
        auto b = Backup::load(folder, level);
        if (!b) {
            continue;
        }
        auto backup = *b;
        if (backup->m_automated) {
            automated.push_back(backup);
        }
    }

    // This takes advantage of the fact that directories are sorted 
    // alphabetically and the date format is oldest-newest when sorted that way 
    // Might want to consider a proper sort-by-newest-first
    std::reverse(automated.begin(), automated.end());

    constexpr size_t MAX_AUTOMATED_COUNT = 3;

    // Do the cleanup if needed
    if (automated.size() > MAX_AUTOMATED_COUNT) {
        size_t ix = 0;
        for (auto backup : automated) {
            // Keep only the MAX_AUTOMATED_COUNT newest automated backups
            if (++ix > MAX_AUTOMATED_COUNT) {
                GEODE_UNWRAP(backup->deleteThis());
            }
        }
        automated.erase(automated.begin() + MAX_AUTOMATED_COUNT, automated.end());
    }

    return Ok();
}
