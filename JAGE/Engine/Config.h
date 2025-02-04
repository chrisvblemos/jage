#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <inih/ini.h>
#include <Logging.h>
#include <format>

namespace Cfg {

    #define ConfigLog "Config"
    #define CFG_ENGINE_PATH "Config/Engine.ini"
    #define CFG_RENDERING_PATH "Config/Rendering.ini"

    class CfgBase {
    public:
        CfgBase(const std::string& fpath) : fpath(fpath) {};

        template <typename T>
        T Read(const std::string& section, const std::string& key, const T deft);

        template <>
        float Read<float>(const std::string& section, const std::string& key, const float deft) {
            INIReader reader(fpath);
            if (reader.ParseError() != 0) {
                LOG(ConfigLog, LOG_CRITICAL, std::format("Failed to parse config file {}", fpath));
            }

            return reader.GetFloat(section, key, deft);
        }

        template <>
        uint32_t Read<uint32_t>(const std::string& section, const std::string& key, const uint32_t deft) {
            INIReader reader(fpath);
            if (reader.ParseError() != 0) {
                LOG(ConfigLog, LOG_CRITICAL, std::format("Failed to parse config file {}", fpath));
            }

            return reader.GetInteger(section, key, deft);
        }

        template <>
        bool Read<bool>(const std::string& section, const std::string& key, const bool deft) {
            INIReader reader(fpath);
            if (reader.ParseError() != 0) {
                LOG(ConfigLog, LOG_CRITICAL, std::format("Failed to parse config file {}", fpath));
            }

            return reader.GetBoolean(section, key, deft);
        }

        template <>
        std::string Read<std::string>(const std::string& section, const std::string& key, const std::string deft) {
            INIReader reader(fpath);
            if (reader.ParseError() != 0) {
                LOG(ConfigLog, LOG_CRITICAL, std::format("Failed to parse config file {}", fpath));
            }

            return reader.Get(section, key, deft);
        }

    private:
        std::string fpath;
    };

    inline CfgBase Engine(CFG_ENGINE_PATH);
    inline CfgBase Rendering(CFG_RENDERING_PATH);
}

