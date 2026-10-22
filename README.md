# <img width="40" height="40" alt="Logo" src="https://github.com/user-attachments/assets/7fef4350-984d-493c-9f7f-dbfb716c6edf" /> Dense Engine

![Language](https://img.shields.io/badge/C%2B%2B-20-00599C)
![Graphics API](https://img.shields.io/badge/DirectX-12-107C10)
![License](https://img.shields.io/badge/license-Apache--2.0-purple)

A DirectX 12 rendering engine for real-time ray tracing.

**⚠️ Note: Dense Engine is still in early development**

<img width="1280" height="745" alt="image" src="https://github.com/user-attachments/assets/f28fb7b2-a49b-4d51-a78c-fd5da57a72bf" />

## Building & Running

Requirements:

- Windows 10 (version 1809) or later
- GPU with DirectX Raytracing support (DXR Tier 1.0, feature level 12_1)
- PowerShell
- CMake >= 3.25
- Microsoft Visual Studio 2022 (C++ workload)
- dxc (DirectX Shader Compiler)

Setup:

- Clone with submodules: ```git clone --recursive https://github.com/EmilDimov93/Dense-Engine.git```

Usage:

- Open PowerShell in the repo root
- Run: ```./run.ps1```
