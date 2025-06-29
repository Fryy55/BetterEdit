#pragma once

#include <filesystem>
#include <Geode/binding/GameObject.hpp>
#include <Geode/utils/VersionInfo.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Event.hpp>
#include "QJS.hpp"

using namespace geode::prelude;

class JsScript final : public std::enable_shared_from_this<JsScript> {
public:
    struct Log final {
        enum class Level {
            Status,
            Info,
            Warning,
            Error,
        };
        Level level;
        std::string message;
    };

private:
    std::filesystem::path m_path;
    std::string m_data;
    std::string m_title;
    std::string m_author;
    VersionInfo m_version;
    std::vector<Log> m_lastRunLogs;
    bool m_queuedLogEvent = false;
    bool m_runnable = true;
    bool m_finished = true;
    qjs::Runtime m_runtime = qjs::Runtime::null();
    qjs::Context m_ctx = qjs::Context::null();
    qjs::Module m_module = qjs::Module::null();

    void log(Log::Level level, std::string_view message);

    template <class... Args>
    void log(Log::Level level, fmt::format_string<Args...> fmt, Args&&... args) {
        this->log(level, fmt::format(fmt, std::forward<Args>(args)...));
    }

public:
    static std::shared_ptr<JsScript> create(std::filesystem::path const& path);

    std::filesystem::path getPath() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    VersionInfo getVersion() const;
    std::vector<Log> getLastRunLogs() const;
    Log::Level getLastRunSeverity() const;
    bool canRun() const;

    bool run();
    bool tick();
};

class JsScriptLoggedEvent : public Event {
public:
    std::shared_ptr<JsScript> script;
    JsScriptLoggedEvent(std::shared_ptr<JsScript> script);
};
class JsScriptLoggedFilter : public EventFilter<JsScriptLoggedEvent> {
protected:
    std::shared_ptr<JsScript> m_script;

public:
    using Callback = void(JsScriptLoggedEvent*);

    JsScriptLoggedFilter() = default;
    JsScriptLoggedFilter(std::shared_ptr<JsScript> script);

    ListenerResult handle(std::function<Callback> fn, JsScriptLoggedEvent* ev);
};

class ScriptManager final {
private:
    std::vector<std::shared_ptr<JsScript>> m_scripts;

public:
    static ScriptManager* get();

    std::vector<std::shared_ptr<JsScript>> getScripts() const;

    void reloadScripts();

    bool tickAll();
};
