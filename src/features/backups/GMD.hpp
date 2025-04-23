#pragma once

#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/GJLevelList.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/JsonValidation.hpp>

namespace gmd {
    enum class GmdFileType {
        /**
         * Lvl contains the level data as a Plist string with GZip-compression
         * applied. A fully obsolete format, supported for basically no reason
         * other than that it can be
         */
        Lvl,
        /**
         * Gmd contains the level data as a basic Plist string
         */
        Gmd,
        /**
         * Gmd2 is a Zip file that contains the level data in Gmd format
         * under level.data, plus metadata under level.meta. May also include
         * the level's song file in the package
         * @note Old GDShare implementations supported compression schemes in
         * Gmd2 - those are not supported in GMD-API due to being completely
         * redundant
         */
        Gmd2,
    };
    enum class GmdListFileType {
        /**
         * Gmdl contains the list data as a basic Plist string
         */
        Gmdl,
    };

    constexpr auto DEFAULT_GMD_TYPE = GmdFileType::Gmd;
    constexpr auto DEFAULT_GMD_LIST_TYPE = GmdListFileType::Gmdl;
    constexpr auto GMD2_VERSION = 1;

    constexpr const char* gmdTypeToString(GmdFileType type) {
        switch (type) {
            case GmdFileType::Lvl:  return "lvl";
            case GmdFileType::Gmd:  return "gmd";
            case GmdFileType::Gmd2: return "gmd2";
            default:                return nullptr;
        }
    }
    constexpr std::optional<GmdFileType> gmdTypeFromString(const char* type) {
        using geode::utils::hash;
        switch (hash(type)) {
            case hash("lvl"):  return GmdFileType::Lvl;
            case hash("gmd"):  return GmdFileType::Gmd;
            case hash("gmd2"): return GmdFileType::Gmd2;
            default:           return std::nullopt;
        }
    }

    constexpr const char* gmdListTypeToString(GmdListFileType type) {
        switch (type) {
            case GmdListFileType::Gmdl: return "gmdl";
            default:                    return nullptr;
        }
    }
    constexpr std::optional<GmdListFileType> gmdListTypeFromString(const char* type) {
        using geode::utils::hash;
        switch (hash(type)) {
            case hash("gmdl"): return GmdListFileType::Gmdl;
            default:           return std::nullopt;
        }
    }

    enum class GmdFileKind {
        None,
        Level,
        List,
    };
    inline GmdFileKind getGmdFileKind(std::filesystem::path const& path) {
        auto ext = path.extension().string();
        if (ext.size()) {
            ext = ext.substr(1);
        }
        if (gmdListTypeFromString(ext.c_str())) {
            return GmdFileKind::List;
        }
        if (gmdTypeFromString(ext.c_str())) {
            return GmdFileKind::Level;
        }
        return GmdFileKind::None;
    }

    template<class T>
    class IGmdFile {
    protected:
        std::optional<GmdFileType> m_type;

    public:
        T& setType(GmdFileType type) {
            m_type = type;
            return *static_cast<T*>(this);
        }
    };

    /**
     * Class for working with importing levels as GMD files
     */
    class ImportGmdFile : public IGmdFile<ImportGmdFile> {
    protected:
        std::filesystem::path m_path;
        bool m_importSong = false;

        ImportGmdFile(std::filesystem::path const& path) : m_path(path) {}

        geode::Result<std::string> getLevelData() const {
            if (!m_type) {
                return geode::Err(
                    "No file type set; either it couldn't have been inferred from the "
                    "file or the developer of the mod forgot to call inferType"
                );
            }
            switch (m_type.value()) {
                case GmdFileType::Gmd: {
                    return geode::utils::file::readString(m_path)
                        .mapErr([this](std::string err) { return fmt::format("Unable to read {}: {}", m_path, err); });
                } break;

                case GmdFileType::Lvl: {
                    GEODE_UNWRAP_INTO(auto data, geode::utils::file::readBinary(m_path));
                    unsigned char* unzippedData;
                    auto count = cocos2d::ZipUtils::ccInflateMemory(
                        data.data(),
                        data.size(),
                        &unzippedData
                    );
                    if (!count) {
                        return geode::Err("Unable to decompress level data");
                    }
                    auto str = std::string(
                        reinterpret_cast<const char*>(unzippedData),
                        count
                    );
                    free(unzippedData);
                    return geode::Ok(str);
                } break;

                case GmdFileType::Gmd2: {
                    try {
                        GEODE_UNWRAP_INTO(
                            auto unzip, geode::utils::file::Unzip::create(m_path)
                                .mapErr([](std::string err) { return fmt::format("Unable to read file: {}", err); })
                        );

                        // read metadata
                        GEODE_UNWRAP_INTO(
                            auto jsonData, unzip.extract("level.meta")
                                .mapErr([](std::string err) { return fmt::format("Unable to read metadata: {}", err); })
                        );

                        GEODE_UNWRAP_INTO(
                            auto json, matjson::parse(std::string(jsonData.begin(), jsonData.end()))
                                .mapErr([](std::string err) { return fmt::format("Unable to parse metadata: {}", err); })
                        );

                        geode::JsonExpectedValue root(json, "[level.meta]");

                        // unzip song
                        std::string songFile;
                        root.has("song-file").into(songFile);
                        if (m_importSong && songFile.size()) {
                            // make sure the song file name is legit. without this check
                            // it's possible to do arbitary code execution with gmd2
                            if (!songFile.ends_with(".mp3")) {
                                return geode::Err("Song file name '{}' is invalid!", songFile);
                            }

                            GEODE_UNWRAP_INTO(
                                auto songNumber, geode::utils::numFromString<int>(songFile.substr(0, songFile.find_first_of(".")))
                                    .mapErr([songFile](std::string err) { return fmt::format("Song file name '{}' is invalid!", songFile); })
                            );

                            GEODE_UNWRAP_INTO(
                                auto songData, unzip.extract(songFile)
                                    .mapErr([](std::string err) { return fmt::format("Unable to read song file: {}", err); })
                            );

                            std::filesystem::path songTargetPath;
                            if (root.has("song-is-custom").get<bool>()) {
                                songTargetPath = std::string(MusicDownloadManager::sharedState()->pathForSong(songNumber));
                            } else {
                                songTargetPath = "Resources/" + songFile;
                            }

                            // if we're replacing a file, figure out a different name
                            // for the old one
                            std::filesystem::path oldSongPath = songTargetPath;
                            while (std::filesystem::exists(oldSongPath)) {
                                oldSongPath.replace_filename(oldSongPath.stem().string() + "_.mp3");
                            }
                            if (std::filesystem::exists(oldSongPath)) {
                                std::filesystem::rename(songTargetPath, oldSongPath);
                            }
                            (void)geode::utils::file::writeBinary(songTargetPath, songData);
                        }

                        GEODE_UNWRAP_INTO(
                            auto levelData, unzip.extract("level.data")
                                .mapErr([](std::string err) { return fmt::format("Unable to read level data: {}", err); })
                        );

                        return geode::Ok(std::string(levelData.begin(), levelData.end()));
                    } catch(std::exception& e) {
                        return geode::Err("Unable to read zip: " + std::string(e.what()));
                    }
                } break;

                default: {
                    return geode::Err("Unknown file type");
                } break;
            }
        }

    public:
        /**
         * Create an ImportGmdFile instance from a file
         * @param path The file to import
         */
        static ImportGmdFile from(std::filesystem::path const& path) {
            return ImportGmdFile(path);
        }
        /**
         * Try to infer the file type from the file's path
         * @returns True if the type was inferred, false if not
         */
        bool tryInferType() {
            auto extension = m_path.extension().string();
            if (extension.size()) {
                extension = extension.substr(1);
            }
            if (auto ext = gmdTypeFromString(extension.c_str())) {
                m_type = ext.value();
                return true;
            }
            return false;
        }
        /**
         * Try to infer the file type from the file's extension. If the
         * extension is unknown, the type is inferred as DEFAULT_GMD_TYPE
        */
        ImportGmdFile& inferType() {
            auto extension = m_path.extension().string();
            if (extension.size()) {
                extension = extension.substr(1);
            }
            if (auto ext = gmdTypeFromString(extension.c_str())) {
                m_type = ext.value();
            } else {
                m_type = DEFAULT_GMD_TYPE;
            }
            return *this;
        }
        /**
         * Set whether to import the song file included in this file or not
         */
        ImportGmdFile& setImportSong(bool song) {
            m_importSong = song;
            return *this;
        }
        /**
         * Load the file and parse it into a GJGameLevel
         * @returns An Ok Result with the parsed level, or an Err with info
         * @note Does not add the level to the user's local created levels -
         * the GJGameLevel will not be retained by anything!
         */
        geode::Result<GJGameLevel*> intoLevel() const {
            auto data = getLevelData();

            GEODE_UNWRAP_INTO(auto value, data);

            for (auto& c : value) {
                if (!c) c = ' ';
            }

            // add gjver if it's missing as otherwise DS_Dictionary fails to load the data
            auto pos = value.substr(0, 100).find("<plist version=\"1.0\">");
            if (pos != std::string::npos) {
                value.replace(pos, 21, "<plist version=\"1.0\" gjver=\"2.0\">");
            }
            bool isOldFile = false;
            if (!value.starts_with("<?xml version")) {
                if (value.substr(0, 100).find("<plist version") == std::string::npos) {
                    isOldFile = true;
                    value = "<?xml version=\"1.0\"?><plist version=\"1.0\" gjver=\"2.0\"><dict><k>root</k>" +
                        value + "</dict></plist>";
                }
                else {
                    value = "<?xml version=\"1.0\"?>" + value;
                }
            }

            auto dict = std::make_unique<DS_Dictionary>();
            if (!dict.get()->loadRootSubDictFromString(value)) {
                return geode::Err("Unable to parse level data");
            }
            dict->stepIntoSubDictWithKey("root");

            auto level = GJGameLevel::create();
            level->dataLoaded(dict.get());

            level->m_isEditable = true;
            level->m_levelType = GJLevelType::Editor;

        #ifdef GEODE_IS_WINDOWS
            // old gdshare double base64 encoded the description,
            // so we decode it again
            if (isOldFile && level->m_levelDesc.size()) {
                unsigned char* out = nullptr;
                // we really should add some base64 utils, this is nasty
                auto size = cocos2d::base64Decode(
                    reinterpret_cast<unsigned char*>(const_cast<char*>(level->m_levelDesc.c_str())),
                    level->m_levelDesc.size(),
                    &out
                );
                if (out) {
                    auto newDesc = std::string(reinterpret_cast<char*>(out), size);
                    free(out);
                    level->m_levelDesc = newDesc;
                }
            }
        #endif

            return geode::Ok(level);
        }
    };

    /**
     * Class for working with exporting levels as GMD files
     */
    class ExportGmdFile : public IGmdFile<ExportGmdFile> {
    protected:
        GJGameLevel* m_level;
        bool m_includeSong = false;

        ExportGmdFile(GJGameLevel* level) : m_level(level) {}

        geode::Result<std::string> getLevelData() const {
            if (!m_level) {
                return geode::Err("No level set");
            }
            auto dict = new DS_Dictionary();
            m_level->encodeWithCoder(dict);
            auto data = dict->saveRootSubDictToString();
            delete dict;
            return geode::Ok(std::string(data));
        }

    public:
        /**
         * Create an ExportGmdFile instance for a level
         * @param path The level to export
         */
        static ExportGmdFile from(GJGameLevel* level) {
            return ExportGmdFile(level);
        }
        /**
         * Set whether to include the song file with the exported file or not
         * @note Currently only supported in GMD2 files
         */
        ExportGmdFile& setIncludeSong(bool song) {
            m_includeSong = song;
            return *this;
        }
        /**
         * Export the level into an in-stream byte array
         * @returns Ok Result with the byte data if succesful, Err otherwise
         */
        geode::Result<geode::ByteVector> intoBytes() const {
            if (!m_type) {
                return geode::Err(
                    "No file type set; seems like the developer of the mod "
                    "forgot to set it"
                );
            }
            switch (m_type.value()) {
                case GmdFileType::Gmd: {
                    GEODE_UNWRAP_INTO(auto data, this->getLevelData());
                    return geode::Ok(geode::ByteVector(data.begin(), data.end()));
                } break;

                case GmdFileType::Lvl: {
                    GEODE_UNWRAP_INTO(auto data, this->getLevelData());
                    unsigned char* zippedData = nullptr;
                    auto count = cocos2d::ZipUtils::ccDeflateMemory(
                        reinterpret_cast<unsigned char*>(data.data()),
                        data.size(),
                        &zippedData
                    );
                    if (!count) {
                        return geode::Err("Unable to compress level data");
                    }
                    auto bytes = geode::ByteVector(
                        reinterpret_cast<uint8_t*>(zippedData),
                        reinterpret_cast<uint8_t*>(zippedData + count)
                    );
                    if (zippedData) {
                        delete[] zippedData;
                    }
                    return geode::Ok(bytes);
                } break;

                case GmdFileType::Gmd2: {
                    GEODE_UNWRAP_INTO(auto data, this->getLevelData());
                    GEODE_UNWRAP_INTO(auto zip, geode::utils::file::Zip::create());

                    auto json = matjson::Value();
                    if (m_includeSong) {
                        auto path = std::filesystem::path(std::string(m_level->getAudioFileName()));
                        json["song-file"] = path.filename().string();
                        json["song-is-custom"] = m_level->m_songID;
                        GEODE_UNWRAP(zip.addFrom(path));
                    }
                    GEODE_UNWRAP(zip.add("level.meta", json.dump()));
                    GEODE_UNWRAP(zip.add("level.data", data));

                    return geode::Ok(zip.getData());
                } break;

                default: {
                    return geode::Err("Unknown file type");
                } break;
            }
        }
        /**
         * Export the level into a file
         * @param path The file to export into. Will be created if it doesn't
         * exist yet
         * @returns Ok Result if exporting succeeded, Err otherwise
         */
        geode::Result<> intoFile(std::filesystem::path const& path) const {
            GEODE_UNWRAP_INTO(auto data, this->intoBytes());
            GEODE_UNWRAP(geode::utils::file::writeBinary(path, data));
            return geode::Ok();
        }
    };

    /**
     * Export a level as a GMD file. For more control over the exporting
     * options, use the ExportGmdFile class
     * @param level The level you want to export
     * @param to The path of the file to export to
     * @param type The type to export the level as
     * @returns Ok Result on success, Err on error
     */
    inline geode::Result<> exportLevelAsGmd(
        GJGameLevel* level,
        std::filesystem::path const& to,
        GmdFileType type = DEFAULT_GMD_TYPE
    ) {
        return ExportGmdFile::from(level).setType(type).intoFile(to);
    }

    /**
     * Import a level from a GMD file. For more control over the importing
     * options, use the ImportGmdFile class
     * @param from The path of the file to import. The path's extension is used
     * to infer the type of the file to import - if the extension is unknown,
     * DEFAULT_GMD_TYPE is assumed
     * @note The level is **not** added to the local created levels list
     */
    inline geode::Result<GJGameLevel*> importGmdAsLevel(
        std::filesystem::path const& from
    ) {
        return ImportGmdFile::from(from).inferType().intoLevel();
    }

    class ImportGmdList final {
    private:
        std::filesystem::path m_path;
        GmdListFileType m_type = DEFAULT_GMD_LIST_TYPE;

        ImportGmdList(std::filesystem::path const& path) : m_path(path) {}

    public:
        static ImportGmdList from(std::filesystem::path const& path) {
            return ImportGmdList(path);
        }

        ImportGmdList& setType(GmdListFileType type) {
            m_type = type;
            return *this;
        }

        geode::Result<geode::Ref<GJLevelList>> intoList() {
            GEODE_UNWRAP_INTO(auto data, geode::utils::file::readString(m_path)
                .mapErr([this](std::string err) { return fmt::format("Unable to read {}: {}", m_path, err); })
            );

            for (auto& c : data) {
                if (!c) c = ' ';
            }

            // add gjver if it's missing as otherwise DS_Dictionary fails to load the data
            auto pos = data.substr(0, 100).find("<plist version=\"1.0\">");
            if (pos != std::string::npos) {
                data.replace(pos, 21, "<plist version=\"1.0\" gjver=\"2.0\">");
            }
            if (!data.starts_with("<?xml version")) {
                if (data.substr(0, 100).find("<plist version") == std::string::npos) {
                    data = "<?xml version=\"1.0\"?><plist version=\"1.0\" gjver=\"2.0\"><dict><k>root</k>" +
                        data + "</dict></plist>";
                }
                else {
                    data = "<?xml version=\"1.0\"?>" + data;
                }
            }

            auto dict = std::make_unique<DS_Dictionary>();
            if (!dict.get()->loadRootSubDictFromString(data)) {
                return geode::Err("Unable to parse list data");
            }
            dict->stepIntoSubDictWithKey("root");

            auto list = GJLevelList::create();
            list->dataLoaded(dict.get());

            list->m_listType = GJLevelType::Editor;
            list->m_isEditable = true;

            return geode::Ok(list);
        }

    };

    class ExportGmdList final {
    private:
        GmdListFileType m_type = DEFAULT_GMD_LIST_TYPE;
        geode::Ref<GJLevelList> m_list;

        ExportGmdList(GJLevelList* list) : m_list(list) {}

    public:
        static ExportGmdList from(GJLevelList* list) {
            return ExportGmdList(list);
        }

        ExportGmdList& setType(GmdListFileType type) {
            m_type = type;
            return *this;
        }

        /**
         * Export the list into an in-stream byte array
         * @returns Ok Result with the byte data if succesful, Err otherwise
         */
        geode::Result<geode::ByteVector> intoBytes() const {
            auto dict = std::make_unique<DS_Dictionary>();
            m_list->encodeWithCoder(dict.get());
            auto data = std::string(dict->saveRootSubDictToString());
            return geode::Ok(geode::ByteVector(data.begin(), data.end()));
        }
        /**
         * Export the list into a file
         * @param path The file to export into. Will be created if it doesn't
         * exist yet
         * @returns Ok Result if exporting succeeded, Err otherwise
         */
        geode::Result<> intoFile(std::filesystem::path const& path) const {
            GEODE_UNWRAP_INTO(auto data, this->intoBytes());
            GEODE_UNWRAP(geode::utils::file::writeBinary(path, data));
            return geode::Ok();
        }
    };

    /**
     * Export a list as a GMD file. For more control over the exporting
     * options, use the ExportGmdFile class
     * @param list The list you want to export
     * @param to The path of the file to export to
     * @param type The type to export the level as
     * @returns Ok Result on success, Err on error
     */
    inline geode::Result<> exportListAsGmd(
        GJLevelList* list,
        std::filesystem::path const& to,
        GmdListFileType type = DEFAULT_GMD_LIST_TYPE
    ) {
        return ExportGmdList::from(list).setType(type).intoFile(to);
    }

    /**
     * Import a list from a GMD file. For more control over the importing
     * options, use the ImportGmdFile class
     * @param from The path of the file to import. The path's extension is used
     * to infer the type of the file to import - if the extension is unknown,
     * DEFAULT_GMD_LIST_TYPE is assumed
     * @note The list is **not** added to the local created lists list
     */
    inline geode::Result<geode::Ref<GJLevelList>> importGmdAsList(
        std::filesystem::path const& from
    ) {
        return ImportGmdList::from(from).setType(DEFAULT_GMD_LIST_TYPE).intoList();
    }
}
