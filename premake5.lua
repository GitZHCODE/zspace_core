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

    filter "configurations:*Debug*"
        targetname ("%{prj.name}")
        optimize "Off"
        warnings "Off"
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

    filter "configurations:*Release*"
        targetname ("%{prj.name}")
        defines {"NDEBUG"}
        optimize "Full"
        warnings "Off"
        flags {"LinkTimeOptimization",
                "MultiProcessorCompile"}

    filter "configurations:*DLL*"
        kind "SharedLib"
        defines {"ZSPACE_DYNAMIC_LIBRARY",
                 "_WINDLL"}

    filter {}
end

path_from_core_to_zspace_deps = path.getrelative(zspace_core_path, zspace_deps_path)
path_from_core_to_zspace_toolsets = path.getrelative(zspace_core_path, zspace_toolsets_path)

-- Redefine to keep paths relative
CoreIncludeDir = prependPath(path_from_core_to_zspace_deps, get_zspace_include_dirs())
CoreLibDir = prependPath(path_from_core_to_zspace_deps, get_zspace_lib_dirs())

--#############__ZSPACE_APP__#############
project "zSpace_App"
    location "projects/zSpace_App/%{wks.name}"
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
        "%{path_from_core_to_zspace_toolsets}/src/headers",
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{cfg.target}",
    }


--#############__ZSPACE_CORE__#############
project "zSpace_Core"
    location "projects/zSpace_Core/%{wks.name}"
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

    --###__Injected__###
        --Inject included dirs
        includedirs {get_include_dirs_injection()}

        --Inject lib dirs
        libdirs {get_lib_dirs_injection()}

        --Inject links
        links {get_links_injection()}
    --##################

    --###__Omniverse__###
    filter {"platforms:Omniverse or Both"}
        -- Add omniverse includes
        includedirs {prependPath(path_from_core_to_zspace_deps, get_omniverse_includes())}

        -- Add omniverse libdirs
        libdirs {prependPath(path_from_core_to_zspace_deps, get_omniverse_libdirs())}

        -- Add omniverse links
        links {get_omniverse_links()}
    filter{}


--#############__ZSPACE_INTERFACE__#############
project "zSpace_Interface"
    location "projects/zSpace_Interface/%{wks.name}"
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
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{CoreLibDir.IGL}",
        "%{cfg.targetdir}",
    }

    links
    {
        "sqlite3.lib",
        "zSpace_Core.lib",
        "igl.lib",
    }
    --##############

    --###__Injected__###
        -- Inject included dirs
        includedirs {get_include_dirs_injection()}

        -- Inject lib dirs
        libdirs {get_lib_dirs_injection()}

        -- Inject links
        links{get_links_injection()}
    --##################

    --###__Omniverse__###
    filter {"platforms:Omniverse or Both"}
        -- Add omniverse includes
        includedirs {prependPath(path_from_core_to_zspace_deps, get_omniverse_includes())}

        -- Add omniverse libdirs
        libdirs {prependPath(path_from_core_to_zspace_deps, get_omniverse_libdirs())}

        -- Add omniverse links
        links {get_omniverse_links()}
    filter {}


--#############__ZSPACE_INTEROP__#############
project "zSpace_InterOp"
    location "projects/zSpace_InterOp/%{wks.name}"
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
    }

    libdirs
    {
        "%{CoreLibDir.SQLITE}",
        "%{maya_dir}/lib",
        "%{cfg.targetdir}",
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

    --###__Injected__###
        --Inject included dirs
        includedirs {get_include_dirs_injection()}

        -- Inject lib dirs
        libdirs {get_lib_dirs_injection()}
    --##################

    --###__Omniverse__###
    filter {"platforms:Omniverse or Both"}
        -- Add omniverse includes
        includedirs {prependPath(path_from_core_to_zspace_deps, get_omniverse_includes())}

        -- Add omniverse libdirs
        libdirs {prependPath(path_from_core_to_zspace_deps, get_omniverse_libdirs())}

        -- Add omniverse links
        links {get_omniverse_links()}
    --###################

    --###__RHINO__###
    filter {"platforms:Rhino or Both"}

        includedirs
        {
            "%{rhino_dir}/inc"
        }

        libdirs
        {
            "%{rhino_dir}/lib/Release"
        }

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
    --###############
