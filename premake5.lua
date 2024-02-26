include("./premake/delay_load.lua")
include("includes.lua")

workspace "zSpace_core"
    filename "zSpace_core"
    architecture "x64"
    configurations {"Debug", "Debug_DLL", "Release", "Release_DLL", "Release_DLL_OV", "Release_Make", "Release_Unreal"}
    startproject "zSpace_Core"

project_path = "projects"

IncludeDir = get_include_dirs()

LibDir = get_lib_dirs()

--#############__GENERAL__CONFIGURATION__SETTINGS__#############
function CommonConfigurationSettings()
    filter "configurations:Debug"
        kind "StaticLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/lib/debug")
        targetname ("%{prj.name}")
        defines {"ZSPACE_STATIC_LIBRARY",
                "USING_ARMA"}
        symbols "On"

    filter "configurations:Debug_DLL"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/dll/")
        targetname ("%{prj.name}")
        symbols "On"

    filter "configurations:Release"
        kind "StaticLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/lib/")
        targetname ("%{prj.name}")
        defines {"ZSPACE_STATIC_LIBRARY",
                "USING_ARMA"}
        optimize "Full"
        warnings "Off"
        flags {"LinkTimeOptimization"}

    filter "configurations:Release_DLL"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/dll/")
        targetname ("%{prj.name}")
        defines {"ZSPACE_DYNAMIC_LIBRARY"}
        optimize "Speed"
        warnings "Off"
        flags {"LinkTimeOptimization"}

    filter "configurations:Release_DLL_OV"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/dll/")
        targetname ("%{prj.name}")
        defines {"ZSPACE_DYNAMIC_LIBRARY"}
        optimize "Speed"
        warnings "Off"
        flags {"LinkTimeOptimization"}
    
    filter "configurations:Release_Unreal"
        kind "StaticLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/lib/")
        targetname ("%{prj.name}")
        defines {"ZSPACE_STATIC_LIBRARY"}
        optimize "Speed"
        warnings "Off"
        flags {"LinkTimeOptimization"}
    
    filter {}
end


--#########################################
project "zSpace_App"
    location "%{project_path}/zSpace_App"
    language "C++"
    cppdialect "C++17"
    dependson("zSpace_Interface")

    CommonConfigurationSettings()

    filter {"files:**zArchGeom.h"}
        flags {"ExcludeFromBuild"}
    filter {}

    files
    {
        "src/headers/zApp/**.h",
        "src/source/zApp/**.cpp",
        --Source files of Includes
        "%{IncludeDir.ALGLIB}/**.cpp",
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        --"%{IncludeDir.ALGLIB}",
        "%{IncludeDir.SRC}",
        "%{IncludeDir.DEPS}",
    }

    libdirs
    {
        --"%{LibDir.GLEW}",
        "%{LibDir.SQLITE}",
        "%{LibDir.OUTDLL}",
        "%{LibDir.OUTLIB}",
    }

    links
    {
        "zSpace_Core.lib",
        "zSpace_Interface.lib",
        "sqlite3.lib",
        --"opengl32.lib",
        --"glew32.lib"
    }

    --ZPACE_APP_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release_Unreal"
        defines {"USING_ARMA"}

    filter "configurations:Release_Make"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/dll/")
        targetname ("%{prj.name}")
    

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


    files
    {
        "src/headers/zCore/**.h",
        "src/source/zCore/**.cpp",
        --Source files of Includes
        --"%{IncludeDir.ALGLIB}/**.cpp",
        "%{IncludeDir.LODEPNG}/lodepng.h",
        "%{IncludeDir.LODEPNG}/lodepng.cpp",
        "%{IncludeDir.TOOJPEG}/*.cpp"
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        --"%{IncludeDir.ALGLIB}",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.TOOJPEG}",
        "%{IncludeDir.EIGEN}",
        "%{IncludeDir.SQLITE}",
        --"%{IncludeDir.GLEW}",
        --"%{IncludeDir.FREEGLUT}",
        "%{IncludeDir.NLOHMANN}",
        "%{IncludeDir.QUICKHULL}",
        "C:/Program Files/Rhino 7 SDK/inc",
        "%{IncludeDir.SRC}",
    }

    libdirs
    {
        --"%{LibDir.GLEW}",
        "%{LibDir.SQLITE}",
        --"%{LibDir.FREEGLUT}",
        "C:/Program Files/Rhino 7 SDK/lib/Release"
    }

    links
    {
        "sqlite3.lib",
        --"opengl32.lib",
        --"glew32.lib"
    }

    --ZPACE_CORE_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release_Unreal"
        defines {"ZSPACE_UNREAL_INTEROP"}

    filter "configurations:Release_Make"
        kind "Makefile"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/make")
        targetname ("%{prj.name}")


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

    files
    {
        "src/headers/zInterface/**.h",
        "src/source/zInterface/**.cpp",
        --Source files of Includes
        --"%{IncludeDir.ALGLIB}/**.cpp"
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
        --"%{IncludeDir.ALGLIB}",
        "%{IncludeDir.EIGEN}",
        "%{IncludeDir.NLOHMANN}",
        "%{IncludeDir.TOOJPEG}",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.QUICKHULL}",
        --"%{IncludeDir.GLEW}",
        --"%{IncludeDir.FREEGLUT}",
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
        --"%{LibDir.GLEW}",
        "%{LibDir.SQLITE}",
        --"%{LibDir.FREEGLUT}",
        "%{LibDir.OUTDLL}",
        "%{LibDir.OUTLIB}",
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

    --ZPACE_INTERFACE_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release_Unreal"
        defines {"ZSPACE_UNREAL_INTEROP"}

    filter "configurations:Release_Make"
        kind "Makefile"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("bin/make")
        targetname ("%{prj.name}")

    filter "configurations:Release_DLL_OV"
        defines {"ZSPACE_USD_INTEROP"}


--#########################################        
project "zSpace_InterOp"
    location "%{project_path}/zSpace_InterOp"
    language "C++"
    cppdialect "C++17"
    dependson("zSpace_Interface")

    CommonConfigurationSettings()

    files
    {
        "src/headers/zInterOp/**.h",
        "src/source/zInterOp/**cpp",
        --Source files of Includes
        --"%{IncludeDir.ALGLIB}/**.cpp"
    }

    --Exclude zObjCurve
    filter {"files:**zObjCurve.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        --"%{IncludeDir.ALGLIB}",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.TOOJPEG}",
        "%{IncludeDir.EIGEN}",
        "%{IncludeDir.SQLITE}",
        --"%{IncludeDir.GLEW}",
        --"%{IncludeDir.FREEGLUT}",
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
        --"%{LibDir.GLEW}",
        "%{LibDir.FREEGLUT}",
        "%{LibDir.SQLITE}",
        "C:/Program Files/Rhino 7 SDK/lib/Release",
        "C:/Program Files/Autodesk/Maya2020/lib",
        "%{LibDir.OUTDLL}",
        "%{LibDir.OUTLIB}",
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
        --"freeglut.lib",
    }

    --All of the Omniverse links
    links {get_omniverse_links()}

    --ZPACE_INTEROP_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release"
        defines {"ZSPACE_UNREAL_INTEROP",
                "ZSPACE_RHINO_INTEROP",
                "NDEBUG",
                "WIN64",
                "_UNICODE",
                "UNICODE"}
    
    filter "configurations:Release_DLL"
        defines {
                "ZSPACE_RHINO_INTEROP",
                "ZSPACE_DYNAMIC_LIBRARY",
                "NDEBUG",
                "WIN64",
                "_UNICODE",
                "UNICODE",
                "_HAS_STD_BYTE=0"}

    filter "configurations:Release_DLL_OV"
        defines {"ZSPACE_RHINO_INTEROP",
                "ZSPACE_USD_INTEROP",
                "ZSPACE_DYNAMIC_LIBRARY",
                "NDEBUG",
                "WIN64",
                "_UNICODE",
                "UNICODE",
                "_HAS_STD_BYTE=0",
                "BOOST_ALL_DYN_LINK",
                [[BOOST_LIB_TOOLSET="vc142"]],
                "TBB_USE_DEBUG=0",
                "_CRT_SECURE_NO_WARNINGS",
                "NOMINMAX",
                "_WINDLL"
            }

    filter "configurations:Release_Unreal"
        defines{"ZSPACE_RHINO_INTEROP",
                "NDEBUG",
                "WIN64",
                "_UNICODE",
                "UNICODE",
                "USING_ARMA"}

    filter "configurations:Release_Make"
        kind "None"
