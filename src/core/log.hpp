// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "dxcommon.hpp"

#include "version.hpp"

#include <deque>
#include <vector>
#include <string>
#include <string_view>
#include <mutex>
#include <fstream>
#include <chrono>

namespace DE::log
{
    enum Level
    {
        Info,
        Warning,
        Fatal
    };

    struct Entry
    {
    public:
        Entry(Level level, std::string_view msg) : level(level), msg(msg) {}

        std::string str() const
        {
            switch (level)
            {
            case Level::Info:
                return "Info: " + msg;
            case Level::Warning:
                return "Warning: " + msg;
            case Level::Fatal:
                return "Fatal: " + msg;
            default:
                return msg;
            }
        }

    private:
        Level level;
        std::string msg;
    };

    inline constexpr uint32_t MAX_ENTRIES = 100u;

    inline std::deque<Entry> entries;
    inline std::mutex entryMutex;

    inline void getEntries(uint32_t count, std::vector<Entry> &v)
    {
        std::lock_guard lock(entryMutex);
        if (count > entries.size())
            count = static_cast<uint32_t>(entries.size());
        v.assign(entries.end() - count, entries.end());
    }

    inline void add(Level level, std::string_view msg)
    {
        std::lock_guard lock(entryMutex);
        if (entries.size() >= MAX_ENTRIES)
            entries.pop_front();
        entries.emplace_back(level, msg);
    }

    inline void info(std::string_view msg)
    {
        add(Level::Info, msg);
    }

    inline void warn(std::string_view msg)
    {
        add(Level::Warning, msg);
    }

    inline void save()
    {
        std::vector<Entry> entriesVec;
        getEntries(MAX_ENTRIES, entriesVec);

        std::ofstream file("log.txt");

        const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm localTime{};
        localtime_s(&localTime, &now);

        file << "Dense Engine Log - " << std::put_time(&localTime, "%Y-%m-%d - %H:%M:%S") << " - Version " << DENSE_ENGINE_VERSION << "\n\n";

        for (auto &entry : entriesVec)
        {
            file << entry.str() << '\n';
        }
    }

    [[noreturn]] inline void fatal(std::string_view msg)
    {
        add(Level::Fatal, msg);

        save();

#ifdef _MSC_VER
        if (IsDebuggerPresent())
            __debugbreak();
#endif

        abort();
    }
}