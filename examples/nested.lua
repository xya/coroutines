function foo()
    print("entering foo")
    local co_bar = coroutine.create(bar)
    local co_baz = coroutine.create(baz)
    coroutine.resume(co_bar)
    coroutine.resume(co_baz, co_bar)
    print("leaving foo")
end

function bar()
    print("entering bar")
    coroutine.yield()
    print("leaving bar")
end

function baz(co_bar)
    print("entering baz")
    coroutine.resume(co_bar)
    print("leaving baz")
end

foo()