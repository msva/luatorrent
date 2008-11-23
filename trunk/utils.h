/*
 * Copyright (c) 2007,2008 Neil Richardson (nrich@iinet.net.au)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */


#define LUA_PUSH_ATTRIB_INT(n, v) \
    lua_pushstring(L, n); \
    lua_pushinteger(L, v); \
    lua_settable(L, -3); 

#define LUA_PUSH_ATTRIB_FLOAT(n, v) \
    lua_pushstring(L, n); \
    lua_pushnumber(L, v); \
    lua_settable(L, -3); 

#define LUA_PUSH_ATTRIB_STRING(n, v) \
    lua_pushstring(L, n); \
    lua_pushstring(L, v); \
    lua_settable(L, -3); 

#define LUA_PUSH_ATTRIB_BOOL(n, v) \
    lua_pushstring(L, n); \
    lua_pushboolean(L, v); \
    lua_settable(L, -3); 




#define LUA_PUSH_ARRAY_INT(n, v) \
    lua_pushinteger(L, n); \
    lua_pushinteger(L, v); \
    lua_settable(L, -3); \
    n++; 

#define LUA_PUSH_ARRAY_FLOAT(n, v) \
    lua_pushinteger(L, n); \
    lua_pushnumber(L, v); \
    lua_settable(L, -3); \
    n++; 

#define LUA_PUSH_ARRAY_STRING(n, v) \
    lua_pushinteger(L, n); \
    lua_pushstring(L, v); \
    lua_settable(L, -3); \
    n++;

#define LUA_PUSH_ARRAY_BOOL(n, v) \
    lua_pushinteger(L, n); \
    lua_pushboolean(L, v); \
    lua_settable(L, -3); \
    n++;


#ifdef _WIN32
    #define LT_EXPORT __declspec(dllexport)
#else
    #define LT_EXPORT
#endif
