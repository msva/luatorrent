#!/usr/bin/lua

require('luatorrent')

-- load torrent file from command line
local info = Torrent.Info.New(arg[1])
info:save_to_file(string.format('%s.torrent', info:info_hash()))
