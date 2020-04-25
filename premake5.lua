workspace "QuadExplorer"
	configurations { "Debug", "Release"}
	platforms "x64"
	systemversion "10.0.16299.0"
	staticruntime "on"

filter { "platforms:x64" }
	includedirs 
	{
		"Source",
		"Depen/AwesomeEngine/Source",
		"Depen/AwesomeEngine/Assets/Shaders",
		"Depen/AwesomeEngine/Depen/GLM",
		"Depen/AwesomeEngine/Depen/PhysX/physx/include",
		"Depen/AwesomeEngine/Depen/PhysX/pxshared/include",
		"Board/lib/QuadFlyController/src"
	}

filter {"configurations:Debug"}
	libdirs
	{
		"Depen/AwesomeEngine/Build/x64/Debug"
	}
	links
	{
		"Core",
		"Graphics"
	}

filter {"configurations:Release"}
	libdirs
	{
		"Depen/AwesomeEngine/Build/x64/Release"
	}
	links
	{
		"Core",
		"Graphics"
	}


project "QuadExplorerApp"
	kind "WindowedApp"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/**.h",
		"Source/**.cpp",
		"Board/lib/QuadFlyController/**.cpp",
		"Board/lib/QuadFlyController/**.h"
	}
	filter "configurations:Debug"
		symbols "On"
		links
		{
			"Graphics", "Core"
		}
		postbuildcommands 
		{
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\assimp\\bin\\%{cfg.buildcfg}\\assimp-vc141-mtd.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\assimp-vc141-mtd.dll",
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\PhysX\\physX\\bin\\win.x86_64.vc141.mt\\debug\\PhysXCommon_64.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysXCommon_64.dll",
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\PhysX\\physX\\bin\\win.x86_64.vc141.mt\\debug\\PhysX_64.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysX_64.dll",
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\PhysX\\physX\\bin\\win.x86_64.vc141.mt\\debug\\PhysXFoundation_64.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysXFoundation_64.dll"
		}
	filter "configurations:Release"
		optimize "On"
		links
		{
			"Graphics", "Core"
		}
		postbuildcommands 
		{
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\assimp\\bin\\%{cfg.buildcfg}\\assimp-vc141-mt.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\assimp-vc141-mt.dll",
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\PhysX\\physX\\bin\\win.x86_64.vc141.mt\\release\\PhysXCommon_64.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysXCommon_64.dll",
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\PhysX\\physX\\bin\\win.x86_64.vc141.mt\\release\\PhysX_64.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysX_64.dll",
			"copy %{wks.location}Depen\\AwesomeEngine\\Depen\\PhysX\\physX\\bin\\win.x86_64.vc141.mt\\release\\PhysXFoundation_64.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysXFoundation_64.dll",
		}