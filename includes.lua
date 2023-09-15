function get_dependencies()
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
    return IncludeDir
end