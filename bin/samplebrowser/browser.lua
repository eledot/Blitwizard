
--[[

blitwizard 2d engine - source code file

  Copyright (C) 2011 Jonas Thiem

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

]]

--[[

 This is the sample browser of blitwizard.
 It might be itself an interesting example to study for a more real-life
 appplication than the examples it is supposed to show, so feel free
 to read the code.

]]

examples = { "01.helloworld", "02.simplecar", "03.sound", "04.simplecar.async", "05.scalerotate", "06.physics" }

yoffset = 150
yspacing = 5
menufocus = 1

function blitwiz.on_init()
	print "Launching blitwizard sample browser"
	blitwiz.graphics.setWindow(640,480,"blitwizard 2d engine", false)
	blitwiz.graphics.loadImage("title.png")
	local i = 1
	while i <= #examples do
		blitwiz.graphics.loadImage("menu" .. i .. ".png")
		i = i + 1
	end
end

function getbuttonpos(index)
	local w,h = blitwiz.graphics.getWindowSize()
	imgw,imgh = blitwiz.graphics.getImageSize("menu" .. index .. ".png")
	local x = w/2 - imgw/2
	local y = yoffset + (index-1)*yspacing + (index-1)*imgh
	return x,y
end

function blitwiz.on_draw()
	local w,h = blitwiz.graphics.getWindowSize()
	blitwiz.graphics.drawRectangle(0, 0, w, h, 1, 1, 1)

	local imgw,imgh = blitwiz.graphics.getImageSize("title.png")
	blitwiz.graphics.drawImage("title.png", w/2 - imgw/2, 0)

	local i = 1
	while i <= #examples do
		imgw,imgh = blitwiz.graphics.getImageSize("menu" .. i .. ".png")
		local x,y = getbuttonpos(i)
		if menufocus == i then
			blitwiz.graphics.drawRectangle(x-2, y-2, imgw+4, imgh+4, 0,0.4,0.8)
		end
		blitwiz.graphics.drawImage("menu" .. i .. ".png", x, y)
		i = i + 1
	end
end

function blitwiz.on_mousemove(mousex, mousey)
	updatemenufocus(mousex, mousey)
end
function updatemenufocus(mousex, mousey)
	menufocus = 0
	local i = 1
	while i <= #examples do
		imgw,imgh = blitwiz.graphics.getImageSize("menu" .. i .. ".png")
		local x,y = getbuttonpos(i)
		if mousex >= x and mousex < x + imgw and
		mousey >= y and mousey < y + imgh then
			menufocus = i
			return
		end
		i = i + 1
	end
end

function blitwiz.on_mousedown(button, mousex, mousey)
	updatemenufocus(mousex, mousey)
	if menufocus > 0 then
		os.chdir("../../examples/" .. examples[menufocus] .. "/")
		-- Delete our previous event functions
		blitwiz.on_close = nil
		blitwiz.on_mousedown = nil
		blitwiz.on_mousemove = nil
		blitwiz.on_init = nil
		blitwiz.on_draw = nil
		-- Load example
		dofile("game.lua")
		-- Run example
		blitwiz.on_init()
	end
end

function blitwiz.on_close()
	os.exit(0)
end

