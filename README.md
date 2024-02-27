# Smoothed Particle Hydrodynamics
<p align="left">
  <a href="#"><img src="https://img.shields.io/github/last-commit/AsulconS/SPH"></a>
  <a href="#"><img src="https://img.shields.io/github/commit-activity/y/AsulconS/SPH"></a>
  <a href="https://github.com/AsulconS/SPH/issues"><img src="https://img.shields.io/github/issues/AsulconS/SPH"></a>
  <a href="https://github.com/AsulconS/SPH/pulls"><img src="https://img.shields.io/github/issues-pr/AsulconS/SPH"></a>
  <a href="#"><img src="https://img.shields.io/github/stars/AsulconS/SPH"></a>
</p>

 Smoothed Particle Hydrodynamics Water Simulation C++ Implementation.

 ## Requirements
  Running `SPH` requires:
  - Any C++ Compiler (Clang, GCC, MSVC, etc.)
  - CMake 3.20 or higher

 ## How to run it?
 ### Option 1: Executing the binaries:
  - Just go to the latest release <a href="https://github.com/AsulconS/SPH/releases/tag/v0.1.0"><img src="https://img.shields.io/github/v/release/AsulconS/SPH"></a>, download and any of the available executables (`blue-fluid.exe`, `volcano.exe`, `legacy.exe`).

 ### Option 2: Building the project
  - First of all, you may have to download the latest `HSGIL` release <a href="https://github.com/AsulconS/HSGIL/releases/tag/v0.2.1"><img src="https://img.shields.io/github/v/release/AsulconS/HSGIL"></a> and extract it directly in the project folder, all bindings are already made on the `CMakeLists.txt` file
  - To configure this project, you only have to define the `SPH_BUILD` variable by passing a defined value to the configure command. For example, for a Release build, exporting compile commands and using Visual Studio 17 2022 from a x64 architecture to a x64 architecture, the configure command would be:
  ```
  cmake -DSPH_BUILD=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -SC:/Path/To/Folder -BC:/Path/To/Folder/build -G "Visual Studio 17 2022" -T host=x64 -A x64
  ```
  - Finally, you can run a typical build command, following the previous example, it would be:
  ```
  cmake --build C:/Path/To/Fonder/build --config Release --target ALL_BUILD
  ```

 ## Running Examples
 ### SPH Running Blue Fluid Example:
 <p align="left">
   <a href="#"><img src="other/res/sph-1.png?raw=true" width="75%"></a>
 </p>

 ### SPH Running Volcano Example:
 <p align="left">
   <a href="#"><img src="other/res/sph-2.png?raw=true" width="75%"></a>
 </p>
