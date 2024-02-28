include("./premake/delay_load.lua")
include("includes.lua")

workspace "zSpace_core"
    filename "zSpace_core"
    architecture "x64"
    configurations {
                    --"Release",
                    "Release_DLL",
                    --"Debug",
                    "Debug_DLL",
                }
    startproject "zSpace_Core"

project_path = "projects"

IncludeDir = get_include_dirs()
LibDir = get_lib_dirs()

--#############__GENERAL__CONFIGURATION__SETTINGS__#############
function CommonConfigurationSettings()

    defines {
            "ZSPACE_RHINO_INTEROP",
            "ZSPACE_USD_INTEROP",
            "WIN64",
            "_UNICODE",
            "UNICODE",
            "_HAS_STD_BYTE=0",
            "BOOST_ALL_DYN_LINK",
            [[BOOST_LIB_TOOLSET="vc142"]],
            "TBB_USE_DEBUG=0",
            "_CRT_SECURE_NO_WARNINGS",
            "NOMINMAX",
            }

--    filter "configurations:Debug"
--        kind "StaticLib"
--        objdir ("bin-int/%{cfg.buildcfg}")
--        targetdir ("bin/lib/debug/")
--        targetname ("%{prj.name}")
--        defines {"ZSPACE_STATIC_LIBRARY"}
--        optimize "Off"
--        warnings "Off"
--        --symbols "On"
--        flags {"MultiProcessorCompile"}
--        buildoptions {"/bigobj"}

    filter "configurations:Debug_DLL"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.buildcfg}")
        targetdir ("bin/dll/debug/")
        targetname ("%{prj.name}")
        defines {"ZSPACE_DYNAMIC_LIBRARY",
                 "_WINDLL"}
        optimize "Off"
        warnings "Off"
        --symbols "On"
        flags {"MultiProcessorCompile"}
        buildoptions {"/bigobj"}

--    filter "configurations:Release"
--        kind "StaticLib"
--        objdir ("bin-int/%{cfg.buildcfg}")
--        targetdir ("bin/lib/")
--        targetname ("%{prj.name}")
--        defines {"ZSPACE_STATIC_LIBRARY",
--                 "NDEBUG"}
--        optimize "Full"
--        warnings "Off"
--        flags {"LinkTimeOptimization",
--                "MultiProcessorCompile"}
--        buildoptions {"/bigobj"}

    filter "configurations:Release_DLL"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.buildcfg}")
        targetdir ("bin/dll/")
        targetname ("%{prj.name}")
        defines {"ZSPACE_DYNAMIC_LIBRARY",
                 "NDEBUG",
                 "_WINDLL"}
        optimize "Full"
        warnings "Off"
        flags {"LinkTimeOptimization",
                "MultiProcessorCompile"}
    
    filter {}
end


--#########################################
project "zSpace_App"
    location "%{project_path}/zSpace_App"
    language "C++"
    cppdialect "C++17"
    dependson("zSpace_Interface")

    CommonConfigurationSettings()

    kind ("None")

    filter {"files:**zArchGeom.h"}
        flags {"ExcludeFromBuild"}
    filter {}

    files
    {
        "src/headers/zApp/**.h",
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.SRC}",
        "%{IncludeDir.DEPS}",
    }

    libdirs
    {
        "%{LibDir.SQLITE}",
        "%{LibDir.OUTDLL}",
        "%{LibDir.OUTLIB}",
        "%{LibDir.DEBUG_OUTDLL}",
        "%{LibDir.DEBUG_OUTLIB}",
    }

--#########################################
project "zSpace_Core"
    location "%{project_path}/zSpace_Core"
    language "C++"
    cppdialect "C++17"

    characterset("MBCS")

    CommonConfigurationSettings()

    filter {"files:**zUtilsDisplay.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    filter {"files:**zObjBuffer.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    pchheader "zCore/zcorepch.h"
    pchsource "src/source/zCore/zcorepch.cpp"
    rawforceincludes "zCore/zcorepch.h"

    files
    {
        "src/headers/zCore/**.h",
        "src/source/zCore/**.cpp",
        --Source files of Includes
        "%{IncludeDir.LODEPNG}/lodepng.h",
        "%{IncludeDir.LODEPNG}/lodepng.cpp",
        "%{IncludeDir.TOOJPEG}/*.cpp"
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.TOOJPEG}",
        "%{IncludeDir.EIGEN}",
        "%{IncludeDir.SQLITE}",
        "%{IncludeDir.NLOHMANN}",
        "%{IncludeDir.QUICKHULL}",
        "C:/Program Files/Rhino 7 SDK/inc",
        "%{IncludeDir.SRC}",
    }

    libdirs
    {
        "%{LibDir.SQLITE}",
        "C:/Program Files/Rhino 7 SDK/lib/Release"
    }

    links
    {
        "sqlite3.lib",
    }


--#########################################
project "zSpace_Interface"
    location "%{project_path}/zSpace_Interface"
    language "C++"
    cppdialect "C++17"
    dependson("zSpace_Core")

    CommonConfigurationSettings()

    defines{
        "_HAS_STD_BYTE=0",
        "NOMINMAX",
    }

    pchheader "zInterface/zinterfacepch.h"
    pchsource "src/source/zInterface/zinterfacepch.cpp"
    rawforceincludes "zInterface/zinterfacepch.h"

    files
    {
        "src/headers/zInterface/**.h",
        "src/source/zInterface/**.cpp",
    }

    filter {"files:**zFnComputeMesh.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    filter {"files:**zModel.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.EIGEN}",
        "%{IncludeDir.NLOHMANN}",
        "%{IncludeDir.TOOJPEG}",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.QUICKHULL}",
        "%{IncludeDir.IGL}",
        "%{IncludeDir.SRC}",
        "%{IncludeDir.DEPS}",
        --Omniverse
        "%{IncludeDir.OV_CLIENT}",
        "%{IncludeDir.OV_USD_RES}",
        "%{IncludeDir.OV_PYTHON}",
        "%{IncludeDir.OV_TINYTOML}",
        "%{IncludeDir.OV_USD}",
    }

    libdirs
    {
        "%{LibDir.SQLITE}",
        "%{LibDir.OUTDLL}",
        "%{LibDir.OUTLIB}",
        "%{LibDir.DEBUG_OUTDLL}",
        "%{LibDir.DEBUG_OUTLIB}",
        --Omniverse
        "%{LibDir.OV_CLIENT}",
        "%{LibDir.OV_USD_RES}",
        "%{LibDir.OV_PYTHON}",
        "%{LibDir.OV_USD}",
    }

    links
    {
        "sqlite3.lib",
        "zSpace_Core.lib",
    }

    --All of the Omniverse links
    links {get_omniverse_links()}


--#########################################        
project "zSpace_InterOp"
    location "%{project_path}/zSpace_InterOp"
    language "C++"
    cppdialect "C++17"
    dependson("zSpace_Interface")

    CommonConfigurationSettings()

    pchheader "zInterOp/zinteroppch.h"
    pchsource "src/source/zInterOp/zinteroppch.cpp"
    rawforceincludes "zInterOp/zinteroppch.h"

    files
    {
        "src/headers/zInterOp/**.h",
        "src/source/zInterOp/**cpp",
    }

    --Exclude zObjCurve
    filter {"files:**zObjCurve.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.TOOJPEG}",
        "%{IncludeDir.EIGEN}",
        "%{IncludeDir.SQLITE}",
        "%{IncludeDir.NLOHMANN}",
        "%{IncludeDir.QUICKHULL}",
        "C:/Program Files/Rhino 7 SDK/inc",
        "C:/Program Files/Autodesk/Maya2020/include",
        "%{IncludeDir.IGL}",
        "%{IncludeDir.SRC}",
        "%{IncludeDir.DEPS}",
        --Omniverse
        "%{IncludeDir.OV_CLIENT}",
        "%{IncludeDir.OV_USD_RES}",
        "%{IncludeDir.OV_PYTHON}",
        "%{IncludeDir.OV_TINYTOML}",
        "%{IncludeDir.OV_USD}",
    }

    libdirs
    {
        "%{LibDir.SQLITE}",
        "C:/Program Files/Rhino 7 SDK/lib/Release",
        "C:/Program Files/Autodesk/Maya2020/lib",
        "%{LibDir.OUTDLL}",
        "%{LibDir.OUTLIB}",
        "%{LibDir.DEBUG_OUTDLL}",
        "%{LibDir.DEBUG_OUTLIB}",
        --Omniverse
        "%{LibDir.OV_CLIENT}",
        "%{LibDir.OV_USD_RES}",
        "%{LibDir.OV_PYTHON}",
        "%{LibDir.OV_USD}",
    }

    delayloaddlls
    {
        "opennurbs.dll",
        "RhinoCore.dll",
        "RhinoLibrary.dll",
    }

    links
    {
        "zSpace_Core.lib",
        "zSpace_Interface.lib",
        --Rhino
        "opennurbs.lib",
        "RhinoCore.lib",
        "RhinoLibrary.lib", --This lib should be in Rhino 7 SDK, if it's not ask Vishu
        --Maya
        "OpenMayaRender.lib",
        "OpenMayaFX.lib",
        "OpenMayaAnim.lib",
        "OpenMaya.lib",
        "OpenMayaUI.lib",
        "Foundation.lib",
        "sqlite3.lib",
    }

    --All of the Omniverse links
    links {get_omniverse_links()}