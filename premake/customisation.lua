require('vstudio')

--DELAY LOAD DLLS
premake.api.register {
    name = "delayloaddlls",
    scope = "config",
    kind = "list:mixed",
    tokens = true,
}

local function AddDelayLoadDLLs(cfg)
    if cfg.delayloaddlls and #cfg.delayloaddlls > 0 then
        local delayed = cfg.delayloaddlls 
        premake.vstudio.vc2010.element("DelayLoadDLLs", condition, table.concat(delayed, ';'))
    end
end

premake.override(premake.vstudio.vc2010.elements, "link", function(base, prj)
    local calls = base(prj)
    local index = table.indexof(calls, premake.vstudio.vc2010.ignoreDefaultLibraries)
    if  index ~= nil then
        table.insertafter(calls, premake.vstudio.vc2010.ignoreDefaultLibraries, AddDelayLoadDLLs)
    end
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
        premake.vstudio.vc2010.element("ForcedIncludeFiles", condition, table.concat(forceincludes, ';'))
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