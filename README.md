
# Sail game engine :sailboat:

[![Build status](https://ci.appveyor.com/api/projects/status/aqy3xsricje3cv28/branch/master?svg=true)](https://ci.appveyor.com/project/Piratkopia13/sail/branch/master)

Sail is a small C++ game engine developed and maintained as a hobby project

Current features include
  - Cross API support - DirectX 11 or 12 
  - Deferred rendering
  - Physically Based Rendering
  - Entity component system
  - Subscription based event system
  - Hot reloadable shaders
  - .. and more

### Installation

Sail is tested for Visual Studio 2019 but should work with earlier versions as well.
Clone repository using the recursive flag
`git clone --recursive https://github.com/Piratkopia13/Sail.git`
If asked for git lfs credentials, the password is the same as the username.
Run `generateVS19.bat` inside the cloned directory or modify it before running to generate solutions files for another IDE than vs19.
Start `Sail.sln` and build!
Everything required to build should be contained in this repo. Hit me up if there are any issues.


### Future things I want to implement

 - DXR raytracing (In the works)
 - Vulkan support

License
----

My code (Sail/src/ and Demo/src/) is published under the MIT license

I also make use of the FBX SDK:

*This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2008 Autodesk, Inc. All rights, reserved. Such code is provided "as is" and Autodesk, Inc. disclaims any and all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-infringement of third party rights. In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code.*

----

ƪ(ړײ)‎ƪ​​

**Free Software, Hell Yeah!**
