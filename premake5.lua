--#############__GENERAL__CONFIGURATION__SETTINGS__#############
local function CommonConfigurationSettings()
    GlobalCommonDefines()

    defines {
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
--        targetname ("%{prj.name}")
--        defines {"ZSPACE_STATIC_LIBRARY"}
--        optimize "Off"
--        warnings "Off"
--        --symbols "On"
--        flags {"MultiProcessorCompile"}
--        buildoptions {"/bigobj"}

    filter "configurations:Debug_DLL*"
        kind "SharedLib"
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
--        targetname ("%{prj.name}")
--        defines {"ZSPACE_STATIC_LIBRARY",
--                 "NDEBUG"}
--        optimize "Full"
--        warnings "Off"
--        flags {"LinkTimeOptimization",
--                "MultiProcessorCompile"}
--        buildoptions {"/bigobj"}

    filter "configurations:Release_DLL*"
        kind "SharedLib"
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

--Redefine to stop interference
CoreIncludeDir = prependPath(deps_path, get_include_dirs())
CoreLibDir = prependPath(deps_path, get_lib_dirs())

--#############__ZSPACE_APP__#############
project "zSpace_App"
    location "projects/zSpace_App"
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

    --###__BASE__###
    includedirs
    {
        "%{CoreIncludeDir.ARMADILLO}",
        "src/headers",
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{cfg.target}",
    }


--#############__ZSPACE_CORE__#############
project "zSpace_Core"
    location "projects/zSpace_Core"
    language "C++"
    cppdialect "C++17"

    characterset("MBCS")

    CommonConfigurationSettings()

    pchheader "zCore/zcorepch.h"
    pchsource "src/source/zCore/zcorepch.cpp"
    rawforceincludes "zCore/zcorepch.h"

    files
    {
        "src/headers/zCore/**.h",
        "src/source/zCore/**.cpp",
        --Source files of Includes
        "%{CoreIncludeDir.LODEPNG}/lodepng.h",
        "%{CoreIncludeDir.LODEPNG}/lodepng.cpp",
        "%{CoreIncludeDir.TOOJPEG}/*.cpp"
    }

    --###__BASE__###
    includedirs
    {
        "%{CoreIncludeDir.ARMADILLO}",
        "%{CoreIncludeDir.LODEPNG}",
        "%{CoreIncludeDir.TOOJPEG}",
        "%{CoreIncludeDir.EIGEN}",
        "%{CoreIncludeDir.SQLITE}",
        "%{CoreIncludeDir.NLOHMANN}",
        "%{CoreIncludeDir.QUICKHULL}",
        --local
        "src/headers",
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{cfg.targetdir}",
    }

    links
    {
        "sqlite3.lib",
    }


--#############__ZSPACE_INTERFACE__#############
project "zSpace_Interface"
    location "projects/zSpace_Interface"
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

    --WIP
    filter {"files:**zFnComputeMesh.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    --###__BASE__###
    includedirs
    {
        "%{CoreIncludeDir.DEPS}",
        "%{CoreIncludeDir.ARMADILLO}",
        "%{CoreIncludeDir.EIGEN}",
        "%{CoreIncludeDir.NLOHMANN}",
        "%{CoreIncludeDir.TOOJPEG}",
        "%{CoreIncludeDir.LODEPNG}",
        "%{CoreIncludeDir.QUICKHULL}",
        "%{CoreIncludeDir.IGL}",
        --local
        "src/headers",
        --Omniverse
        "%{CoreIncludeDir.OV_CLIENT}",
        "%{CoreIncludeDir.OV_USD_RES}",
        "%{CoreIncludeDir.OV_PYTHON}",
        "%{CoreIncludeDir.OV_TINYTOML}",
        "%{CoreIncludeDir.OV_USD}",
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{CoreLibDir.IGL}",
        "%{cfg.targetdir}",
        --Omniverse
        "%{CoreLibDir.OV_CLIENT}",
        "%{CoreLibDir.OV_USD_RES}",
        "%{CoreLibDir.OV_PYTHON}",
        "%{CoreLibDir.OV_USD}",
    }

    links
    {
        "sqlite3.lib",
        "zSpace_Core.lib",
        "igl.lib",
    }

    --###__OMNIVERSE__###
    --Omniverse is default
    links {get_omniverse_links()}


--#############__ZSPACE_INTEROP__#############
project "zSpace_InterOp"
    location "projects/zSpace_InterOp"
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

    --Exclude zObjCurve WIP
    filter {"files:**zObjCurve.*"}
        flags {"ExcludeFromBuild"}
    filter {}

    --###__BASE__###
    includedirs
    {
        "%{CoreIncludeDir.DEPS}",
        "%{CoreIncludeDir.ARMADILLO}",
        "%{CoreIncludeDir.LODEPNG}",
        "%{CoreIncludeDir.TOOJPEG}",
        "%{CoreIncludeDir.EIGEN}",
        "%{CoreIncludeDir.SQLITE}",
        "%{CoreIncludeDir.NLOHMANN}",
        "%{CoreIncludeDir.QUICKHULL}",
        "%{CoreIncludeDir.IGL}",
        "%{maya_dir}/include",
        --local
        "src/headers",
        --Omniverse
        "%{CoreIncludeDir.OV_CLIENT}",
        "%{CoreIncludeDir.OV_USD_RES}",
        "%{CoreIncludeDir.OV_PYTHON}",
        "%{CoreIncludeDir.OV_TINYTOML}",
        "%{CoreIncludeDir.OV_USD}",
        --Rhino
        "%{rhino_dir}/inc"
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{maya_dir}/lib",
        "%{cfg.targetdir}",
        --Omniverse
        "%{CoreLibDir.OV_CLIENT}",
        "%{CoreLibDir.OV_USD_RES}",
        "%{CoreLibDir.OV_PYTHON}",
        "%{CoreLibDir.OV_USD}",
        --Rhino
        "%{rhino_dir}/lib/Release"
    }

    links
    {
        "zSpace_Core.lib",
        "zSpace_Interface.lib",
        "sqlite3.lib",
        --Maya
        "OpenMayaRender.lib",
        "OpenMayaFX.lib",
        "OpenMayaAnim.lib",
        "OpenMaya.lib",
        "OpenMayaUI.lib",
        "Foundation.lib",
    }

    --###__OMNIVERSE__###
    --Omniverse is default
    links {get_omniverse_links()}

    --###__RHINO__###
    filter {"configurations:*Rhino"}
        delayloaddlls
        {
            "opennurbs.dll",
            "RhinoCore.dll",
            "RhinoLibrary.dll",
        }

        links
        {
            "opennurbs.lib",
            "RhinoCore.lib",
            "RhinoLibrary.lib", --This lib should be in Rhino 7 SDK, if it's not ask Vishu
        }
    filter {}
