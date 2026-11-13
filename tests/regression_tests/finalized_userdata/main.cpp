#include <sol/sol.hpp>

#include <iostream>
#include <memory>

namespace {
struct Item {
    static int destroyed;
    int value = 7;
    ~Item() { ++destroyed; }
};
int Item::destroyed = 0;
}

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::debug);
    lua.new_usertype<Item>("Item", sol::no_constructor, "value", &Item::value);
    lua["make"] = [] { return std::make_shared<Item>(); };
    lua["consume"] = [](std::shared_ptr<Item> p) { return p->value; };
    lua["destroyed"] = [] { return Item::destroyed; };
    auto result = lua.safe_script(R"(
        local m = make()
        assert(m.value == 7)
        m.value = 9
        assert(consume(m) == 9)

        local mt = getmetatable(m)
        mt.__gc(m)
        assert(destroyed() == 1)
        assert(not pcall(function() return m.value end))
        assert(not pcall(function() m.value = 1 end))
        assert(not pcall(function() consume(m) end))
        mt.__gc(m)
        assert(destroyed() == 1) -- repeated finalization must be harmless
        m = nil
        collectgarbage('collect')
        collectgarbage('collect')
        assert(destroyed() == 1)

        local cache = setmetatable({}, {__mode = 'k'})
        for i = 1, 100 do cache[make()] = make() end
        collectgarbage('collect')
        -- Finalized keys may survive until the next GC cycle.
        for k, v in pairs(cache) do pcall(function() v.value = 2 end) end
        collectgarbage('collect')
        collectgarbage('collect')
        assert(destroyed() == 201)

        local live = make()
        live.value = 42
        assert(consume(live) == 42)
    )", sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error error = result;
        std::cerr << error.what() << "\n";
        return 1;
    }
    std::cout << "FINALIZED_USERDATA_PASS\n";
}
