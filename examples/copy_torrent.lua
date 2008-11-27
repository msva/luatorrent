#!/usr/bin/lua

--
-- usage: copy_torrent.lua <file.torrent> 
--

require('luatorrent')

-- load torrent file from command line
local info = Torrent.Info.New(arg[1])

-- write info to a new file
-- based on the info hash string
info:save_to_file(string.format('%s.torrent', info:info_hash()))
