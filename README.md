# JAGE

(J)ust (A)nother (G)ame (E)ngine. Highly unoptimized, written by an imbecile. This exists because I want to learn about 3D graphics. Sort of [ECS](https://en.wikipedia.org/wiki/Entity_component_system) at play here.
Made using C++20 and OpenGL 4.60.
 
## Features

You compile the code and suddenly and you can move around a beautiful 3D rendered backpack.

# TODO

- PBR rendering;
- Skeletal meshes;
- Run animations;
- Particles;
- Audio; 
- Physics;
- Vulkan? Ray tracing? Sure.
- I want to see an ocean, maybe implement this in the future
- Try to implement something crazy that has been developed in the recent years and fail.

## To optimize

- Draw calls are dumb, one per static mesh instance, no buffer memory management yet - write [MDI](https://ktstephano.github.io/rendering/opengl/mdi);
- Make better use of uniform BOs;

## Resources

- [Guy in the couch writing a FPS from scratch while chilling, you probably saw him on Youtube at least once - I trust this guy](https://www.youtube.com/@tokyospliff);
- [Learn Open GL](https://learnopengl.com/)