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


extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if !defined(LUA_VERSION_NUM) || (LUA_VERSION_NUM < 501)
#include <compat-5.1.h>
#endif
};

#include <iostream>
#include <fstream>
#include <iterator>
#include <iomanip>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/session.hpp"

#include "utils.h"

using namespace libtorrent;

/*
 * session = Torrent.Session.New([first_port, last_port])
 *
 *   creates a new session object on the first available port between
 *   first_port and last_port inclusive
 */
static int torrent_session_new(lua_State *L) {
    int n = lua_gettop(L);

    try {
	session **s = (session **)lua_newuserdata(L, sizeof(session *));
	*s = new session();

	if (n == 2) {
	    int first = luaL_checkinteger(L, 1);
	    int last = luaL_checkinteger(L, 2);

	    (*s)->listen_on(std::make_pair(first, last));
	}

	luaL_getmetatable(L, "Torrent.Session");
	lua_setmetatable(L, -2);
    } catch (std::exception& e) {
	luaL_error(L, "%s", e.what());
	lua_pushnil(L);
    }

    return 1;
}

/*
 * session:abort()
 *
 *   destruct the session asynchronously
 */
static int torrent_session_abort(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    s->abort();

    return 0;
}

/*
 * torrent_handle = session:add_torrent(torrent_info, [save_path, resume_data_file])
 *
 *   adds the torrent described by torrent_info to this session, to the save_path (default cwd)
 *   with fast resume file resume_data_file  
 */
static int torrent_session_add_torrent(lua_State *L) {
    int n = lua_gettop(L);
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    ud = luaL_checkudata(L, 2, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    boost::intrusive_ptr<torrent_info> t(ti);

    try {
	torrent_handle th;

	if (n >= 3 && !lua_isnil(L, 3)) {
	    const char *path = "./";

	    if (!lua_isnil(L, 3)) {
		path = luaL_checkstring(L, 3);
	    }
	    
	    if (n == 4 && !lua_isnil(L, 4)) {
		const char *filename = luaL_checkstring(L, 4);

		std::ifstream in(filename, std::ios_base::binary);
		in.unsetf(std::ios_base::skipws);

		entry e = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());

		th = s->add_torrent(t, path, e);
	    } else {
		th = s->add_torrent(t, path);
	    }
	} else {
	    th = s->add_torrent(t, "./");
	}

	torrent_handle **h = (torrent_handle **)lua_newuserdata(L, sizeof(torrent_handle *));
	*h = new torrent_handle(th);

	luaL_getmetatable(L, "Torrent.Handle");
	lua_setmetatable(L, -2);
    } catch (std::exception& e) {
	luaL_error(L, "%s", e.what());
	lua_pushnil(L);
    }

    return 1;
}

/*
 * handles = session:torrent_handles()
 *
 *   returns an tables of handles associated with this session
 */
static int torrent_session_torrent_handles(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int c = 1;
    lua_newtable(L);

    std::vector<torrent_handle> handles = (*s).get_torrents();

    for (std::vector<torrent_handle>::const_iterator i = handles.begin(); i != handles.end(); ++i) {
	lua_pushinteger(L, c);

	torrent_handle **h = (torrent_handle **)lua_newuserdata(L, sizeof(torrent_handle *));
	*h = new torrent_handle(*i);

	luaL_getmetatable(L, "Torrent.Handle");
	lua_setmetatable(L, -2);

        lua_settable(L, -3);
        c++;
    }

    return 1;
}

/*
 * status = session:status()
 *
 *   returns a table describing the current session 
 */
static int torrent_session_status(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    session_status status = s->status();

    lua_newtable(L);

    LUA_PUSH_ATTRIB_BOOL("has_incoming_connections", status.has_incoming_connections);
    LUA_PUSH_ATTRIB_FLOAT("upload_rate", status.upload_rate);
    LUA_PUSH_ATTRIB_FLOAT("download_rate", status.download_rate);
    LUA_PUSH_ATTRIB_FLOAT("payload_upload_rate", status.payload_upload_rate);
    LUA_PUSH_ATTRIB_FLOAT("payload_download_rate", status.payload_download_rate);
    LUA_PUSH_ATTRIB_INT("total_download", status.total_download);
    LUA_PUSH_ATTRIB_INT("total_upload", status.total_upload);
    LUA_PUSH_ATTRIB_INT("total_payload_download", status.total_payload_download);
    LUA_PUSH_ATTRIB_INT("total_payload_upload", status.total_payload_upload);
    LUA_PUSH_ATTRIB_INT("num_peers", status.num_peers);

    return 1;
}

/*
 * bool = session:is_listening()
 *
 *   returns true if the session has successfully opened a listing port,
 *   otherwise false
 */
static int torrent_session_is_listening(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    lua_pushboolean(L, s->is_listening());

    return 1;
}

/*
 * port_num = session:listen_port()
 *
 *   returns the current listening port
 */
static int torrent_session_listen_port(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    lua_pushinteger(L, s->listen_port());

    return 1;
}

/*
 * count = session:num_uploads()
 *
 *   returns the number of currently unchoked peers 
 */
static int torrent_session_num_uploads(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    lua_pushinteger(L, s->num_uploads());

    return 1;
}

/*
 * count = session:num_connections()
 *
 *   returns the number of connections, including half-open ones
 */
static int torrent_session_num_connections(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    lua_pushinteger(L, s->num_connections());

    return 1;
}

/*
 * session:remove_torrent(torrent_handle)
 *
 *   removes the torrent in by torrent_handle from the session
 */
static int torrent_session_remove_torrent(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    ud = luaL_checkudata(L, 2, "Torrent.Handle");
    torrent_handle *th = *((torrent_handle **)ud);

    try {
	s->remove_torrent(*th);
    } catch (std::exception& e) {
        luaL_error(L, "%s", e.what());
    }


    return 0;
}

/*
 * bytes_per_second = session:upload_rate_limit()
 *
 *   returns the current session upload rate limit
 */
static int torrent_session_upload_rate_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int url = s->upload_rate_limit();
    lua_pushinteger(L, url);

    return 1;
}

/*
 * bytes_per_second = session:download_rate_limit()
 *
 *   returns the current session download rate limit
 */
static int torrent_session_download_rate_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int drl = s->download_rate_limit();

    lua_pushinteger(L, drl);

    return 1;
}

/*
 * session:set_upload_rate_limit(bytes_per_second)
 *
 *   sets the maximum number of bytes allowed to be sent to peers per second
 */
static int torrent_session_set_upload_rate_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int bytes_per_second = luaL_checkinteger(L, 2);
    s->set_upload_rate_limit(bytes_per_second);

    return 0;
}

/*
 * session:set_download_rate_limit(bytes_per_second)
 *
 *   sets the maximum number of bytes allowed to be downloaded from peers per second
 */
static int torrent_session_set_download_rate_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int bytes_per_second = luaL_checkinteger(L, 2);
    s->set_download_rate_limit(bytes_per_second);

    return 0;
}

/*
 * session:set_max_uploads(max)
 *
 *   sets a limit on the number of unchoked peers (uploads)
 *   for this session
 */
static int torrent_session_set_max_uploads(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int limit = luaL_checkinteger(L, 2);
    s->set_max_uploads(limit);

    return 0;
}

/*
 * session:set_max_connections(max)
 *
 *   sets the maximum number of connections this session will 
 *   have when connecting to peers
 */
static int torrent_session_set_max_connections(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int limit = luaL_checkinteger(L, 2);
    s->set_max_connections(limit);

    return 0;
}

/*
 * session:set_max_half_open_connections(max)
 *
 *   sets the maximum number of half-open connections this session will 
 *   have when connecting to peers
 */
static int torrent_session_set_max_half_open_connections(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int limit = luaL_checkinteger(L, 2);
    s->set_max_half_open_connections(limit);

    return 0;
}

/*
 * session:set_key(int)
 */
static int torrent_session_set_key(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int key = luaL_checkinteger(L, 2);
    s->set_key(key);

    return 0;
}

/*
 * success = session:listen_on(first_port, last_port)
 *
 *   changes the current listening port range to first_port to last_port
 */
static int torrent_session_listen_on(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    int start = luaL_checkinteger(L, 2);
    int end = luaL_checkinteger(L, 2);

    lua_pushboolean(L, s->listen_on(std::make_pair(start, end)));

    return 1;
}

/*
 * __gc
 *
 *   frees session on GC
 */
static int torrent_session_gc(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Session");
    session *s = *((session **)ud);

    delete s;

    return 0;
}

static const luaL_Reg torrent_session_methods[] = {
    {"abort", torrent_session_abort},
    {"add_torrent", torrent_session_add_torrent},
    {"torrent_handles", torrent_session_torrent_handles},
    {"status", torrent_session_status},
    {"is_listening", torrent_session_is_listening},
    {"listen_port", torrent_session_listen_port},
    {"num_uploads", torrent_session_num_uploads},
    {"num_connections", torrent_session_num_connections},
    {"remove_torrent", torrent_session_remove_torrent},
    {"upload_rate_limit", torrent_session_upload_rate_limit},
    {"download_rate_limit", torrent_session_download_rate_limit},
    {"set_upload_rate_limit", torrent_session_set_upload_rate_limit},
    {"set_download_rate_limit", torrent_session_set_download_rate_limit},
    {"set_max_uploads", torrent_session_set_max_uploads},
    {"set_max_connections", torrent_session_set_max_connections},
    {"set_max_half_open_connections", torrent_session_set_max_half_open_connections},
    {"set_key", torrent_session_set_key},
    {"listen_on", torrent_session_listen_on},
    {NULL, NULL}
};

static const luaL_Reg torrent_session_class_methods[] = {
    {"New", torrent_session_new},
    {NULL, NULL}
};

int torrent_session_register(lua_State *L) {
    luaL_newmetatable(L, "Torrent.Session");
    luaL_register(L, 0, torrent_session_methods);  
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");    

    lua_pushcfunction(L, torrent_session_gc);
    lua_setfield(L, -2, "__gc"); 

    luaL_register(L, "Torrent.Session", torrent_session_class_methods);  

    return 1;
}

