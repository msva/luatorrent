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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/session.hpp"

#include "utils.h"

using namespace libtorrent;
using namespace boost::filesystem;

/*
 * status_table = handle:status()
 *
 *  return a table of status information
 *  about this torrent
 */
static int torrent_handle_status(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    torrent_status status = h->status();

    lua_newtable(L);

    LUA_PUSH_ATTRIB_INT("state", status.state);
    LUA_PUSH_ATTRIB_BOOL("paused", status.paused);
    LUA_PUSH_ATTRIB_FLOAT("progress", status.progress);
    //next_announce
    //announce_interval
    LUA_PUSH_ATTRIB_STRING("current_tracker", status.current_tracker.c_str());
    LUA_PUSH_ATTRIB_FLOAT("total_download", status.total_download);
    LUA_PUSH_ATTRIB_FLOAT("total_upload", status.total_upload);
    LUA_PUSH_ATTRIB_INT("total_payload_download", status.total_payload_download);
    LUA_PUSH_ATTRIB_INT("total_payload_upload", status.total_payload_upload);
    LUA_PUSH_ATTRIB_FLOAT("total_failed_bytes", status.total_failed_bytes);
    LUA_PUSH_ATTRIB_FLOAT("total_redundant_bytes", status.total_redundant_bytes);
    LUA_PUSH_ATTRIB_FLOAT("download_rate", status.download_rate);
    LUA_PUSH_ATTRIB_FLOAT("upload_rate", status.upload_rate);
    LUA_PUSH_ATTRIB_FLOAT("download_payload_rate", status.download_payload_rate);
    LUA_PUSH_ATTRIB_FLOAT("upload_payload_rate", status.upload_payload_rate);
    LUA_PUSH_ATTRIB_INT("num_peers", status.num_peers);
    LUA_PUSH_ATTRIB_INT("num_complete", status.num_complete);
    LUA_PUSH_ATTRIB_INT("num_incomplete", status.num_incomplete);

    const std::vector<bool> *pieces = status.pieces;
    lua_pushstring(L, "pieces");
    lua_newtable(L);

    if (pieces) {
	int d = 1;
	for (std::vector<bool>::const_iterator b = pieces->begin(); b != pieces->end(); ++b) {
	    LUA_PUSH_ARRAY_BOOL(d, *b);
	}
    }

    lua_settable(L, -3);
   
    LUA_PUSH_ATTRIB_INT("num_pieces", status.num_pieces);
    LUA_PUSH_ATTRIB_INT("total_done", status.total_done);
    LUA_PUSH_ATTRIB_INT("total_wanted_done", status.total_wanted_done);
    LUA_PUSH_ATTRIB_INT("num_seeds", status.num_seeds);
    LUA_PUSH_ATTRIB_FLOAT("distributed_copies", status.distributed_copies);
    LUA_PUSH_ATTRIB_INT("block_size", status.block_size);

    return 1;
}

/*
 * bool = handle:is_seed()
 *
 *  return true if this torrent is a seed (i.e. has finished downloading)
 *  otherwise false
 */
static int torrent_handle_is_seed(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushboolean(L, h->is_seed());

    return 1;
}

/*
 * bool = handle:is_paused()
 *
 *  returns true if this torrent is paused
 *  otherwise false
 */
static int torrent_handle_is_paused(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushboolean(L, h->is_paused());

    return 1;
}

/*
 * handle:pause()
 *
 *  unconditionally pauses this torrent
 */
static int torrent_handle_pause(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    h->pause();

    return 0;
}

/*
 * handle:resume()
 *
 *  unconditionally resumes (unpauses) this torrent.
 *  the opposite of handle:pause()
 */
static int torrent_handle_resume(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    h->resume();

    return 0;
}

/*
 * handle:force_reannounce()
 *
 *  forces this torrent to do another tracker request to receive new peers
 */
static int torrent_handle_force_reannounce(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    h->force_reannounce();

    return 0;
}

/*
 * str = handle:name()
 *  returns the name of this torrent
 */
static int torrent_handle_name(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushstring(L, h->name().c_str());

    return 1;
}

/*
 * handle:set_upload_limit(bytes_per_second)
 *
 *  limit the upload bandwidth used by this torrent to bytes_per_second
 */
static int torrent_handle_set_upload_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    int limit = luaL_checkint(L, 2);

    h->set_upload_limit(limit);

    return 0;
}

/*
 * handle:set_download_limit(bytes_per_second)
 *
 *  limit the download bandwidth used by this torrent to bytes_per_second
 */
static int torrent_handle_set_download_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    int limit = luaL_checkint(L, 2);

    h->set_download_limit(limit);

    return 0;
}

/*
 * handle:set_sequenced_download_threshold(int)
 *
 *  missing from libtorrent docs, deprecated?
 */
static int torrent_handle_set_sequenced_download_threshold(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    int threshold = (int)lua_tonumber(L, 2);

    h->set_sequenced_download_threshold(threshold);

    return 0;
}

/*
 * handle:set_ratio(ratio)
 *
 *  sets the desired download / upload ratio. If set to 0, it is considered being infinite
 */
static int torrent_handle_set_ratio(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    float up_down_ratio = (float)lua_tonumber(L, 2);

    h->set_ratio(up_down_ratio);

    return 0;
}

/*
 * path_str = handle:save_path()
 *
 *  returns the save path of this torrent
 */
static int torrent_handle_save_path(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    boost::filesystem::path path = h->save_path();

    lua_pushstring(L, path.string().c_str());

    return 1;
}

/*
 * handle:set_max_uploads(max_upload_peers)
 *
 *  sets the maximum number of peers unchoked at the same time on this torrent. 
 *  If you set this to -1, there will be no limit.
 */
static int torrent_handle_set_max_uploads(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    int max_uploads = luaL_checkint(L, 2);

    h->set_max_uploads(max_uploads);

    return 0;
}

/*
 * handle:set_max_connections(max_connections)
 *
 *  sets the maximum number of connection this torrent will open. 
 *  If all connections are used up, incoming connections may be refused 
 *  or poor connections may be closed. This must be at least 2. 
 *  The default is unlimited number of connections. 
 *  If -1 is given to the function, it means unlimited.
 */
static int torrent_handle_set_max_connections(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    int max_connections = luaL_checkint(L, 2);

    h->set_max_connections(max_connections);


    return 0;
}

/*
 * handle:set_tracker_login(username, password)
 *
 *  sets a username and password that will be sent along in 
 *  the HTTP-request of the tracker announce.
 */
static int torrent_handle_set_tracker_login(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    const char *username = lua_tostring(L, 2);
    const char *password = lua_tostring(L, 3);

    h->set_tracker_login(std::string(username), std::string(password));

    return 0;
}

/*
 * bool = handle:has_metadata()
 *
 *  returns true if this torrent has metadata
 */
static int torrent_handle_has_metadata(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushboolean(L, h->has_metadata());

    return 1;
}

/*
 * info = handle:get_torrent_info()
 *
 *  returns the torrent_info object associated with this torrent
 */
static int torrent_handle_get_torrent_info(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    const torrent_info **ti = (const torrent_info **)lua_newuserdata(L, sizeof(torrent_info *));
    const torrent_info &info = h->get_torrent_info();

    *ti = &info;

    luaL_getmetatable(L, "Torrent.Info");
    lua_setmetatable(L, -2);

    return 1;
}

/*
 * bool = handle:is_valid()
 *
 *  returns true if this handle refers to a valid torrent, 
 *  or false if it hasn't been initialized or if the torrent it refers to has been aborted.
 */
static int torrent_handle_is_valid(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushboolean(L, h->is_valid());

    return 1;
}

/*
 * files = handle:file_progress()
 *
 *  returns a numerically indexed table with the the number of 
 *  bytes downloaded of each file in this torrent.
 */
static int torrent_handle_file_progress(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    std::vector<float> progress;

    h->file_progress(progress);

    int c = 1;
    lua_newtable(L);
    for (std::vector<float>::const_iterator i = progress.begin(); i != progress.end(); ++i) {
	LUA_PUSH_ARRAY_FLOAT(c, *i);
    }

    return 1;
}

/*
 * peers = handle:get_peer_info()
 *
 *  returns a numerically indexed table with peer information
 *  for each peer connected to this torrent
 */
static int torrent_handle_get_peer_info(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    std::vector<peer_info> peers;

    h->get_peer_info(peers);

    int c = 1;
    lua_newtable(L);
    for (std::vector<peer_info>::const_iterator i = peers.begin(); i != peers.end(); ++i) {
        lua_pushinteger(L, c);

        lua_newtable(L);
        LUA_PUSH_ATTRIB_INT("flags", i->flags);
	
	std::ostringstream out;
	out << i->ip;
	LUA_PUSH_ATTRIB_STRING("ip", out.str().c_str());

        LUA_PUSH_ATTRIB_FLOAT("up_speed", i->up_speed);
        LUA_PUSH_ATTRIB_FLOAT("down_speed", i->down_speed);
        LUA_PUSH_ATTRIB_FLOAT("payload_up_speed", i->payload_up_speed);
        LUA_PUSH_ATTRIB_FLOAT("payload_down_speed", i->payload_down_speed);
        LUA_PUSH_ATTRIB_INT("total_download", i->total_download);
        LUA_PUSH_ATTRIB_INT("total_upload", i->total_upload);
	//pid
	//pieces
	
	std::vector<bool> pieces = i->pieces;
	lua_pushstring(L, "pieces");
	lua_newtable(L);
	int d = 1;
	for (std::vector<bool>::const_iterator b = pieces.begin(); b != pieces.end(); ++b) {
	    LUA_PUSH_ARRAY_BOOL(d, *b);
	}	

	lua_settable(L, -3);

	LUA_PUSH_ATTRIB_BOOL("seed", i->seed);
        LUA_PUSH_ATTRIB_INT("upload_limit", i->upload_limit);
        LUA_PUSH_ATTRIB_INT("download_limit", i->download_limit);
        LUA_PUSH_ATTRIB_STRING("country", i->country);
        LUA_PUSH_ATTRIB_INT("load_balancing", i->load_balancing);
        LUA_PUSH_ATTRIB_INT("download_queue_length", i->download_queue_length);
        LUA_PUSH_ATTRIB_INT("upload_queue_length", i->upload_queue_length);
        LUA_PUSH_ATTRIB_INT("downloading_piece_index", i->downloading_piece_index);
        LUA_PUSH_ATTRIB_INT("downloading_block_index", i->downloading_block_index);
        LUA_PUSH_ATTRIB_INT("downloading_progress", i->downloading_progress);
        LUA_PUSH_ATTRIB_INT("downloading_total", i->downloading_total);
        LUA_PUSH_ATTRIB_STRING("client", i->client.c_str());
        LUA_PUSH_ATTRIB_INT("connection_type", i->connection_type);

        lua_settable(L, -3);

        c++;
    }

    return 1;
}

/*
 * queue = handle:get_download_queue()
 *
 *  returns a numerically indexed table with information about pieces 
 *  that are partially downloaded or not downloaded at all but partially requested
 *
 *  Not currently implemented
 */
static int torrent_handle_get_download_queue(lua_State *L) {
    void* ud = 0;

    luaL_error(L, "Not implemented");

    ud = luaL_checkudata(L, 1, "Torrent.Handle");

    lua_pushnil(L);

    return 1;
}

/*
 * handle:move_storage(newpath)
 *
 *  moves the file(s) that this torrent are currently seeding from or downloading to.
 */
static int torrent_handle_move_storage(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    boost::filesystem::path newpath(lua_tostring(L, 2)); 

    h->move_storage(newpath);

    return 0;
}

/*
 * bytes_per_second = handle:upload_limit()
 *
 *  returns the current limit setting for upload
 */
static int torrent_handle_upload_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushinteger(L, h->upload_limit());
    return 1;
}

/*
 * bytes_per_second = handle:download_limit()
 *
 *  returns the current limit setting for download
 */
static int torrent_handle_download_limit(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    lua_pushinteger(L, h->download_limit());
    return 1;
}

/*
 * priority = handle:piece_priority(piece_index)
 *  OR
 * handle:piece_priority(piece_index, priority)
 *
 * gets or sets the piece priority
 */

static int torrent_handle_piece_priority(lua_State *L) {
    int n = lua_gettop(L);
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    int index = luaL_checkinteger(L, 1);

    if (n == 3) {
	int priority = luaL_checkinteger(L, 1);
	h->piece_priority(index, priority);
    } else {
	int priority = h->piece_priority(index);
	lua_pushinteger(L, priority);
    }

    return n == 3 ? 0 : 1;
}

/*
 * handle:prioritize_pieces(pieces)
 *
 *  takes a table of integers, one integer per piece in the torrent. 
 *  All the piece priorities will be updated with the priorities in the vector.
 */
static int torrent_handle_prioritize_pieces(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    std::vector<int> pieces;

    lua_pushnil(L);  // first key
    while (lua_next(L, 2) != 0) {
        // uses 'key' (at index -2) and 'value' (at index -1)
        int priority = luaL_checkinteger(L, -1);

        pieces.push_back(priority);

        lua_pop(L, 1);
    }

    h->prioritize_pieces(pieces);
    return 0;
}

/*
 * pieces = handle:piece_priorities()
 *
 *  returns a table with one element for each piece in the torrent. 
 *  Each element is the current priority of that piece.
 */
static int torrent_handle_piece_priorities(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    std::vector<int> pieces = h->piece_priorities();

    int c = 1;
    lua_newtable(L);
    for (std::vector<int>::const_iterator i = pieces.begin(); i != pieces.end(); ++i) {
        LUA_PUSH_ARRAY_INT(c, *i);
    } 

    return 1;
}

/*
 * handle:prioritize_files(files)
 *
 *  takes a table that has at as many elements as there are 
 *  files in the torrent. Each entry is the priority of that file.
 */
static int torrent_handle_prioritize_files(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    std::vector<int> files;

    lua_pushnil(L);  // first key
    while (lua_next(L, 2) != 0) {
        // uses 'key' (at index -2) and 'value' (at index -1)
        int priority = luaL_checkinteger(L, -1);

        files.push_back(priority);

        lua_pop(L, 1);
    }

    h->prioritize_files(files);
    return 0;
}

/* 
 * handle:scrape_tracker()
 *
 *  send a scrape request to the tracker
 */
static int torrent_handle_scrape_tracker(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    h->scrape_tracker();    

    return 0;
}

/*
 * handle:use_interface(interface_name)
 *
 *  sets the network interface this torrent will use when it opens outgoing connections
 */
static int torrent_handle_use_interface(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    h->use_interface(luaL_checkstring(L, 2));

    return 0;
}

/*
 * handle:write_resume_data(filepath)
 *
 *  writes resume data to the file named filepath
 */
static int torrent_handle_write_resume_data(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    const char *filepath = luaL_checkstring(L, 2);

    ofstream out(complete(filepath), std::ios_base::binary);

    entry e = h->write_resume_data();
    libtorrent::bencode(std::ostream_iterator<char>(out), e);

    return 0;
}

/*
 * __gc
 */
static int torrent_handle_gc(lua_State *L) {
    void* ud = 0;

    ud = luaL_checkudata(L, 1, "Torrent.Handle");
    torrent_handle *h = *((torrent_handle **)ud);

    delete h;

    return 0;
}

static const luaL_Reg torrent_handle_methods[] = {
    {"status", torrent_handle_status},
    {"is_seed", torrent_handle_is_seed},
    {"is_paused", torrent_handle_is_paused},
    {"pause", torrent_handle_pause},
    {"resume", torrent_handle_resume},
    {"force_reannounce", torrent_handle_force_reannounce},
    {"name", torrent_handle_name},
    {"set_upload_limit", torrent_handle_set_upload_limit},
    {"set_download_limit", torrent_handle_set_download_limit},
    {"set_sequenced_download_threshold", torrent_handle_set_sequenced_download_threshold},
    {"set_ratio", torrent_handle_set_ratio},
    {"save_path", torrent_handle_save_path},
    {"set_max_uploads", torrent_handle_set_max_uploads},
    {"set_max_connections", torrent_handle_set_max_connections},
    {"set_tracker_login", torrent_handle_set_tracker_login}, 

    {"has_metadata", torrent_handle_has_metadata}, 
    {"get_torrent_info", torrent_handle_get_torrent_info}, 
    {"is_valid", torrent_handle_is_valid},
    {"file_progress", torrent_handle_file_progress},

    {"piece_priority", torrent_handle_piece_priority},
    {"prioritize_pieces", torrent_handle_prioritize_pieces},
    {"piece_priorities", torrent_handle_piece_priorities},
    {"prioritize_files", torrent_handle_prioritize_files},

    {"scrape_tracker", torrent_handle_scrape_tracker},
    
    /*
     * TODO implement these functions
     */
//  resolve_countries
//  trackers
//  replace_trackers
//  add_url_seed
//  remove_url_seed
//  url_seeds

    {"use_interface", torrent_handle_use_interface},
    {"write_resume_data", torrent_handle_write_resume_data}, 



    {"get_peer_info", torrent_handle_get_peer_info},
//    {"send_chat_message", torrent_handle_send_chat_message},
    {"get_download_queue", torrent_handle_get_download_queue},

    {"move_storage", torrent_handle_move_storage},
    {"upload_limit", torrent_handle_upload_limit},
    {"download_limit", torrent_handle_download_limit},

    {NULL, NULL}
};

int torrent_handle_register(lua_State *L) {
    luaL_newmetatable(L, "Torrent.Handle");
    luaL_register(L, 0, torrent_handle_methods);  
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");    

    lua_pushcfunction(L, torrent_handle_gc);
    lua_setfield(L, -2, "__gc");

    luaL_register(L, "Torrent.Handle", torrent_handle_methods);  

    return 1;
}

