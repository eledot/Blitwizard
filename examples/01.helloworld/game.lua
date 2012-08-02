
--[[
   This is a simple example that simply loads an image
   and then displays it to the user in a 640x480 window.

   Moreover, the user can close the window.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Hello world example in blitwizard")

-- All blitwiz.on_* functions are predetermined by
-- blitwizard and are called when various things happen
-- (if you have added them to your program). This allows
-- you to react to certain events (e.g. program has loaded,
-- the window is redrawn etc.).

-- For more information on which callback functions are
-- available, check the other examples or see the full
-- documentation on http://games.homeofjones.de/blitwizard/

-- Warn if we run without templates (you can remove this from
-- your own game if you wish to, it is just for convenience)
if blitwiz.templatesinitialised ~= true then
    error "The templates/ sub folder with the templates is apparently missing. Please copy it into the same folder as your game.lua before you start up."
end

function blitwiz.on_init()
	-- This function is called right after blitwizard has
	-- started. You would normally want to open up a
	-- window here with blitwiz.graphics.setWindow().

	-- Open a window
	function openwindow()
		blitwiz.graphics.setWindow(640, 480, "Hello World", false) -- resolution/size: 640x480, title: "Hello World", fullscreen: false/no
	end
	if pcall(openwindow) == false then
		-- Opening a window failed.
		-- Open fullscreen at any resolution (for Android)
		resolution = blitwiz.graphics.getDisplayModes()[1]
		blitwiz.graphics.setWindow(resolution[1], resolution[2], "Hello World", true)
	end

	-- Load images
	blitwiz.graphics.loadImage("hello_world.png")
    blitwiz.graphics.loadImage("system_info.png")
end

function blitwiz.on_keydown(key)
	-- When pressing space, we can switch between accelerated and software rendering with this:
	if key == "space" then
		if switchedtosoftware ~= true then
			blitwiz.graphics.setWindow(600,480,"Hello World", false, "software")
			switchedtosoftware = true
			print("Now: software mode")
		else
			blitwiz.graphics.setWindow(600,480,"Hello World", false)
			switchedtosoftware = false
			print("Now: accelerated mode")
		end
	end
	-- When escape is pressed, we want to quit
    if key == "escape" then
		os.exit(0)
	end
end

function blitwiz.on_draw()
	-- This gets called each time the window is redrawn.
	-- If you don't put this function here or if you don't put
	-- any drawing calls into it, the window will simply stay
	-- black.
	
	-- When the image is loaded, draw it centered:
	local w,h = blitwiz.graphics.getImageSize("hello_world.png")
	local mw,mh = blitwiz.graphics.getWindowSize()

	-- Actual drawing happens here
	blitwiz.graphics.drawImage("hello_world.png", {x=mw/2 - w/2, y=mh/2 - h/2})
    blitwiz.graphics.drawImage("system_info.png", {x=mw/2 - w/2, y=mh/2 + h/2 - 30})
	blitwiz.font.draw("default", _VERSION .. ",\nrunning on: "  .. os.sysname() .. " (" .. os.sysversion() .. ")", mw/2 - w/2 + 20, mh/2 + h/2 - 30 + 28)

	-- Done!
end

function blitwiz.on_close()
	-- This function gets called whenever the user clicks
	-- the close button in the window title bar.
	
	-- The user has attempted to close the window,
	-- so we want to respect his wishes and quit :-)
	os.exit(0)
end

function blitwiz.on_step()
	-- This gets called with fixed 60 FPS constantly (it
	-- will get called more often if FPS are lower to
	-- keep up).
	
	-- This is the right place for game physics/continuous
	-- movements or calculations of any sort that are part
	-- of your continuously running game simulation.

	-- For this example, we could e.g. quit after 10 seconds -
	-- see below for an explanation on how to achieve that:
	
    -- Remember the time when we started up:
	if startupTime == nil then -- Check if we already remembered the time
        -- We didn't, therefore set it:
		startupTime = blitwiz.time.getTime()
	end

    -- Check when 10 seconds have expired:
	if blitwiz.time.getTime() > startupTime + 10000 then
		-- Remove leading -- from next line to enable quitting after 10s:
        --os.exit(0)
	end
end

