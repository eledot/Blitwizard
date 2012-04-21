--[[-----
blitwiz.net.irc
Under the zlib license:

Copyright (c) 2012 Jonas Thiem

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required. 

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

--]]-----

blitwiz.net.irc = {}

blitwiz.net.irc.open = function(name, port, nickname, callback_on_event)
    -- callback_on_event gets a server info table (server) as first parameter,
    -- an event name as a second parameter,
    -- and additional parameters depending on the event type.
    -- event types:
    --     "connect": you connected to the server.
    --                you might want to use
    --                blitwiz.net.irc.channeljoin(server, "#somechannel") now
    --     "join": additional parameters: channel, nickname
    --             Someone with the given nickname joined the given channel.
    --             If it is yourself, you'll get a "names" event shortly after.
    --             In a channel, you might want to start using
    --             blitwiz.net.irc.channelmsg(server, "#joined_channel", "hello")
    --     "names": additional parameters: channel, list with names
    --              Contains a list of nicknames of users present in the given
    --              channel right now. If join/leave/quit/kick events follow,
    --              you will probably want to keep track of that and update
    --              your list.
    --     "leave": additional parameters: channel, nickname, reason
    --              Someone with the given nickname left the given channel.
    --              The reason can contain a free text string specifying
    --              a reason.
    --     "quit": additional parameters: nickname, reason
    --             Someone with the given nickname disconnected (-> leaves all
    --             channels). The reason can contain a free text string
    --             specifying a reason.
    --     "channelmessage": additional parameters: channel, nickname, message
    --                       Someone said something in a channel.
    --     "privatemessage": additional parameters: nickname, message
    --                       Someone said something privately to you.
    --                       You might want to respond with
    --                       blitwiz.net.irc.privatemsg(server, "nickname",
    --                       "hello you")
    --     "disconnect": you were disconnected from the server.

    local server = {
        connection = ""
    }
    server["connection"] = 
    blitwiz.net.open({server=name, port=port, linebuffered=true},
        function(stream)
            blitwiz.net.send(stream, "Nick " .. nickname .. "\nUser " .. nickname .. " 0 0 :" .. nickname .. "\n")
        end,
        function(stream, line)
            local args = {string.split(line, " ")}
            local i = 1
            local argc = #args
            while i <= argc do
                local start = 1
                if i == 1 then
                    start = 2
                end
                if string.find(args[i], ":", start) ~= nil then -- we got one final arg
                    local j = i
                    local finalarg = ""
                    -- get all following args as one single thing
                    while j <= #args do
                        finalarg = finalarg .. args[j]
                        j = j + 1
                    end
                    -- split it up correctly
                    args[i],args[i+1] = string.split(finalarg,":",1)
                    if #args[i] <= 0 then
                        args[i] = args[i+1]
                        i = i - 1
                    end
                    -- nil/remove all the following args
                    i = i+2
                    while i <= argc do
                        args[i] = nil
                        i = i + 1
                    end
                    break
                end
                i = i + 1
            end
            if string.starts(args[1], ":") and #args >= 2 then
                if args[2] == "001" then
                    callback_on_event(server, "connect")
                end
                if string.lower(args[2]) == "join" then
                    local actor = string.split(string.sub(args[1], 2), "!", 1)
                    callback_on_event(server, "join", args[3], actor)
                end
            end
            if string.lower(args[1]) == "ping" then
                blitwiz.net.send(stream, "PONG :" .. args[2] .. "\n")
            end
        end,
        function(stream)
            callback_on_event(server, "disconnect")
        end
    )
end

function blitwiz.net.irc.channeljoin(server, channel)
    blitwiz.net.send(server["connection"], "JOIN " .. channel .. "\n")
end

function blitwiz.net.irc.privatemsg(server, nickname, msg)
    blitwiz.net.send(server["connection"], "PRIVMSG " .. nickname .. " :" .. msg .. "\n")
end

function blitwiz.net.irc.channelmsg(server, channel, msg)
    blitwiz.net.send(server["connection"], "PRIVMSG " .. channel .. " :" .. msg .. "\n")
end

