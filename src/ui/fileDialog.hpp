// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <shobjidl.h>
#include <wrl/client.h>
#include <filesystem>
#include <optional>

namespace DE
{
    inline std::optional<std::filesystem::path> openFileDialog(HWND ownerWindowHandle, COMDLG_FILTERSPEC fileTypeFilter)
    {
        Microsoft::WRL::ComPtr<IFileOpenDialog> fileOpenDialog;
        if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog))))
            return std::nullopt;

        const COMDLG_FILTERSPEC fileTypeFilters[] = {
            fileTypeFilter,
            {L"All Files", L"*.*"},
        };
        fileOpenDialog->SetFileTypes(static_cast<UINT>(std::size(fileTypeFilters)), fileTypeFilters);
        fileOpenDialog->SetTitle(fileTypeFilter.pszName);

        DWORD dialogOptions = 0;
        fileOpenDialog->GetOptions(&dialogOptions);
        fileOpenDialog->SetOptions(dialogOptions | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

        if (FAILED(fileOpenDialog->Show(ownerWindowHandle)))
            return std::nullopt;

        Microsoft::WRL::ComPtr<IShellItem> selectedItem;
        if (FAILED(fileOpenDialog->GetResult(&selectedItem)))
            return std::nullopt;

        PWSTR selectedFilePath = nullptr;
        if (FAILED(selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &selectedFilePath)))
            return std::nullopt;

        std::filesystem::path result(selectedFilePath);
        CoTaskMemFree(selectedFilePath);
        return result;
    }

    inline std::optional<std::filesystem::path> saveFileDialog(HWND ownerWindowHandle)
    {
        Microsoft::WRL::ComPtr<IFileSaveDialog> fileSaveDialog;
        if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileSaveDialog))))
            return std::nullopt;

        const COMDLG_FILTERSPEC fileTypeFilters[] = {
            {L"Dense Engine Scene", L"*.descene"},
            {L"All Files", L"*.*"}
        };
        fileSaveDialog->SetFileTypes(static_cast<UINT>(std::size(fileTypeFilters)), fileTypeFilters);
        fileSaveDialog->SetTitle(L"Save as .descene");
        fileSaveDialog->SetFileName(L"scene");
        fileSaveDialog->SetDefaultExtension(L"descene");

        DWORD dialogOptions = 0;
        fileSaveDialog->GetOptions(&dialogOptions);
        fileSaveDialog->SetOptions(dialogOptions | FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT | FOS_PATHMUSTEXIST);

        if (FAILED(fileSaveDialog->Show(ownerWindowHandle)))
            return std::nullopt;

        Microsoft::WRL::ComPtr<IShellItem> selectedItem;
        if (FAILED(fileSaveDialog->GetResult(&selectedItem)))
            return std::nullopt;

        PWSTR selectedFilePath = nullptr;
        if (FAILED(selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &selectedFilePath)))
            return std::nullopt;

        std::filesystem::path result(selectedFilePath);
        CoTaskMemFree(selectedFilePath);
        return result;
    }
}