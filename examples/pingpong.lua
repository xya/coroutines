function ping(arg)
    local n = arg
    local ok = true
    local co_pong = coroutine.create(pong)
    repeat
        print("ping! " .. n)
        ok, n = coroutine.resume(co_pong, n + 1)
    until not ok or coroutine.status(co_pong) == 'dead'
end

function pong(arg)
    local n = arg
    while n < 10 do
        print("pong! " .. n)
        n = coroutine.yield(n + 1)
    end
end

ping(1)