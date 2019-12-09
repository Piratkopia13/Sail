# SPLASH<sub>2</sub>0

[![Build Status](https://dev.azure.com/BTH-StoraSpel-DXR/Stora%20Spel/_apis/build/status/Master%20pipeline?branchName=master)](https://dev.azure.com/BTH-StoraSpel-DXR/Stora%20Spel/_build/latest?definitionId=1&branchName=master)

***S**cientists **P**acking **L**ights **A**nd **S**hooting **H<sub>2</sub>0***

> *"A fluid first person shooter"*

A game developed by 12 people as part of a 14 week-long course under the name *Large Game Project (PA2526)* at [Blekinge Institute of Technology](https://www.bth.se/) in Sweden, 2019.

## Features and used techniques
- Hybrid raytracer
    - Tracing from GBuffers
    - Reflections for one bounce
- Raytraced soft shadows
    - Support for many light sources
    - Using spatiotemporal filtering for denoising
- Water "blob" rendering
    - Using metaball calculations through an intersection shader
- Water rendering on surfaces
    - Using a voxel grid and metaball calculations which modifies surface materials
- Post processing
    - Bloom
    - FXAA
    - Tonemapping
- PBR
    - With support for emissive materials
- Skeletal animations
    - Updated on GPU
- Our own physics engine
    - Octree (for collisions, raycasting, and frustum culling)
    - AABB-AABB collisions
    - AABB-Mesh collisions
    - Swept sphere collision detection for fast moving objects
    - Ragdoll physics for torch
- Entity Component System
- Multiplayer network
    - With state interpolation
- Audio
    - Stream-from-file
    - Spatial sound (HRTF)
- AI
    - Adaptive node system
    - A* path finding
    - Finite state machine
- Procedurally generated maps
- Instant replay (kill cam)

#### Devs:
[Samuel Asp](https://github.com/Smaugmuel)

[David Bengtsson](https://github.com/Discojanne)

[Gustav Björk](https://github.com/Praccen)

[Viktor Enfeldt](https://github.com/viktor4006094)

[Tobias Fast](https://github.com/tofb15)

[Daniel Fredriksson](https://github.com/DanielFredriksson)

[Henrik Johansson](https://github.com/h3nx)

[Fredrik Junede](https://github.com/Skratzy)

[Peter Meunier](https://github.com/soridanm)

[Emil Wahl](https://github.com/whalemane)

[Alexander Wester](https://pirat.dev)

[Oliver Glandberger](https://github.com/OliverGlandberger)

![](https://pbs.twimg.com/profile_images/1162476165736456194/NUBsjYSV_200x200.jpg "RTX ON")

Licenses
----
Our code (Sail/src/ and SPLASH/src/) is published under the MIT license

We also make use of the FBX SDK:

*This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2008 Autodesk, Inc. All rights, reserved. Such code is provided "as is" and Autodesk, Inc. disclaims any and all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-infringement of third party rights. In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code.*

See submodules for other specific licenses.

----

ƪ(ړײ)‎ƪ​​

**Free Software, Hell Yeah!**
