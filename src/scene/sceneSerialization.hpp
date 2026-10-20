// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "sceneDescription.hpp"

#include "core/assertions.hpp"
#include "version.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include <limits>
#include <filesystem>

namespace DE
{
    constexpr const char *endOfObject = "eoo";

    inline bool saveScene(const std::filesystem::path &filePath, const SceneDescription &sceneDescription)
    {
        std::ofstream outputFile(filePath);
        if (!outputFile.is_open())
        {
            warn("Failed to save scene");
            return false;
        }

        outputFile << std::setprecision(std::numeric_limits<float>::max_digits10);

        outputFile << DENSE_ENGINE_VERSION << '\n';

        outputFile << "Models" << '\n';

        for (const ModelDescription &modelDescription : sceneDescription.modelDescriptions)
        {
            outputFile << modelDescription.handle << '\n'
                       << modelDescription.filePath << '\n'
                       << endOfObject << '\n';
        }

        outputFile << "Instances" << '\n';

        for (const ModelInstanceDescription &instanceDescription : sceneDescription.modelInstanceDescriptions)
        {
            outputFile << instanceDescription.handle << '\n'
                       << instanceDescription.modelHandle << '\n'
                       << instanceDescription.transform.pos.x << '\n'
                       << instanceDescription.transform.pos.y << '\n'
                       << instanceDescription.transform.pos.z << '\n'
                       << instanceDescription.transform.rot.x << '\n'
                       << instanceDescription.transform.rot.y << '\n'
                       << instanceDescription.transform.rot.z << '\n'
                       << instanceDescription.transform.scale.x << '\n'
                       << instanceDescription.transform.scale.y << '\n'
                       << instanceDescription.transform.scale.z << '\n'
                       << endOfObject << '\n';
        }

        outputFile.close();
        if (outputFile.fail())
        {
            warn("Failed to save scene");
            return false;
        }

        return true;
    }

    inline bool loadScene(const std::filesystem::path &filePath, SceneDescription &sceneDescription)
    {
        sceneDescription = SceneDescription();

        std::ifstream inputFile(filePath);
        if (!inputFile.is_open())
        {
            warn("Failed to load scene file");
            return false;
        }

        try
        {
            std::string line;
            if (!std::getline(inputFile, line))
            {
                warn("Scene file is empty");
                return false;
            }
            else
            {
                if (stoi(line) != DENSE_ENGINE_VERSION)
                {
                    warn("Scene file version mismatch. Scene(" + line + "). Client version (" + std::to_string(DENSE_ENGINE_VERSION) + ")");
                }
            }

            const uint32_t contextSectionId = 0u;
            const uint32_t modelsSectionId = 1u;
            const uint32_t instancesSectionId = 2u;
            uint32_t currSectionID = contextSectionId;
            uint32_t tokenCounter = 0;
            ModelDescription currModel;
            ModelInstanceDescription currInstance;
            while (std::getline(inputFile, line))
            {
                if (line == "Models")
                {
                    currSectionID = modelsSectionId;
                    tokenCounter = 0;
                    continue;
                }
                else if (line == "Instances")
                {
                    currSectionID = instancesSectionId;
                    tokenCounter = 0;
                    continue;
                }

                switch (currSectionID)
                {
                case modelsSectionId:
                {
                    if (line == endOfObject)
                    {
                        sceneDescription.modelDescriptions.push_back(currModel);
                        tokenCounter = 0;
                        continue;
                    }
                    else
                    {
                        switch (tokenCounter)
                        {
                        case 0:
                            currModel.handle = stoi(line);
                            break;
                        case 1:
                            currModel.filePath = line;
                            break;
                        }
                    }
                    break;
                }
                case instancesSectionId:
                {
                    if (line == endOfObject)
                    {
                        sceneDescription.modelInstanceDescriptions.push_back(currInstance);
                        tokenCounter = 0;
                        continue;
                    }
                    else
                    {
                        switch (tokenCounter)
                        {
                        case 0:
                            currInstance.handle = stoi(line);
                            break;
                        case 1:
                            currInstance.modelHandle = stoi(line);
                            break;
                        case 2:
                            currInstance.transform.pos.x = stof(line);
                            break;
                        case 3:
                            currInstance.transform.pos.y = stof(line);
                            break;
                        case 4:
                            currInstance.transform.pos.z = stof(line);
                            break;
                        case 5:
                            currInstance.transform.rot.x = stof(line);
                            break;
                        case 6:
                            currInstance.transform.rot.y = stof(line);
                            break;
                        case 7:
                            currInstance.transform.rot.z = stof(line);
                            break;
                        case 8:
                            currInstance.transform.scale.x = stof(line);
                            break;
                        case 9:
                            currInstance.transform.scale.y = stof(line);
                            break;
                        case 10:
                            currInstance.transform.scale.z = stof(line);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    continue;
                }
                }

                tokenCounter++;
            }
        }
        catch (const std::exception &)
        {
            warn("Failed to load scene: corrupted file");
            sceneDescription = SceneDescription();
            return false;
        }

        return true;
    }
}