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
#include "libtorrent/file.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/hasher.hpp"
#include "libtorrent/file_pool.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>


#include "utils.h"

using namespace libtorrent;
using namespace boost::filesystem;

static int torrent_info_new(lua_State *L) {
    try {
	const char *filename = luaL_checkstring(L, 1);

	std::ifstream in(filename, std::ios_base::binary);
	in.unsetf(std::ios_base::skipws);

	entry e = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
	torrent_info **ti = (torrent_info **)lua_newuserdata(L, sizeof(torrent_info *));
	*ti = new torrent_info(e);

	luaL_getmetatable(L, "Torrent.Info");
	lua_setmetatable(L, -2);
    } catch (std::exception& e) {
	luaL_error(L, "%s", e.what());
	lua_pushnil(L);
    }

    return 1;
}

static int torrent_info_nodes(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_newtable(L);
    int c = 1; 

    typedef std::vector<std::pair<std::string, int> > node_vec;

    const node_vec &nodes = ti->nodes();

    for (node_vec::const_iterator i = nodes.begin(), end(nodes.end()); i != end; ++i) {
	lua_pushinteger(L, c);

	int d = 1;

	lua_newtable(L);
	LUA_PUSH_ARRAY_STRING(d, i->first.c_str());
	LUA_PUSH_ARRAY_INT(d, i->second);
	lua_settable(L, -3);

	c++;
    }

    return 1;
}

static int torrent_info_files(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_newtable(L);
    int c = 1; 
    for (torrent_info::file_iterator i = (*ti).begin_files(); i != (*ti).end_files(); ++i) {
	lua_pushinteger(L, c);

	lua_newtable(L);
	LUA_PUSH_ATTRIB_STRING("path", i->path.string().c_str());
	LUA_PUSH_ATTRIB_INT("size", i->size);
	LUA_PUSH_ATTRIB_INT("offset", i->offset);
	lua_settable(L, -3);

	c++;
    }

    return 1;
}

static int torrent_info_trackers(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_newtable(L);
    int c = 1; 
    for (std::vector<announce_entry>::const_iterator i = (*ti).trackers().begin(); i != (*ti).trackers().end(); ++i) {
	lua_pushinteger(L, c);

	lua_newtable(L);
	LUA_PUSH_ATTRIB_STRING("url", i->url.c_str());
	LUA_PUSH_ATTRIB_INT("tier", i->tier);
	lua_settable(L, -3);

	c++;
    }

    return 1;
}

static int torrent_info_tracker_urls(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    int c = 1;
    lua_newtable(L);
    for (std::vector<announce_entry>::const_iterator i = (*ti).trackers().begin(); i != (*ti).trackers().end(); ++i) {
        LUA_PUSH_ARRAY_STRING(c, i->url.c_str());
    }

    return 1;
}

static int torrent_info_filenames(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_newtable(L);
    int c = 1; 
    for (torrent_info::file_iterator i = (*ti).begin_files(); i != (*ti).end_files(); ++i) {
	LUA_PUSH_ARRAY_STRING(c, i->path.string().c_str());
    }

    return 1;
}



static int torrent_info_creator(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushstring(L, ti->creator().c_str());

    return 1;
}

static int torrent_info_comment(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushstring(L, ti->comment().c_str());

    return 1;
}

static int torrent_info_num_files(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushinteger(L, ti->num_files());

    return 1;
}

static int torrent_info_file_at(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    int index = (int)lua_tonumber(L, 2);

    file_entry fe = ti->file_at(index-1);

    lua_newtable(L);
    LUA_PUSH_ATTRIB_STRING("path", fe.path.string().c_str());
    LUA_PUSH_ATTRIB_INT("size", fe.size);
    LUA_PUSH_ATTRIB_INT("offset", fe.offset);

    return 1;
}

static int torrent_info_total_size(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushnumber(L, ti->total_size());

    return 1;
}

static int torrent_info_piece_length(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushinteger(L, ti->piece_length());

    return 1;
}

static int torrent_info_num_pieces(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushinteger(L, ti->num_pieces());

    return 1;
}

static int torrent_info_name(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushstring(L, ti->name().c_str());

    return 1;
}

static int torrent_info_is_valid(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushboolean(L, ti->is_valid());

    return 1;
}

static int torrent_info_priv(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    lua_pushboolean(L, ti->priv());

    return 1;
}

static int torrent_info_set_priv(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    bool priv = (bool)lua_toboolean(L, 2);

    ti->set_priv(priv);

    return 0;
}

static int torrent_info_convert_file_names(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    torrent_info *ti = *((torrent_info **)ud);

    ti->convert_file_names();

    return 0;
}

static int torrent_info_piece_size(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    int index = (int)lua_tonumber(L, 2);

    int ps = ti->piece_size(index);

    lua_pushinteger(L, ps);

    return 1;
}

static int torrent_info_set_comment(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    ti->set_comment(luaL_checkstring(L, 2));

    return 0;
}

static int torrent_info_set_creator(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    ti->set_creator(luaL_checkstring(L, 2));

    return 0;
}

static int torrent_info_set_piece_size(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    ti->set_piece_size(luaL_checkinteger(L, 2));

    return 0;
}

static int torrent_info_set_hash(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");

    luaL_error(L, "Not implemented");

    return 0;
}

static int torrent_info_add_tracker(lua_State *L) {
    int n = lua_gettop(L);
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    if (n == 3)
	ti->add_tracker(std::string(luaL_checkstring(L, 2)), luaL_checkinteger(L, 3));
    else
	ti->add_tracker(std::string(luaL_checkstring(L, 2)));

    return 0;
}

static int torrent_info_add_file(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    boost::filesystem::path fp(luaL_checkstring(L, 2));

    ti->add_file(fp, file_size(fp));

    return 0;
}

static int torrent_info_add_url_seed(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    ti->add_url_seed(std::string(luaL_checkstring(L, 2)));

    return 0;
}

static int torrent_info_url_seeds(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    const std::vector<std::string> &url_seeds = ti->url_seeds();

    lua_newtable(L);
    int c = 1;
    for (std::vector<std::string>::const_iterator i = url_seeds.begin(), end(url_seeds.end()); i != end; ++i) {
	LUA_PUSH_ARRAY_STRING(c, (*i).c_str());
    } 

    return 1;
}

static int torrent_info_info_hash(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    const sha1_hash hash = ti->info_hash();

    std::ostringstream out;
    out << std::hex;
    for (sha1_hash::const_iterator i = hash.begin(); i != hash.end(); ++i) {
	out << std::setw(2) << std::setfill('0') << (int)*i;
    }

    lua_pushstring(L, out.str().c_str());

    return 1;
}


static int torrent_info_save_to_file(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Info");
    torrent_info *ti = *((torrent_info **)ud);

    const char *filepath = luaL_checkstring(L, 2);

    ofstream out(complete(filepath), std::ios_base::binary);

    entry e = ti->create_torrent();
    libtorrent::bencode(std::ostream_iterator<char>(out), e);

    return 0;
}

static int torrent_info_gc(lua_State *L) {
    return 0;
}

static const luaL_Reg torrent_info_methods[] = {
    {"nodes", torrent_info_nodes},
    {"files", torrent_info_files},
    {"trackers", torrent_info_trackers},
    {"creator", torrent_info_creator},
    {"comment", torrent_info_comment},
    {"num_files", torrent_info_num_files},
    {"file_at", torrent_info_file_at},
    {"total_size", torrent_info_total_size},
    {"piece_length", torrent_info_piece_length},
    {"num_pieces", torrent_info_num_pieces},
    {"name", torrent_info_name},
    {"is_valid", torrent_info_is_valid},
    {"priv", torrent_info_priv},
    {"set_priv", torrent_info_set_priv},
    {"convert_file_names", torrent_info_convert_file_names},
    {"piece_size", torrent_info_piece_size}, 
    
    {"set_comment", torrent_info_set_comment},
    {"set_creator", torrent_info_set_creator},
    {"set_piece_size", torrent_info_set_piece_size},
    {"set_hash", torrent_info_set_hash},
    {"add_tracker", torrent_info_add_tracker},
    {"add_file", torrent_info_add_file},
    {"add_url_seed", torrent_info_add_url_seed},
    {"info_hash", torrent_info_info_hash},

    {"url_seeds", torrent_info_url_seeds},

    //luatorrent specific helper stuff
    {"save_to_file", torrent_info_save_to_file},

    {"tracker_urls", torrent_info_tracker_urls},
    {"filenames", torrent_info_filenames},

    {NULL, NULL}
};

static const luaL_Reg torrent_info_class_methods[] = {
    {"New", torrent_info_new},
    {NULL, NULL}
};

int torrent_info_register(lua_State *L) {
    luaL_newmetatable(L, "Torrent.Info");
    luaL_register(L, 0, torrent_info_methods);  
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");    

    lua_pushcfunction(L, torrent_info_gc);
    lua_setfield(L, -2, "__gc");

    luaL_register(L, "Torrent.Info", torrent_info_class_methods);  

    return 1;
}
