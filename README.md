<h1 align="center" style="border-bottom: none;">
  <a href="https://github.com/OnionGalaxy/LittleOrionEngine/">LittleOrion Engine</a>
</h1>
<h3 align="center">C++ 3D Engine developed during the Master's Degree in AAA Videogames Development</h3>
<p align="center">
  <a href="https://github.com/OnionGalaxy/LittleOrionEngine/releases/">
    <img alt="GitHub version" src="https://img.shields.io/github/v/release/OnionGalaxy/LittleOrionEngine?include_prereleases">
  </a>
  <a href="https://github.com/OnionGalaxy/LittleOrionEngine/stargazers/">
    <img alt="GitHub stars" src="https://img.shields.io/github/stars/OnionGalaxy/LittleOrionEngine.svg">
  </a>
  <a href="https://github.com/OnionGalaxy/LittleOrionEngine/network/">
    <img alt="GitHub forks" src="https://img.shields.io/github/forks/OnionGalaxy/LittleOrionEngine.svg">
  </a>
  <a href="https://github.com/OnionGalaxy/LittleOrionEngine/">
    <img alt="GitHub repo size in bytes" src="https://img.shields.io/github/repo-size/OnionGalaxy/LittleOrionEngine.svg">
  </a>
  <a href="https://github.com/OnionGalaxy/LittleOrionEngine/blob/master/LICENSE">
    <img alt="GitHub license" src="https://img.shields.io/github/license/OnionGalaxy/LittleOrionEngine.svg">
  </a>
</p>

![](https://github.com/OnionGalaxy/LittleOrionEngine/blob/master/Media/README_usage/engine.png)

## Build With VS2019
clone my github:https://github.com/SiCoYu/YXK_LittleOrionEngine

Operate as follow:

Prepare:

clone https://github.com/OnionGalaxy/LittleOrionEngine

Download [LittleOrionEnginev1.6.0.zip] from Releases

Two differect directory.

### Step1:
Download [LittleOrionEnginev1.6.0.zip](https://github.com/OnionGalaxy/LittleOrionEngine/releases/download/v1.6.0/LittleOrionEnginev1.6.0.zip)
### Step2:
Unzip LittleOrionEnginev1.6.0.zip
### Step3:
Copy "LittleOrionEnginev1.6.0\Assets\Scripts" to "LittleOrionEngine\Engine\Script"
### Step4:
Rename class name, "Scripts/EventManager" to "Scripts/CEventManager"
### Step5:
Copy "LittleOrionEnginev1.6.0\Assets" and "LittleOrionEnginev1.6.0\Resources" to "LittleOrionEngine\Game"
### Step6:
Use VS2019 Open LittleOrionEngine.sln

Switch WinSDK to "10.0.22621.0"

Switch VC++ to "v142"
### Step7:
"Path.h“ add "#include <iostream>"

"ModuleAudio.h" add  "#include <iostream>"
### Step8:
Add "LittleOrionEngine\Engine\Script" to LittleOrionEngine.sln
### Step9:
Set sln to Debug/Win32
### Step10:
Build imgui

Build MathGeoLib

Build recast
### Step11:
Build LittleOrionEngine（setup as startup project)
### Step12:
Wait to build finish.

## Highlights
- Create, save and load scenes.
- Create GameObjects and modify their behaviours using Component system.
- Explore the scene using a Unity-like camera.
- Configure different engine properties: renderer, time management, ...

## Installation
- Download the last release (or whatever release you want) from the [Releases section](https://github.com/Unnamed-Company/LittleOrionEngine/releases).
- Extract `Build.zip`.
- Execute `LittleOrionEngine.exe`.

## Usage
Please refer to our [Wiki](https://github.com/Unnamed-Company/LittleOrionEngine/wiki) in order to learn how to use the engine.

## Development setup
In order to do that assure that you have the following:
- A computer.
- A graphic card with OpenGL support.
- [VisualStudio 2017 or above](https://visualstudio.microsoft.com/es/).

## Contributing
Because this is a academic project is not possible to contribute directly to this repo. Said that, feel free to fork it (<https://github.com/Unnamed-Company/LittleOrionEngine/fork>) and to expand it in your own way!

## Authors
<table>
  <tr>
    <td align="center"><a href="https://github.com/mariofv"><img src="https://avatars2.githubusercontent.com/u/17348688?v=4" width="100px;" alt=""/><br /><sub><b>Mario Fernández Villalba</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/yalania"><img src="https://avatars2.githubusercontent.com/u/48101863?v=4" width="100px;" alt=""/><br /><sub><b>Anabel Hernández Barrera</b></sub></a><br /></td>  
    <td align="center"><a href="https://github.com/Dartemiss"><img src="https://avatars2.githubusercontent.com/u/18176345?v=4" width="100px;" alt=""/><br /><sub><b>Enrique Alexandre González Sequeira</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/JoanStinson"><img src="https://avatars2.githubusercontent.com/u/24247839?v=4" width="100px;" alt=""/><br /><sub><b>Joan Ginard Mateo</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/JorxSS"><img src="https://avatars2.githubusercontent.com/u/49075678?v=4" width="100px;" alt=""/><br /><sub><b>Jordi Sauras</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/JordiRomagosa"><img src="https://avatars2.githubusercontent.com/u/9912719?v=4" width="100px;" alt=""/><br /><sub><b>Jordi Romagosa</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/Kibium"><img src="https://avatars2.githubusercontent.com/u/24247683?v=4" width="100px;" alt=""/><br /><sub><b>Toni Ferrari Juan</b></sub></a><br /></td>
 </tr>
 <tr>
     <td align="center"><a href="https://github.com/luismoyano"><img src="https://avatars2.githubusercontent.com/u/39064112?v=4" width="100px;" alt=""/><br /><sub><b>Luis Moyano</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/marcorod94"><img src="https://avatars2.githubusercontent.com/u/29025282?v=4" width="100px;" alt=""/><br /><sub><b>Marco Rodríguez</b></sub></a><br /></td>  
    <td align="center"><a href="https://github.com/Misarte"><img src="https://avatars2.githubusercontent.com/u/34191130?v=4" width="100px;" alt=""/><br /><sub><b>Artemis Georgakopoulou</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/pacasasgar"><img src="https://avatars2.githubusercontent.com/u/15977683?v=4" width="100px;" alt=""/><br /><sub><b>Pau Casas Garcia</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/raulgonzalezupc"><img src="https://avatars2.githubusercontent.com/u/56253770?v=4" width="100px;" alt=""/><br /><sub><b>Raúl González</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/RickyVimon"><img src="https://avatars2.githubusercontent.com/u/33130090?v=4" width="100px;" alt=""/><br /><sub><b>Ricard Vivó</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/vidalmelero"><img src="https://avatars2.githubusercontent.com/u/43371298?v=4" width="100px;" alt=""/><br /><sub><b>Vidal Melero</b></sub></a><br /></td>
</tr>
</table>

## Developement methodology
This project was developed following agile methodologies philosophy. If you are interested to know the tasks distribution don't hesitate to visit our [Trello board](https://trello.com/b/uFhGXWJ1/littleorionengine)!

## Built With
* [SDL 2.0](https://www.libsdl.org/) - Used to manage window creation and input.
* [glew 2.1.0](http://glew.sourceforge.net/) - Used to manage OpenGL extensions.
* [MathGeoLib 3.0.0](https://github.com/juj/MathGeoLib/) - Used to do the maths.
* [Dear ImGui 1.73](https://github.com/ocornut/imgui/) - Used to build the GUI.
* [ImGui filebrowser](https://github.com/AirGuanZ/imgui-filebrowser) - Used to open file directory with imgui.
* [DevIL 1.8](http://openil.sourceforge.net/) - Used to load images.
* [assimp 5.0](https://github.com/assimp/assimp/) - Used to load 3D models.
* [Font Awesome 5](https://github.com/FortAwesome/Font-Awesome) - Used to display fancy icons.
* [Icon Font Cpp Headers](https://github.com/juliettef/IconFontCppHeaders/) - Used to import Font Awesome icons in C++.
* [PCG](http://www.pcg-random.org/) - Used to generate **good** random numbers in C++.
* [rapidjson 1.1.0](https://github.com/Tencent/rapidjson/) - Used to handle json files.
* [Debug Draw](https://github.com/glampert/debug-draw/) - Used to draw basic shapes.
* [par_shapes](https://github.com/prideout/par/blob/master/par_shapes.h) - Used to generate basic primitives.
* [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) - Used to draw gizmos.
* [PhysFS 3.0.2](http://icculus.org/physfs/) - Used to manage file system.
* [Bullet Physics](https://pybullet.org/wordpress/) - Used to simulate physics.
* [spdlog](https://github.com/gabime/spdlog) - Used to log stuff.

## License
This project is licensed under the MIT License - see the [LICENSE](https://github.com/Unnamed-Company/LittleOrionEngine/blob/master/LICENSE) file for details.

## Release History
* [[1.0.1] Erradicating the bugs and polishing](https://github.com/mariofv/LittleOrionEngine/releases/tag/v1.0.1)
    * Lots of bug fixes.
    * Tuned up controls in order to give best user experience.
    * Performance improvements when loading resources.
* [[1.0.0] v1.0.0 release is here!](https://github.com/mariofv/LittleOrionEngine/releases/tag/v1.0.0)
    * Mesh materials. It mesh has its own material, that stores different textures and light parameters.
    * New `ComponentLight`, that represents a light source for the scene. It's used along the mesh materials.
    * Mouse picking. Game objects can be selected clicking on them in scene window.
    * Gizmos. They are used to modify the transform of the selected object.
* [[0.2.2] Binary files and scene serialization](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.2.2-alpha)
    * Better resources management.
    * Assets are transformed to less space-consuming format.
    * New cache that avoids duplication of resources.
    * Serialization of a scene, that allows loading and saving scenes.
* [[0.2.1] Speeding up the engine!](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.2.1-alpha)
    * New module ModuleCamera. It contains camera frustums that can be attached to GameObjects.
    * Two new tabs, Scene and Game. The former contains the scene seen from the editor camera while the later contains the scene seen from the game camera.
    * Frustum culling. With this improvement meshes that are outside camera frustum are discarded in the render process.
    * Camera clear modes. Camera can be cleared into a skybox or a plain color.
    * Texture is rendered using a texture instead of OpenGL primitives.
    * Quadtree and OctTree. With this improvement we can speed up even more the render process by reducing the number of checks.
    * New module ModuleDebug. It contains usefull Debug functions.
* [[0.2.0] And there were GameObjects](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.2.0-alpha)
    * GameObjects come in. Each GameObject represents an entity that can be modified using components.
    * Components are objects that contain different entities parts, like meshes, materials, etc.  
    * New hierarchy window that represents the scene GameObject hierarchy.
    * Revamped properties window. Here component parameters can be modified.
* [[0.1.5] Final 0.1 version (this is the one teachers)!](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.1.5-alpha)
    * Fixed various bugs.
    * Several refactors in order to follow good C++ practises.
    * Super cool README.
* [[0.1.4] New LOG, preparations for final 0.1 alpha version!](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.1.4-alpha)
    * Revamped log. It displays messages in a clearer and more readable way.
    * Debug messages are captured from Assimp and OpenGL.
    * Filter messages by soruce!
    * Model texture can be overwritten dragging and dropping a png file into the engine window.
* [[0.1.3] wow UI, so much configuration options](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.1.3-alpha)
    * New gorgeous GUI.
    * Lots of new configuration options for different modules (camera, render, ...).
    * Scene is rendered in a separate window.
    * Model properties are shown in a new window.
    * Camera speed and focus is calculated regarding the loaded model size.
* [[0.1.2] Time starts to flow, impending new GUI shows in the horizon...](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.1.2-alpha)
    * Added time module, that controls time flow. It implements two clocks, the real time clock and the game clock.
    * Time can be paused and stepped foward one frame. Also time scale can be changed.
    * FPS can be limited.
    * Started new implementation of GUI. Added some new info to camera config.
* [[0.1.1] This is about model loading and a camera](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.1.1-alpha)
    * Improved model loading. Is possible to load models dragging and dropping their .fbx files into the engine's window. Camera will be scaled accordingly to new model dimensions.
    * Improved camera controls. Pressing F will focus the camera on the loaded model. Also, pressing Alt will make the camera orbit around the current model.
    * Added bounding box. Pressing B will enable/disable the bounding box of the current model.
* [[0.1.0] Here comes the base version!](https://github.com/mariofv/LittleOrionEngine/releases/tag/v0.1.0-alpha)
    * It can load models from fbx files using a diffuse texture.
    * Camera can be controlled in Unity-like style.
    * Engine has basic UI with several config and debug options.
