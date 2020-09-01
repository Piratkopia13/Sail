-- premake5.lua
workspace "Sail"
	configurations { "Debug", "Release" }
	platforms { "DX11 x64", "DX11 x86",
				"DX12 x64", "DX12 x86",
				"pVulkan x64", "pVulkan x86", -- starting a platform name with 'V' changes the platform toolset in VS. Bug in premake?
			  }

	filter "platforms:*86"
		architecture "x86"

	filter "platforms:*64"
		architecture "x64"

local binDir = "bin/%{cfg.platform}-%{cfg.buildcfg}/%{prj.name}"
local intermediatesDir = "intermediates/%{prj.name}-%{cfg.platform}-%{cfg.buildcfg}"

IncludeDir = {}
IncludeDir["GLFW"] = "libraries/glfw/include"
IncludeDir["FBX_SDK"] = "libraries/FBX_SDK/include"
IncludeDir["ImGui"] = "libraries/imgui"
IncludeDir["vulkan"] = os.getenv("VULKAN_SDK").."/Include"

group "Libraries"
include "libraries/glfw"
include "libraries/imgui"

group ""

-----------------------------------
--------------  DEMO --------------
-----------------------------------
project "Demo"
	location "Demo"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir (binDir)
	objdir (intermediatesDir)

	files { 
		"%{prj.name}/Demo.rc",    -- For icon
		"%{prj.name}/resource.h", -- For icon
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
		"%{IncludeDir.FBX_SDK}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.vulkan}"
	}

	links {
		"Sail"
	}

	defines { "NOMINMAX",				-- Removes min max macros which cause issues
			  "WIN32_LEAN_AND_MEAN" }	-- Exclude some less used APIs to speed up the build process on windows
	flags { "MultiProcessorCompile" }

	filter { "platforms:DX11*" }
		defines {
			"_SAIL_DX11"
		}
	filter { "platforms:DX12* or Vulkan*" }
		defines {
			"_SAIL_DX12"
		}
	filter { "platforms:*Vulkan*" }
		defines {
			"_SAIL_VK"
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
			"{COPY} \"../libraries/FBX_SDK/lib/vs2017/x64/%{cfg.buildcfg}/libfbxsdk.dll\" \"%{cfg.targetdir}\""
		}
	filter { "action:vs2017 or vs2019", "platforms:*86" }
		postbuildcommands {
			"{COPY} \"../libraries/FBX_SDK/lib/vs2017/x86/%{cfg.buildcfg}/libfbxsdk.dll\" \"%{cfg.targetdir}\""
		}

-----------------------------------
--------------  Sail --------------
-----------------------------------
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
		"%{prj.name}/src/API/DX11/**",
		"%{prj.name}/src/API/DX12/**",
		"%{prj.name}/src/API/VULKAN/**",

		"**/DXCubeMap.*",
		"%{prj.name}/src/Sail/resources/audio/**",
		"**/Skybox.*",
		"**/ParticleEmitter.*",
		"%{prj.name}/src/Sail/graphics/shadows/**",
		"%{prj.name}/src/Sail/graphics/postprocessing/**",
		"**/Quadtree.*"
	}

	includedirs {
		"libraries",
		"Sail/src",
		"%{IncludeDir.FBX_SDK}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.vulkan}"
	}

	links {
		"libfbxsdk",
		"GLFW",
		"ImGui",
		"vulkan-1"
	}

	flags { "MultiProcessorCompile" }

	defines {
		"SAIL_PLATFORM=\"%{cfg.platform}\"",
		-- "_SAIL_BREAK_ON_WARNING",
		"_SAIL_BREAK_ON_ERROR"
	}

	filter { "platforms:DX11*" }
		defines {
			"_SAIL_DX11"
		}
		files {
			"%{prj.name}/src/API/DX11/**",
			"%{prj.name}/src/API/Windows/**"
		}
	filter { "platforms:DX12*" }
		defines {
			"_SAIL_DX12"
		}
		files {
			"%{prj.name}/src/API/DX12/**",
			"%{prj.name}/src/API/Windows/**"
		}
	filter { "platforms:*Vulkan*" }
		defines {
			"_SAIL_VK"
		}
		files {
			"%{prj.name}/src/API/VULKAN/**",
			"%{prj.name}/src/API/Windows/**"
		}

	filter { "action:vs2017 or vs2019", "platforms:*64" }
		libdirs {
			"libraries/FBX_SDK/lib/vs2017/x64/%{cfg.buildcfg}",
			os.getenv("VULKAN_SDK").."/Lib"
		}
	filter { "action:vs2017 or vs2019", "platforms:*86" }
		libdirs {
			"libraries/FBX_SDK/lib/vs2017/x86/%{cfg.buildcfg}",
			os.getenv("VULKAN_SDK").."/Lib32"
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
