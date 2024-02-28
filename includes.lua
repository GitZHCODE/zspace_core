function get_include_dirs()
    local IncludeDir = {}
    IncludeDir["ARMADILLO"]         = "Dependencies/Armadillo"
    IncludeDir["EIGEN"]             = "Dependencies/Eigen"
    IncludeDir["EIGEN_UNSUPPORTED"] = "Dependencies/Eigen_Unsupported/Eigen"
    IncludeDir["FREEGLUT"]          = "Dependencies/freeglut"
    IncludeDir["LODEPNG"]           = "Dependencies/lodePNG"
    IncludeDir["NLOHMANN"]          = "Dependencies/nlohmann"
    IncludeDir["GLEW"]              = "Dependencies/glew"
    IncludeDir["QUICKHULL"]         = "Dependencies/quickhull"
    IncludeDir["SPA"]               = "Dependencies/spa"
    IncludeDir["SPECTRA"]           = "Dependencies/spectra/inlude"
    IncludeDir["SQLITE"]            = "Dependencies/SQLITE"
    IncludeDir["TOOJPEG"]           = "Dependencies/tooJPEG"
    IncludeDir["IGL"]               = "Dependencies/igl/headers"
    IncludeDir["OMNIVERSE"]         = "Dependencies/omniverse"
    IncludeDir["SRC"]               = "src/headers"
    IncludeDir["DEPS"]              = "Dependencies"

    --OV INCLUDES
    IncludeDir["OV_CLIENT"]         = "Dependencies/omniverse/omni_client_library/include"
    IncludeDir["OV_USD_RES"]        = "Dependencies/omniverse/omni_usd_resolver/include"
    IncludeDir["OV_PYTHON"]         = "Dependencies/omniverse/python/include"
    IncludeDir["OV_TINYTOML"]       = "Dependencies/omniverse/tinytoml/include"
    IncludeDir["OV_USD"]            = "Dependencies/omniverse/usd/release/include"

    return IncludeDir
end

function get_lib_dirs()
    local LibDir = {}
    LibDir["FREEGLUT"]        = "Dependencies/freeglut"
    LibDir["GLEW"]            = "Dependencies/glew/lib"
    LibDir["SQLITE"]          = "Dependencies/SQLITE/lib"
    LibDir["IGL"]             = "Dependencies/igl/build/lib"
    LibDir["OUTDLL"]          = "bin/dll"
    LibDir["OUTLIB"]          = "bin/lib"
    LibDir["DEBUG_OUTDLL"]    = "bin/dll/debug"
    LibDir["DEBUG_OUTLIB"]    = "bin/lib/debug"

    --OV LIBS
    LibDir["OV_CLIENT"]         = "Dependencies/omniverse/omni_client_library/release"
    LibDir["OV_USD_RES"]        = "Dependencies/omniverse/omni_usd_resolver/release"
    LibDir["OV_PYTHON"]         = "Dependencies/omniverse/python/libs"
    LibDir["OV_USD"]            = "Dependencies/omniverse/usd/release/lib"
    return LibDir
end

function get_omniverse_links()
    local omnilinks = {
    "omniclient.lib",
    "omni_usd_resolver.lib",
    "python310.lib",
    "boost_python310-vc142-mt-x64-1_76.lib",
    "ar.lib",
    "arch.lib",
    "gf.lib",
    "js.lib",
    "kind.lib",
    "pcp.lib",
    "plug.lib",
    "sdf.lib",
    "tf.lib",
    "trace.lib",
    "usd.lib",
    "usdGeom.lib",
    "usdLux.lib",
    "usdPhysics.lib",
    "usdShade.lib",
    "usdSkel.lib",
    "usdUtils.lib",
    "vt.lib",
    "work.lib",
    "shlwapi.lib",
    }
    return omnilinks
end