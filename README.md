<center>

```

     ██╗ █████╗  ██████╗ ███████╗
     ██║██╔══██╗██╔════╝ ██╔════╝
     ██║███████║██║  ███╗█████╗  
██   ██║██╔══██║██║   ██║██╔══╝  
╚█████╔╝██║  ██║╚██████╔╝███████╗
 ╚════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝

(J)ust (A)nother (G)ame (E)ngine

```

</center>

<figure align = "center">
    <img src="cool.png" align="center" alt="scene01" width="600"/>
    <figcaption><p align="center">Screenshot from 2025-01-21.</p></figcaption>
</figure>


 On its path to be optimized, written by an imbecile. This exists because I want to learn about 3D graphics.

Game objects are handled via [entity-component-system](https://en.wikipedia.org/wiki/Entity_component_system). Heavily inspired by Unity and Unreal Engine.

## Dependencies

- CPP+20
- OpenGL 4.6.0
 
## Graphics API Supported Features

Below is a list of features supported by the engine at the moment. Gets updated over time as I implement new features.

- [Variance Shadow Mapping](https://developer.download.nvidia.com/SDK/10/direct3d/Source/VarianceShadowMapping/Doc/VarianceShadowMapping.pdf);
- [Cascaded Shadow Maps](https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps)
- [Multi-Draw Indirect](https://ktstephano.github.io/rendering/opengl/mdi)
- [Shadow Smoothing PCF - Poisson Sampling](https://electronicmeteor.wordpress.com/2013/02/05/poisson-disc-shadow-sampling-ridiculously-easy-and-good-looking-too/)
- [Deferred Shading](https://en.wikipedia.org/wiki/Deferred_shading)
- [Screen Space Ambient Occlusion](https://en.wikipedia.org/wiki/Screen_space_ambient_occlusion)

## Building

At the moment, I only provide build configurations for Visual Studio Community 2022 and Visual Studio Code.

**Visual Studio Community 2022**

Just open the solution and build it. All assets and dlls will automatically be copied over to the build folder.

**Visual Studio Code**

Make sure you have MSVC set up in your Windows. All necessary settings are included in the repo inside .vscode folder - should be path agnostic. Currently uses `gl` to build game. Supports C++ extension debugger.