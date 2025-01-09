# JAGE

(J)ust (A)nother (G)ame (E)ngine. It is what it is! Highly unoptimize, written by an imbecile. This exists because I want to learn about 3D graphics. I'm following an ECS approach to developing it.

Made using C++20 and OpenGL 4.60.
 
## Features

- Components, static meshes, point lights, directional lights, textures, materials;
- Deferred rendering;
- Easy to load new assets (3d models, shaders, etc);
- Easily add components to entities;

# TODO

- PBR rendering;
- Skeletal meshes;
- Run animations;
- Particles;
- Audio; 
- Physics;

## To optimize

- Draw calls are dumb, one per static mesh instance, no buffer memory management yete - write [MDI](https://ktstephano.github.io/rendering/opengl/mdi);
- Make better use of uniform BOs;
- Vulkan? Ray tracing? Sure.
- I want to see an ocean, maybe implement this in the future
- Try to implement something crazy that has been developed in the recent years and fail.

## Resources

- [Guy in the couch writing a FPS from scratch while chilling, you probably saw him on Youtube at least once - I trust this guy](https://www.youtube.com/@tokyospliff);
- [Learn Open GL](https://learnopengl.com/)