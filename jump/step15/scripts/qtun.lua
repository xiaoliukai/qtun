qtun = {
    log = {
        syslog = function (level, fmt, ...)
            _syslog(level, string.format(fmt, ...))
        end
    }
}

function trim(s)
    return s:gsub('^%s*(.-)%s*$', '%1')
end

function is_numeric(s)
    return tonumber(s) ~= nil
end

function is_string(s)
    return not is_numeric(s)
end

function signature_load()
    local file = io.open(qtun.conf.signature)
    local ret = ''
    assert(file)
    if qtun.state.is_server then
        ret = file:read('*a')
    else
        ret = file:read(31)
    end
    file:close()
    return ret
end

function signature_verify(signature)
    if qtun.state.is_server then
        local file = io.open(qtun.conf.signature)
        assert(file)
        while true do
            local line = file:read()
            if line == nil then break end
            if line == signature then return true end
        end
        file:close()
        return false
    else
        return signature == signature_load(qtun.conf.signature)
    end
end

