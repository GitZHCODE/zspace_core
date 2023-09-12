require('vstudio')

workspace "zSpace_core"
    filename "zSpace_core"
    architecture "x64"
    configurations {"Debug", "Debug_DLL", "Release", "Release_DLL", "Release_DLL_OV", "Release_Make", "Release_Unreal"}
    startproject "zSpace_Core"

project_path = "projects/CPP"

IncludeDir = {}
IncludeDir["ALGLIB"] = "cpp/depends/alglib/cpp/src"
IncludeDir["ARMADILLO"] = "cpp/depends/Armadillo"
IncludeDir["EIGEN"] = "cpp/depends/Eigen/src"
IncludeDir["EIGEN_UNSUPPORTED"]= "cpp/depends/Eigen_Unsupported/Eigen"
IncludeDir["FREEGLUT"] = "cpp/depends/freeglut"
IncludeDir["LODEPNG"] = "cpp/depends/lodePNG"
IncludeDir["NLOHMANN"] = "cpp/depends/nlohmann"
IncludeDir["OPENGL"] = "cpp/depends/openGL"
IncludeDir["QUICKHULL"] = "cpp/depends/quickhull"
IncludeDir["SPA"] = "cpp/depends/spa"
IncludeDir["SPECTRA"] = "cpp/depends/spectra/inlude"
IncludeDir["SQLITE"] = "cpp/depends/SQLITE"
IncludeDir["TOOJPEG"] = "cpp/depends/tooJPEG"

LibDir = {}
LibDir["FREEGLUT"] = "cpp/depends/freeglut"
LibDir["OPENGL"] = "cpp/depends/openGL/lib"
LibDir["SQLITE"] = "cpp/depends/SQLITE/lib" 

PropsFiles = {}
PropsFiles["zCore"] = "../zSpace_Core/zCorePropertySheet.props"
PropsFiles["OV_203"] = "../zSpace_Core/OV_203.props"
PropsFiles["zInterface"] = "../zSpace_Interface/zInterfacePropertySheet.props"
PropsFiles["zInterOp"] = "../zInterOp/zInterOpPropertySheet.props"

PropsDir = {}
--          project        propfiles                   configurations
PropsDir["zSpace_Core"] = {{{"zCore"}, {"Release_DLL", "Release_DLL_OV"}}}

PropsDir["zSpace_Interface"] = {{{"zCore", "OV_203"}, {"Release", "Release_DLL_OV"}},
                                {{"zCore"},           {"Debug", "Release_DLL"}},
                                {{"zInterface"}, {"Release_Make", "Release_Unreal"}}}


--#############__GENERAL__CONFIGURATION__SETTINGS__#############
function CommonConfigurationSettings()
    filter "configurations:Debug"
        kind "ConsoleApp"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("%{cfg.architecture}/%{cfg.buildcfg}")
        targetname ("%{prj.name}")
        symbols "On"

    filter "configurations:Debug_DLL"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("cpp/compiled/dll")
        targetname ("%{prj.name}")
        symbols "On"

    filter "configurations:Release"
        kind "StaticLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("cpp/compiled/dll")
        targetname ("%{prj.name}")
        defines {"ZSPACE_STATIC_LIBRARY",
                "USING_ARMA"}
        optimize "Full"
        warnings "Off"
        flags {"LinkTimeOptimization"}

    filter "configurations:Release_DLL"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("cpp/compiled/dll")
        targetname ("%{prj.name}")
        defines {"ZSPACE_DYNAMIC_LIBRARY"}
        optimize "Speed"
        warnings "Off"
        flags {"LinkTimeOptimization"}

    filter "configurations:Release_DLL_OV"
        kind "SharedLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("cpp/compiled/dll")
        targetname ("%{prj.name}")
        defines {"ZSPACE_DYNAMIC_LIBRARY"}
        optimize "Speed"
        warnings "Off"
        flags {"LinkTimeOptimization"}
    
    filter "configurations:Release_Unreal"
        kind "StaticLib"
        objdir ("bin-int/%{cfg.architecture}/%{cfg.buildcfg}")
        targetdir ("cpp/compiled/UnrealLib")
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

    CommonConfigurationSettings()

    files
    {
        "cpp/headers/zApp/**.h",
        "cpp/source/zApp/**.cpp",
        --Source files of Includes
        "%{IncludeDir.ALGLIB}/**.cpp",
        --Other Prj includes
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.ALGLIB}",
        "cpp/depends",
        "cpp"
    }

    libdirs
    {
        "%{LibDir.OPENGL}",
        "%{LibDir.SQLITE}",
        "cpp/compiled/dll"
    }

    links
    {
        "zSpace_Core.lib",
        "zSpace_Interface.lib",
        "sqlite3.lib",
        "opengl32.lib",
        "glew32.lib"
    }

    --ZPACE_APP_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release_Unreal"
        defines {"USING_ARMA"}

    filter "configurations:Release_Make"
        kind "SharedLib"
    

--#########################################
project "zSpace_Core"
    location "%{project_path}/zSpace_Core"
    language "C++"
    cppdialect "C++17"

    characterset("MBCS")

    CommonConfigurationSettings()

    files
    {
        "cpp/headers/zCore/**.h",
        "cpp/source/zCore/**.cpp",
        --Source files of Includes
        "%{IncludeDir.ALGLIB}/**.cpp",
        "%{IncludeDir.LODEPNG}/lodepng.h",
        "%{IncludeDir.LODEPNG}/lodepng.cpp",
        "%{IncludeDir.TOOJPEG}/*.cpp"
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.ALGLIB}",
        "$(MayaDir)/include",
        "%{IncludeDir.LODEPNG}",
        "%{IncludeDir.TOOJPEG}",
        "cpp/depends",
        "cpp"
    }

    libdirs
    {
        "%{LibDir.OPENGL}",
        "%{LibDir.SQLITE}",
        "%{LibDir.FREEGLUT}"
    }

    links
    {
        "sqlite3.lib",
        "opengl32.lib",
        "glew32.lib"
    }

    --ZPACE_CORE_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release_Unreal"
        defines {"ZSPACE_UNREAL_INTEROP"}

    filter "configurations:Release_Make"
        kind "Makefile"


--#########################################
project "zSpace_Interface"
    location "%{project_path}/zSpace_Interface"
    language "C++"
    cppdialect "C++17"

    CommonConfigurationSettings()

    files
    {
        "cpp/headers/zInterface/**.h",
        "cpp/source/zInterface/**.cpp",
        --Source files of Includes
        "%{IncludeDir.ALGLIB}/**.cpp"
    }
    -- Exclude zFnComputeMesh from builds
    filter {"files:**zFnComputeMesh.cpp"}
        flags {"ExcludeFromBuild"}
    filter {}

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.ALGLIB}",
        "cpp/depends",
        "cpp"
    }

    libdirs
    {
        "%{LibDir.OPENGL}",
        "%{LibDir.SQLITE}",
        "%{LibDir.FREEGLUT}",
        "cpp/compiled/dll"
    }

    links
    {
        "sqlite3.lib",
        "zSpace_Core.lib"
    }

    --ZPACE_INTERFACE_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release_Unreal"
        defines {"ZSPACE_UNREAL_INTEROP"}

    filter "configurations:Release_Make"
        kind "Makefile"


--#########################################        
project "zSpace_InterOp"
    location "%{project_path}/zInterOp"
    language "C++"
    cppdialect "C++17"

    CommonConfigurationSettings()

    files
    {
        "cpp/headers/zInterOp/**.h",
        "cpp/source/zInterOp/**cpp",
        --Source files of Includes
        "%{IncludeDir.ALGLIB}/**.cpp"
    }

    includedirs
    {
        "%{IncludeDir.ARMADILLO}",
        "%{IncludeDir.ALGLIB}",
        "cpp/depends",
        "cpp"
    }

    libdirs
    {
        "%{LibDir.OPENGL}",
        "%{LibDir.SQLITE}",
        "cpp/compiled/dll",
        "$(MayaDir)/lib",
        "$(RhinoDir)/lib/Release"
    }

    links
    {
        "zSpace_Core.lib",
        "zSpace_Interface.lib",
        "opennurbs.lib",
        "RhinoCore.lib",
        "RhinoLibrary.lib",
        "OpenMayaRender.lib",
        "OpenMayaFX.lib",
        "OpenMayaAnim.lib",
        "OpenMaya.lib",
        "OpenMayaUI.lib",
        "Foundation.lib",
        "sqlite3.lib",
    }

    --ZPACE_INTEROP_SPECIFIC_CONFIGURATION_SETTINGS
    filter "configurations:Release"
        defines {"ZSPACE_UNREAL_INTEROP",
                "ZSPACE_RHINO_INTEROP",
                "NDEBUG",
                "WIN64",
                "_UNICODE",
                "UNICODE"}
    
    filter "configurations:Release_DLL"
        defines {"ZSPACE_RHINO_INTEROP",
                 "NDEBUG",
                 "WIN64",
                 "_UNICODE",
                 "UNICODE",
                 "_HAS_STD_BYTE=0"}

    filter "configurations:Release_DLL_OV"
        defines {"ZSPACE_RHINO_INTEROP",
                 "NDEBUG",
                 "WIN64",
                 "_UNICODE",
                 "UNICODE",
                 "_HAS_STD_BYTE=0"}

    filter "configurations:Release_Unreal"
        defines{"ZSPACE_MAYA_INTEROP",
                "ZSPACE_RHINO_INTEROP",
                "NDEBUG",
                "WIN64",
                "_UNICODE",
                "UNICODE",
                "USING_ARMA"}

    filter "configurations:Release_Make"
        kind "None"


--#########################################
--CUSTOM FUNCTIONS

function listContains(list, check)
    for i, v in ipairs(list) do
        if v == check then
            return true
        end
    end
    return false
end

function addPropsFiles(cfg)
    for project, propsinfos in pairs(PropsDir) do
        if cfg.project.name == project then
            for _, propinfo in ipairs(propsinfos) do
                propfiles, configs = propinfo[1], propinfo[2]
                if listContains(configs, cfg.name) then
                    for i = 1, #propfiles do
                        premake.w('<Import Project="%s" />', PropsFiles[propfiles[i]])
                    end
                end    
            end   
        end 
    end
end 

--#########################################
--OVERRIDE CERTAIN FUNCTIONS IN PREMAKE

--Add .props file to correct project and config
premake.override(premake.vstudio.vc2010, "propertySheets", function(base, cfg)
    premake.push('<ImportGroup Label="PropertySheets" %s>', premake.vstudio.vc2010.condition(cfg))
	premake.w('<Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />')
    addPropsFiles(cfg)
	premake.pop('</ImportGroup>')
end)