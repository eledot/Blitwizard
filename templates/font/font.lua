--[[-----
blitwiz.font
Under the zlib license:

Copyright (c) 2012 Jonas Thiem

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required. 

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

--]]-----

blitwiz.font = {}
blitwiz.font.fonts = {}

function loadfontimage(path)
    if blitwiz.graphics.isImageLoaded(path) ~= true then
        blitwiz.graphics.loadImageAsync(path)
    end
end

function blitwiz.font.register(path, name, charwidth, charheight, charsperline, charset)
	blitwiz.font.fonts[name] = {
		path,
		charwidth,
		charheight,
		charsperline,
		charset
	}
    result = pcall(loadfontimage, path)
    if result == false then
        -- We cannot load the font as it seems
        blitwiz.font.fonts[name] = nil
    end
end

local function drawfontslot(font, slot, posx, posy, r, g, b, a, clipx, clipy, clipw, cliph)
	-- check for invalid slot:
	if slot < 1 or slot > 32*8 then
		return
	end	

	-- do we need to draw at all?
	if blitwiz.graphics.isImageLoaded(font[1]) ~= true then
		return
	end
	if clipx ~= nil and clipy ~= nil then
		if clipx >= posx + font[2] or clipy >= posy + font[3] then
			return
		end
		if clipw ~= nil and cliph ~= nil then
			if clipx + clipw < posx or clipy + cliph < posy then
				return
			end
		end
	end
	
	-- Find out row
	local row = 1
	while slot > font[4] do
		row = row + 1
		slot = slot - font[4]
	end
	
	-- Draw
	local cutx = 0
	local cuty = 0
	local cutw = font[2]
	local cuth = font[3]
	if clipx ~= nil and clipy ~= nil then
		cutx = math.max(0, clipx - posx)
		cuty = math.max(0, clipy - posy)
		if clipw ~= nil and cliph ~= nil then
			cutw = math.max(0, clipw + (clipx - posx))
			cuth = math.max(0, cliph + (clipy - posy))
		end
	end
	if cutw + cutx > font[2] then
		cutw = font[2] - cutx
	end
	if cuth + cuty > font[3] then
		cuth = font[3] - cuty
	end
	blitwiz.graphics.drawImage(font[1], {x=posx, y=posy, alpha=a, cutx=cutx + (slot-1) * font[2], cuty=cuty + (row-1) * font[3], cutwidth=cutw, cutheight=cuth, red=r, green=g, blue=b})
end

function blitwiz.font.draw(name, text, posx, posy, r, g, b, a, wrapwidth, clipx, clipy, clipw, cliph)
	local origposx = posx
	local font = blitwiz.font.fonts[name]
    if font == nil then
        error ("Font \"" .. name .. "\" is not loaded")
    end
    -- calculate wrap width:
    local maxperline = nil
    if wrapwidth ~= nil then
        maxperline = math.floor(wrapwidth/font[2])
        if maxperline < 1 then
            maxperline = 1
        end
    end
    -- draw the font char by char
	local i = 1
    local charsperline = 0
	while i <= #text do
		local character = string.byte(string.sub(text, i, i))
		if character == string.byte("\n") then
            -- user defined line breaks should be possible:
			posx = origposx
			posy = posy + font[3]
            charsperline = 0
		else 
            -- adhere to maximum line length
            local linefull = false
            if maxperline then
                if charsperline >= maxperline then
                    charsperline = 0
                    linefull = true
                    i = i - 1 -- revoke later skipping of current char
                end
            end
            -- draw char:
            if not linefull then
			    local slot = (character - string.byte(" "))+1
			    drawfontslot(font, slot, posx, posy, r, g, b, a, clipx, clipy, clipw, cliph)
			    posx = posx + font[2]
            end
		end
		i = i + 1
        charsperline = charsperline + 1
	end
end

function blitwiz.font.addToString_inner(str, line, maxlinelength, maxlinecount)
    -- Split line when horizontally too long
    while #line > maxlinelen do
        str = str .. string.sub(line, 1, maxlinelen) .. "\n"
        line = "  " .. string.sub(line, maxlinelen + 1)
    end
    str = str .. line .. "\n"

    -- Scroll text when too many vertical lines
    local linecount = #{string.split(str, "\n")}
    while linecount > maxlines do
        local throwaway = ""
        throwaway,str = string.split(str, "\n", 1)
        linecount = linecount - 1
    end
    return str
end

function blitwiz.font.addToString(str, line, maxlinelength, maxlinecount)
    local lines = {string.split(line, "\n")}
    for key,value in ipairs(lines) do
        str = blitwiz.font.addToString_inner(str, value, maxlinelength, maxlinecount)
    end
    return str
end

blitwiz.font.register("font/default.png", "default", 7, 14, 32, "iso-8859-15")

