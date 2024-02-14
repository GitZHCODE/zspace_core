require('vstudio')

premake.api.register {
    name = "delayloaddlls",
    scope = "config",
    kind = "list:mixed",
    tokens = true,
}

local function AddDelayLoadDLLs(cfg)
    if cfg.delayloaddlls and #cfg.delayloaddlls  > 0 then
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