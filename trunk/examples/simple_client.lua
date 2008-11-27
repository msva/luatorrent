#!/usr/bin/lua

--
-- usage: simple_client.lua <file.torrent> 
--

require('luatorrent')

-- load torrent info from first argument
local info = Torrent.Info.New(arg[1])

-- create a session object listening on ports 7000-7010
local session = Torrent.Session.New(7000, 7010)

-- add torrent file(info) to session
local handle = session:add_torrent(info, arg[2])

-- loop while torrent is not complete
while (handle:status().progress < 1) do
end

