int lua_user_list(lua_State * L)
{
    const char *desc = luaL_checkstring(L,1);
    lua_newtable(L);

    enumerate(users, userinfo, u,
        if(!strcmp(u.desc, desc)) {
            lua_pushstring(L, u.name);
            lua_newtable(L);

            lua_pushstring(L, "pubkey");
            lua_pushstring(L, u.pubkey);
            lua_settable(L, -3);

            lua_pushstring(L, "privilege");
            lua_pushstring(L, u.privilege);
            lua_settable(L, -3);

            lua_settable(L, -3);
        })

    return 1;
}
