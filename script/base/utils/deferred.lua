--[[
    Futures/Promises implementation based on dojo.Deferred
]]

local deferred_resolve,
      deferred_reject,
      deferred_next,
      deferred_flush_queue,
      forward_handler,
      bind,
      is_deferred

function Deferred()
    
    local object = {}
    object._queue = {}
    object._finished = false
    
    local deferred_methods = {
        resolve = deferred_resolve,
        reject = deferred_reject,
        next = deferred_next
    }
    
    setmetatable(object, {
        __index = deferred_methods,
        _is_deferred_object = true
    })
    
    return object
end

function deferred_resolve(self, value)
    
    assert(not self._finished)
    
    self._result = value
    self._finished = true
    self._get_callback = function(deferred) return deferred._resolve_handler end
    self._chain_method = deferred_resolve
    
    deferred_flush_queue(self)
end

function deferred_reject(self, value)

    assert(not self._finished)
    
    self._result = value
    self._finished = true
    self._get_callback = function(deferred) return deferred._reject_handler end
    self._chain_method = deferred_reject
    
    deferred_flush_queue(self)
end

function deferred_next(self, resolve_handler, reject_handler)
    
    local next_deferred = Deferred()
    
    next_deferred._resolve_handler = resolve_handler or forward_handler
    next_deferred._reject_handler = reject_handler or forward_handler
    
    self._queue[#self._queue + 1] = next_deferred
    
    deferred_flush_queue(self)
    
    setmetatable(next_deferred, {
        __index = {next = deferred_next},
        _is_deferred_object = true
    })
    
    return next_deferred
end

function deferred_flush_queue(self)
    
    if not self._finished then
        return
    end
    
    while #self._queue > 0 do
        
        local next_deferred = self._queue[1]
        
        local result = self._get_callback(next_deferred)(self._result)
        
        if is_deferred(result) then
            result:next(bind(deferred_resolve, next_deferred), bind(deferred_reject, next_deferred))
        else
            self._chain_method(next_deferred, result)
        end
        
        table.remove(self._queue, 1)
    end
    
end

function forward_handler(...)
    return table.unpack({...})
end

function bind(method, object)
    return function(value)
        method(object, value)
    end
end

function is_deferred(deferred)
    local mt = getmetatable(deferred)
    return mt and mt._is_deferred_object
end

function run_deferred_test_cases()
    
    local function add_call_history(history, mutator)
        return function(result)
            history[#history + 1] = result
            local return_value = result
            if mutator then
                return_value = mutator(result)
            end
            return return_value
        end
    end
    
    local function add(value)
        return function(result)
            return result + value
        end
    end
    
    local function assert_equal_to(value)
        return function(result)
            assert(result == value)
            return result
        end
    end
    
    local function wrong_callback()
        error("Wrong callback called")
    end
    
    local function test_pre_resolve()
        local call_history = {}
        local d = Deferred()
        d:resolve(123)
        d:next(add_call_history(call_history), wrong_callback)
        assert(call_history[1] == 123)
    end
    
    local function test_post_resolve()
        local call_history = {}
        local d = Deferred()
        d:next(add_call_history(call_history), wrong_callback)
        assert(#call_history == 0)
        d:resolve(123)
        assert(call_history[1] == 123)
    end
    
    local function test_pre_reject()
        local call_history = {}
        local d = Deferred()
        d:reject(123)
        d:next(wrong_callback, add_call_history(call_history))
        assert(call_history[1] == 123)
    end
    
    local function test_post_reject()
        local call_history = {}
        local d = Deferred()
        d:next(wrong_callback, add_call_history(call_history))
        assert(#call_history == 0)
        d:reject(123)
        assert(call_history[1] == 123)
    end
    
    local function test_pre_resolve_chain()
        local call_history = {}
        local d = Deferred()
        d:resolve(123)
        
        d:next(add_call_history(call_history, add(1)), wrong_callback)
         :next(add_call_history(call_history, add(1)), wrong_callback)
         :next(add_call_history(call_history), wrong_callback)
         
        assert(call_history[1] == 123)
        assert(call_history[2] == 124)
        assert(call_history[3] == 125)
    end
    
    local function test_post_resolve_chain()
        local call_history = {}
        local d = Deferred()
        
        d:next(add_call_history(call_history, add(1)), wrong_callback)
         :next(add_call_history(call_history, add(1)), wrong_callback)
         :next(add_call_history(call_history), wrong_callback)
        
        assert(#call_history == 0)
        
        d:resolve(123)
        
        assert(call_history[1] == 123)
        assert(call_history[2] == 124)
        assert(call_history[3] == 125)
    end
    
    local function test_pre_reject_chain()
        local call_history = {}
        local d = Deferred()
        d:reject(123)
        
        d:next(wrong_callback, add_call_history(call_history, add(1)))
         :next(wrong_callback, add_call_history(call_history, add(1)))
         :next(wrong_callback, add_call_history(call_history))
         
        assert(call_history[1] == 123)
        assert(call_history[2] == 124)
        assert(call_history[3] == 125)
    end
    
    local function test_post_reject_chain()
        local call_history = {}
        local d = Deferred()
        
        d:next(wrong_callback, add_call_history(call_history, add(1)))
         :next(wrong_callback, add_call_history(call_history, add(1)))
         :next(wrong_callback, add_call_history(call_history))
        
        assert(#call_history == 0)
        
        d:reject(123)
        
        assert(call_history[1] == 123)
        assert(call_history[2] == 124)
        assert(call_history[3] == 125)
    end
    
    local function test_deferred_resolve_handler()
        
        local call_history = {}
        local d = Deferred()
        
        local sub_deferred = Deferred()
        
        d:next(function() return sub_deferred end)
         :next(add_call_history(call_history))
        
        d:resolve()
        
        assert(#call_history == 0)
        
        sub_deferred:resolve(123)
        
        assert(call_history[1] == 123)
    end
    
    local function test_deferred_reject_handler()
        
        local call_history = {}
        local d = Deferred()
        
        local sub_deferred = Deferred()
        
        d:next(nil, function() return sub_deferred end)
         :next(nil, add_call_history(call_history))
        
        d:reject()
        
        assert(#call_history == 0)
        
        sub_deferred:reject(123)
        
        assert(call_history[1] == 123)
    end
    
    local function test_reject_deferred_to_resolve()
        
        local call_history = {}
        local d = Deferred()
        
        local sub_deferred = Deferred()
        
        d:next(nil, function() return sub_deferred end)
         :next(add_call_history(call_history), wrong_callback)
        
        d:reject()
        
        sub_deferred:resolve(123)
        
        assert(call_history[1] == 123)
    end
    
    local function test_read_only_next()
        local d = Deferred()
        local d2 = d:next()
        assert(d2.next)
        assert(not d2.resolve)
        assert(not d2.reject)
    end
    
    test_pre_resolve()
    test_post_resolve()
    test_pre_reject()
    test_post_reject()
    test_pre_resolve_chain()
    test_post_resolve_chain()
    test_pre_reject_chain()
    test_post_reject_chain()
    test_deferred_resolve_handler()
    test_deferred_reject_handler()
    test_reject_deferred_to_resolve()
    test_read_only_next()
end

