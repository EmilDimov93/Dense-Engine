# <img width="40" height="40" alt="Logo" src="https://github.com/user-attachments/assets/7fef4350-984d-493c-9f7f-dbfb716c6edf" /> Dense Engine

![Language](https://img.shields.io/badge/C%2B%2B-20-00599C)
![Graphics API](https://img.shields.io/badge/DirectX-12-107C10)
![License](https://img.shields.io/badge/license-Apache--2.0-purple)

A DirectX 12 rendering engine with real-time ray tracing, meant for smooth and interactive scene editing, allowing great visuals and animation.

<p align="center">
  <img width="100%" alt="Preview - Forest" src="https://github.com/user-attachments/assets/ae6b7027-492a-4d70-8c54-868865b8cdea" />
  <br>
  <sub>Preview - Forest</sub>
</p>

<p align="center">
  <img width="100%" alt="Preview - Tunnel" src="https://github.com/user-attachments/assets/94faa179-6e21-435e-9ea8-a5bc6c94d490" />
  <br>
  <sub>Preview - Tunnel</sub>
</p>

## Features

#### Performance
- **DLOD** (Dynamic Level Of Detail) adjusts the SSAA samples per pixel, downscaling or ray recursion depth to sustain a stable framerate
- **ODR** (On-Demand Rendering) renders a new frame only when something changes, achieving lower power use and less heat

#### Rendering with DirectX 12
- Real-time ray tracing
- SSAA
- Reflections based on metallic and roughness
- Transparency & refractions
- Point & directional lights
- Ambient light, clear color & exposure
- Hard shadows & smooth shading
- Textures
- Tonemapping (ACES)

#### UI with ImGui
- Full control of rendering features
- Manipulate model instances & lights
- Select scene components by clicking on them in the viewport or in the sidebar
- Highlight on the selected model instance and marker on the selected point light
- Camera movement with WASD and rotation by holding the right mouse button
- Ctrl+S to save scene
- Performance Monitor for FPS and rays per frame
- File & folder dialogs from WinAPI

#### Scene
- Multi-threaded model loading
- Support for .obj format
- Saving & loading scene from custom .descene format
- Open .descene files directly with DenseEngine.exe
- Model & model instance setup to avoid loading duplicate models
- CPU data is released after upload

#### Scripter
- Used for animating scene components
- Add instructions for how a component changes every frame
- Write and run scripts from the UI

#### Other
- Screenshot at maximum quality with F2
- Shaders, font & icon embedded in the executable
- Window with WinAPI

## Building & Running

Requirements:

- Windows 10 (version 1809) or later
- GPU with DirectX Raytracing support (DXR Tier 1.0, feature level 12_1)
- PowerShell
- CMake >= 3.25
- Microsoft Visual Studio 2022 (C++ workload)
- dxc (DirectX Shader Compiler)

Setup:

- Clone with submodules: `git clone --recursive https://github.com/EmilDimov93/Dense-Engine.git`

Usage:

- Open PowerShell in the repo root
- Run: `./run.ps1`

## Dependencies

- [**Dear ImGui**](ext/imgui) for UI
- [**stb_image**](ext/stb_image) for texture loading
