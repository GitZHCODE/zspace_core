function get_include_dirs()
    local IncludeDir = {}
    IncludeDir["ALGLIB"] = "Dependencies/alglib/cpp/src"
    IncludeDir["ARMADILLO"] = "Dependencies/Armadillo"
    IncludeDir["EIGEN"] = "Dependencies/Eigen"
    IncludeDir["EIGEN_UNSUPPORTED"]= "Dependencies/Eigen_Unsupported/Eigen"
    IncludeDir["FREEGLUT"] = "Dependencies/freeglut"
    IncludeDir["LODEPNG"] = "Dependencies/lodePNG"
    IncludeDir["NLOHMANN"] = "Dependencies/nlohmann"
    IncludeDir["GLEW"] = "Dependencies/glew"
    IncludeDir["QUICKHULL"] = "Dependencies/quickhull"
    IncludeDir["SPA"] = "Dependencies/spa"
    IncludeDir["SPECTRA"] = "Dependencies/spectra/inlude"
    IncludeDir["SQLITE"] = "Dependencies/SQLITE"
    IncludeDir["TOOJPEG"] = "Dependencies/tooJPEG"
    IncludeDir["IGL"] = "Dependencies/igl/headers"
    IncludeDir["OMNIVERSE"] = "Dependencies/omniverse"
    IncludeDir["SRC"] = "src/headers"
    IncludeDir["DEPS"] = "Dependencies"
    return IncludeDir
end

function get_lib_dirs()
    local LibDir = {}
    LibDir["FREEGLUT"] = "Dependencies/freeglut"
    LibDir["GLEW"] = "Dependencies/glew/lib"
    LibDir["SQLITE"] = "Dependencies/SQLITE/lib"
    LibDir["OUTDLL"] = "bin/dll"
    LibDir["OUTLIB"] = "bin/lib"
    return LibDir
end