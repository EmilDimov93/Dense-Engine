// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "scripter.hpp"

#include "core/assertions.hpp"

#include <stdexcept>
#include <string>

namespace DE::scripter
{
    XMFLOAT3 &findPosXMFLOAT3(SceneDescription &sceneDescription, const Handle &handle)
    {
        for (ModelInstanceDescription &modelInstanceDescription : sceneDescription.modelInstanceDescriptions)
            if (modelInstanceDescription.handle == handle)
                return modelInstanceDescription.transform.pos;

        for (PointLightDescription &pointLightDescription : sceneDescription.pointLightDescriptions)
            if (pointLightDescription.handle == handle)
                return pointLightDescription.pos;

        throw std::runtime_error("Invalid handle used in script (" + std::to_string(handle) + ")");
    }

    XMFLOAT3 &findRotXMFLOAT3(SceneDescription &sceneDescription, const Handle &handle)
    {
        for (ModelInstanceDescription &modelInstanceDescription : sceneDescription.modelInstanceDescriptions)
            if (modelInstanceDescription.handle == handle)
                return modelInstanceDescription.transform.rot;

        for (DirectionalLightDescription &directionalLightDescription : sceneDescription.directionalLightDescriptions)
            if (directionalLightDescription.handle == handle)
                return directionalLightDescription.direction;

        throw std::runtime_error("Invalid handle used in script (" + std::to_string(handle) + ")");
    }

    XMFLOAT3 &findScaleXMFLOAT3(SceneDescription &sceneDescription, const Handle &handle)
    {
        for (ModelInstanceDescription &modelInstanceDescription : sceneDescription.modelInstanceDescriptions)
            if (modelInstanceDescription.handle == handle)
                return modelInstanceDescription.transform.scale;

        throw std::runtime_error("Invalid handle used in script (" + std::to_string(handle) + ")");
    }

    XMFLOAT3 &findColorXMFLOAT3(SceneDescription &sceneDescription, const Handle &handle)
    {
        for (PointLightDescription &pointLightDescription : sceneDescription.pointLightDescriptions)
            if (pointLightDescription.handle == handle)
                return pointLightDescription.color;

        for (DirectionalLightDescription &directionalLightDescription : sceneDescription.directionalLightDescriptions)
            if (directionalLightDescription.handle == handle)
                return directionalLightDescription.color;

        throw std::runtime_error("Invalid handle used in script (" + std::to_string(handle) + ")");
    }

    float &findIntensityFloat(SceneDescription &sceneDescription, const Handle &handle)
    {
        for (PointLightDescription &pointLightDescription : sceneDescription.pointLightDescriptions)
            if (pointLightDescription.handle == handle)
                return pointLightDescription.intensity;

        for (DirectionalLightDescription &directionalLightDescription : sceneDescription.directionalLightDescriptions)
            if (directionalLightDescription.handle == handle)
                return directionalLightDescription.intensity;

        throw std::runtime_error("Invalid handle used in script (" + std::to_string(handle) + ")");
    }

    float &findRangeFloat(SceneDescription &sceneDescription, const Handle &handle)
    {
        for (PointLightDescription &pointLightDescription : sceneDescription.pointLightDescriptions)
            if (pointLightDescription.handle == handle)
                return pointLightDescription.range;

        throw std::runtime_error("Invalid handle used in script (" + std::to_string(handle) + ")");
    }

    bool execute(const Script &script, SceneDescription &sceneDescription)
    {
        try
        {
            for (const instruction::Instruction &instruction : script)
            {
                if (const instruction::MOVE *moveInstruction = std::get_if<instruction::MOVE>(&instruction))
                    findPosXMFLOAT3(sceneDescription, moveInstruction->handle) += moveInstruction->pos;
                else if (const instruction::ROTATE *rotateInstruction = std::get_if<instruction::ROTATE>(&instruction))
                    findRotXMFLOAT3(sceneDescription, rotateInstruction->handle) += rotateInstruction->rot;
                else if (const instruction::SCALE *scaleInstruction = std::get_if<instruction::SCALE>(&instruction))
                    findScaleXMFLOAT3(sceneDescription, scaleInstruction->handle) += scaleInstruction->scale;
                else if (const instruction::COLOR *colorInstruction = std::get_if<instruction::COLOR>(&instruction))
                    findColorXMFLOAT3(sceneDescription, colorInstruction->handle) += colorInstruction->color;
                else if (const instruction::INTENSITY *intensityInstruction = std::get_if<instruction::INTENSITY>(&instruction))
                    findIntensityFloat(sceneDescription, intensityInstruction->handle) += intensityInstruction->intensity;
                else if (const instruction::RANGE *rangeInstruction = std::get_if<instruction::RANGE>(&instruction))
                    findRangeFloat(sceneDescription, rangeInstruction->handle) += rangeInstruction->range;
                else if (const instruction::EXPOSURE *exposureInstruction = std::get_if<instruction::EXPOSURE>(&instruction))
                    sceneDescription.exposure += exposureInstruction->exposure;
                else if (const instruction::AMBIENT *ambientInstruction = std::get_if<instruction::AMBIENT>(&instruction))
                    sceneDescription.ambient += ambientInstruction->ambient;
            }
        }
        catch (const std::runtime_error &e)
        {
            warn(e.what());
            return false;
        }

        return true;
    }
}