
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

examples = { "01.helloworld", "02.simplecar", "03.sound", "04.simplecar.async", "05.scalerotate", "06.physics", "07.movingcharacter" }

yoffset = 150
yspacing = 1
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
    blitwiz.graphics.loadImage("return.png")
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
	blitwiz.graphics.drawImage("title.png", {x=w/2 - imgw/2, y=0})

	local i = 1
	while i <= #examples do
		imgw,imgh = blitwiz.graphics.getImageSize("menu" .. i .. ".png")
		local x,y = getbuttonpos(i)
		if menufocus == i then
			blitwiz.graphics.drawRectangle(x-2, y-1, imgw+4, imgh+2, 0,0.4,0.8)
		end
		blitwiz.graphics.drawImage("menu" .. i .. ".png", {x=x, y=y})
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

		-- Remember and delete our previous event functions
        browser_on_close = blitwiz.on_close
		blitwiz.on_close = nil
        browser_on_mousedown = blitwiz.on_mousedown
		blitwiz.on_mousedown = nil
        browser_on_mousemove = blitwiz.on_mousemove
		blitwiz.on_mousemove = nil
        browser_on_init = blitwiz.on_init
		blitwiz.on_init = nil
        browser_on_draw = blitwiz.on_draw
		blitwiz.on_draw = nil
        browser_on_step = blitwiz.on_step
        blitwiz.on_step = nil

		-- Load example
		dofile("game.lua")

        -- Wrap blitwiz.graphics.loadImage to load an image only if not present:
        if browser_loadImage_wrapped ~= true then
            browser_loadImage_wrapped = true

            -- wrap loadImage:
            local f = blitwiz.graphics.loadImage
            function blitwiz.graphics.loadImage(imgname)
                -- don't do anything if already being loaded or present
                if blitwiz.graphics.isImageLoaded(imgname) ~= nil then
                    return
                end

                -- otherwise, load:
                f(imgname)
            end

            -- wrap loadImageAsync:
            local f = blitwiz.graphics.loadImageAsync
            function blitwiz.graphics.loadImageAsync(imgname)
                -- don't do anything if already being loaded or present
                if blitwiz.graphics.isImageLoaded(imgname) ~= nil then
                    -- fire the callback if fully loaded:
                    if blitwiz.graphics.isImageLoaded(imgname) == true then
                        blitwiz.on_image(imgname, true)
                    end
                    return
                end

                -- otherwise, load:
                f(imgname)
            end
        end

        -- Wrap drawing to show return button:
        local f = blitwiz.on_draw
        example_start_time = blitwiz.time.getTime()
        blitwiz.on_draw = function()
            f()
            blitwiz.graphics.drawImage("return.png", {x=blitwiz.graphics.getWindowSize()-blitwiz.graphics.getImageSize("return.png"), y= -150 + math.min(150, (blitwiz.time.getTime() - example_start_time) * 0.2)})
        end

        -- Wrap on_mousedown to enable clicking the return button:
        local f = blitwiz.on_mousedown
        blitwiz.on_mousedown = function(button, x, y)
            local imgw,imgh = blitwiz.graphics.getImageSize("return.png")
            if (x > blitwiz.graphics.getWindowSize()-imgw and y < imgh) then
                -- Unload some often-conflicting images:
                blitwiz.graphics.unloadImage("bg.png")
                blitwiz.graphics.unloadImage("background.png")

                -- Restore event functions of the sample browser:
                blitwiz.on_close = browser_on_close
                blitwiz.on_mousedown = browser_on_mousedown
                blitwiz.on_mousemove = browser_on_mousemove
                blitwiz.on_init = browser_on_init
                blitwiz.on_draw = browser_on_draw
                blitwiz.on_step = browser_on_step
                return
            else
                if f ~= nil then
                    f(button, x, y)
                end
            end
        end

		-- Run example
		blitwiz.on_init()
	end
end

function blitwiz.on_close()
	os.exit(0)
end

