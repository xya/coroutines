function foo()
    print("in foo")
    local co_bar = coroutine.create(bar)
    local co_baz = coroutine.create(baz)
    print("foo: starting bar")
    coroutine.resume(co_bar)
    print("foo: starting baz")
    coroutine.resume(co_baz, co_bar)
    print("leaving foo")
end

function bar()
    print("in bar")
    print("bar: yield()")
    coroutine.yield()
    print("bar: yield returned")
    print("leaving bar")
end

function baz(co_bar)
    print("in baz")
    print("baz: resume(bar)")
    coroutine.resume(co_bar)
    print("baz: resume returned")
    print("leaving baz")
end

foo()