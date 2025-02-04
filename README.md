# OpenGL-ArealLights

## Description

This repository demonstrates the implementation of Areal Lights (Area Lights) in OpenGL using C++ and GLSL. It showcases the rendering of light sources with a defined area, providing softer shadows and more realistic lighting effects compared to point lights.

## Screenshots

![Screenshot 1](Screenshot1.png)

## Folder Structure

*   `ArealLights/x64/Debug`: Contains build files and output executables.
*   `resources`: Contains model and texture assets used in the project.
*   `shaders`: Contains GLSL shader programs for rendering and lighting.
*   `x64/Debug`: Contains debug build output.

## Dependencies

Before building and running this project, you need to install the following dependencies:

*   **GLEW (OpenGL Extension Wrangler Library):** Download GLEW and add the `include` directory and library files (e.g., `glew32.lib`) to your system environment variables.
*   **GLM (OpenGL Mathematics):** Download GLM. GLM is a header-only library, so simply copy the `glm` directory into your project's include path.

**Note:** Adding GLEW and GLM to your system environment variables ensures that Visual Studio can find them during the build process. This typically involves adding paths to the `INCLUDE` and `LIB` environment variables.

## Build Instructions (Visual Studio 2022)

Follow these steps to build the project using Visual Studio 2022:

1.  **Clone the Repository:** Clone this repository to your local machine using the following command:
    ```bash
    git clone https://github.com/Tushar-Wagdare/OpenGL-ArealLights.git
    ```
2.  **Open the Solution:** Open the `OGL.sln` file in Visual Studio 2022.
3.  **Build the Solution:** Go to `Build` -> `Build Solution` (or press `Ctrl+Shift+B`).
4.  **Ensure all dependencies are present:** Make sure all the dependencies are installed and placed in the system environment.
5.  **Run the Solution:** After building, run the solution.

## Usage

After successfully building the project, you can run the executable located in the `x64/Debug` directory. The demo showcases the Areal Lights effect with soft shadows.
