-- premake5.lua
workspace "Sail"
	configurations { "Debug", "Release" }
	platforms { "x64", "x86", }

	filter "platforms:*86"
		architecture "x86"

	filter "platforms:*64"
		architecture "x64"

local binDir = "bin/%{cfg.platform}-%{cfg.buildcfg}"
local intermediatesDir = "intermediates/%{prj.name}-%{cfg.platform}-%{cfg.buildcfg}"

project "Demo"
	location "Demo"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir (binDir)
	objdir (intermediatesDir)

	files { 
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	-- include and fix these as soon as new cross-platform architecture is finished
	removefiles { 
		"**/ParticleHandler.*",
		"**/PlayerCameraController.*",
		"**/Scene.*"
	}

	includedirs {
		"libraries",
		"Sail/src",
		"libraries/FBX_SDK/include"
	}

	links {
		"Sail"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	-- Copy fbxsdk dll to executable path
	filter { "action:vs2017 or vs2019", "platforms:*64" }
		postbuildcommands {
			"{COPY} ../libraries/FBX_SDK/lib/vs2017/x64/%{cfg.buildcfg}/libfbxsdk.dll %{cfg.targetdir}"
		}
	-- filter { "action:vs2017 or vs2019", "platforms:*86" }
	-- 	postbuildcommands {
	-- 		("{COPY} libraries/FBX_SDK/lib/vs2017/x86/debug/libfbxsdk.dll %{cfg.targetdir}")
	-- 	}


project "Sail"
	location "Sail"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.platform}-%{cfg.buildcfg}"
	objdir (intermediatesDir)
	cppdialect "C++17"
	staticruntime "on"

	pchheader "pch.h"
	pchsource "Sail/src/pch.cpp"

	files { 
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	-- include and fix these as soon as new cross-platform architecture is finished
	removefiles { 
		"**/DXCubeMap.*",
		"%{prj.name}/src/Sail/resources/audio/**",
		"**/Skybox.*",
		"**/ParticleEmitter.*",
		"%{prj.name}/src/Sail/graphics/shadows/**",
		"**/VertexShader.*",
		"**/PixelShader.*",
		"**/HullShader.*",
		"**/ComputeShader.*",
		"**/GeometryShader.*",
		"**/DomainShader.*",
		"%{prj.name}/src/Sail/graphics/shader/postprocess/**",
		"%{prj.name}/src/Sail/graphics/shader/instanced/**",
		"%{prj.name}/src/Sail/graphics/shader/deferred/**",
		"%{prj.name}/src/Sail/graphics/shader/component/ConstantBuffer**",
		"%{prj.name}/src/Sail/graphics/shader/component/Sampler**",
		"%{prj.name}/src/Sail/graphics/shader/basic/**",
		"**/OldDeferredRenderer.*",
		"**/DeferredRenderer.*",
		"%{prj.name}/src/Sail/graphics/postprocessing/**",
		"**/Quadtree.*",
		"%{prj.name}/src/Sail/api/Renderer.*"
	}

	includedirs {
		"libraries",
		"Sail/src",
		"libraries/FBX_SDK/include",
		"libraries/glfw/include"
	}

	links {
		"libfbxsdk",
		"d3d11.lib",
		"d3dcompiler.lib"
	}

	filter { "action:vs2017 or vs2019", "platforms:*64" }
		libdirs {
			"libraries/FBX_SDK/lib/vs2017/x64/%{cfg.buildcfg}"
		}
	filter { "action:vs2017 or vs2019", "platforms:*86" }
		libdirs {
			"libraries/FBX_SDK/lib/vs2017/x86/%{cfg.buildcfg}"
		}

	filter "system:windows"
		systemversion "latest"

		defines {
			"FBXSDK_SHARED",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"