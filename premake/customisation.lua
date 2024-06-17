require('vstudio')

--DELAY LOAD DLLS
premake.api.register {
    name = "delayloaddlls",
    scope = "config",
    kind = "list:string",
    tokens = true,
}

local function AddDelayLoadDLLs(cfg)
    if cfg.delayloaddlls and #cfg.delayloaddlls > 0 then
        local delayed = cfg.delayloaddlls
        premake.vstudio.vc2010.element("DelayLoadDLLs", nil, table.concat(delayed, ';'))
    end
end

premake.override(premake.vstudio.vc2010.elements, "link", function(base, prj)
    local calls = base(prj)
    table.insert(calls, AddDelayLoadDLLs)

    return calls
end)

--RAW FORCE INCLUDES
premake.api.register {
    name = "rawforceincludes",
    scope = "config",
    kind = "list:string",
    tokens = true,
}

local function AddRawForceIncludes(cfg)
    if cfg.rawforceincludes and #cfg.rawforceincludes > 0 then
        local forceincludes = cfg.rawforceincludes
        premake.vstudio.vc2010.element("ForcedIncludeFiles", nil, table.concat(forceincludes, ';'))
    end
end

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
    local calls = base(cfg)
    local index = table.indexof(calls, premake.vstudio.vc2010.forceIncludes)
    if index ~= nil then
        table.insertafter(calls, premake.vstudio.vc2010.forceIncludes, AddRawForceIncludes)
    end
    return calls
end)


-- FORCE SYMBOLS PATH
premake.api.register {
    name = "forcesymbolspath",
    scope = "config",
    kind = "boolean",
}

local function forceSymbolsPath(cfg)
    if cfg.forcesymbolspath ~= nil and cfg.forcesymbolspath and cfg.symbolspath then
        local value = premake.project.getrelative(cfg.project, cfg.symbolspath)
        premake.vstudio.vc2010.element("ProgramDataBaseFileName", nil, value)
    end
end

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
    local calls = base(cfg)
    local index = table.indexof(calls, premake.vstudio.vc2010.debugInformationFormat)
    if index ~= nil then
        table.insertafter(calls, premake.vstudio.vc2010.debugInformationFormat, forceSymbolsPath)
    end
    return calls
end)

-- FORCE DEBUG FORMAT
premake.api.register {
    name = "forcez7debugformat",
    scope = "config",
    kind = "boolean",
}

local function forceZ7DebugFormat(cfg)
    if cfg.forcez7debugformat ~= nil and cfg.forcez7debugformat then
        premake.vstudio.vc2010.element("DebugInformationFormat", nil, "OldStyle")
    end
end

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
    local calls = base(cfg)
    local index = table.indexof(calls, premake.vstudio.vc2010.debugInformationFormat)
    if index ~= nil then
        table.insertafter(calls, premake.vstudio.vc2010.debugInformationFormat, forceZ7DebugFormat)
    end
    return calls
end)

-- FORCE PRECOMPILED HEADER OUTPUT FILE
premake.api.register {
    name = "forceprecompiledheaderoutputfile",
    scope = "config",
    kind = "path",
    tokens = true,
}

local function forcePrecompiledHeaderOutputFile(cfg)
    if cfg.forceprecompiledheaderoutputfile ~= nil then
        local value = premake.project.getrelative(cfg.project, cfg.forceprecompiledheaderoutputfile)
        premake.vstudio.vc2010.element("PrecompiledHeaderOutputFile", nil, value)
    end
end

premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
    local calls = base(cfg)
    local index = table.indexof(calls, premake.vstudio.vc2010.debugInformationFormat)
    if index ~= nil then
        table.insertafter(calls, premake.vstudio.vc2010.debugInformationFormat, forcePrecompiledHeaderOutputFile)
    end
    return calls
end)
