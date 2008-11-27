#!/usr/bin/lua

--
-- usage: dump_torrent.lua <file.torrent> 
--

require('luatorrent')

-- load torrent file from command line
local info = Torrent.Info.New(arg[1])

print("nodes:")
for i,e in pairs(info:nodes()) do
    print(string.format('%s %i', e[1], e[2]))
end

print("trackers:")
for i,t in pairs(info:trackers()) do
    print(string.format('%i %s bytes', t.tier, t.url))
end

print(info:total_size())
print(string.format("number of pieces: %i", info:num_pieces()))
print(string.format("piece length: %i", info:piece_length()))
print(string.format("comment: %s", info:comment()))
print(string.format("created by: %s", info:creator()))
print(string.format("info hash: %s", info:info_hash()))

print("files:")
for i,f in pairs(info:files()) do
    print(string.format('%s %i bytes', f.path, f.size))
end

