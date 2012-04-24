
--[[
   This is an example that simply shows a moving car to
   demonstrate how to do a simple movement animation.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Simple car example in blitwizard")


function blitwiz.on_init()
	-- This function is called right after blitwizard has
	-- started. You would normally want to open up a
	-- window here with blitwiz.graphics.setWindow().
	
	-- Open a window
	blitwiz.graphics.setWindow(640, 480, "Simple Car")
	
	-- Think of a car position:
	carx = 0
	-- We will use this later in the step function and
	-- increase it steadily for a moving car.
	
	-- Load images
	blitwiz.graphics.loadImage("background.png")
	blitwiz.graphics.loadImage("car.png")
	blitwiz.graphics.loadImage("nightmask.png")
end

function blitwiz.on_keydown(key)
	-- Quit on escape
	if  key == "escape" then
		os.exit(0)
	end
end

function blitwiz.on_draw()
	-- Draw scenery:
	
	-- Calculate background position
	local w,h = blitwiz.graphics.getImageSize("background.png")
	local mw,mh = blitwiz.graphics.getWindowSize()

	-- Draw background
	blitwiz.graphics.drawImage("background.png", {x=mw/2 - w/2, y=mh/2 - h/2})

	-- Draw car
	local carwidth,carheight = blitwiz.graphics.getImageSize("car.png");
	-- We want to draw it at the car x position, and directly on the ground of the window
	blitwiz.graphics.drawImage("car.png", {x=carx, y=mh - carheight}); 
	
	-- Draw night mask
	blitwiz.graphics.drawImage("nightmask.png", {x=mw/2 - w/2, y=mh/2 - h/2})
end

function blitwiz.on_close()
	-- The user has attempted to close the window,
	-- so we want to respect his wishes and quit :-)
	os.exit(0)
end


function blitwiz.on_step()
	-- We will continuously move our car here:
	carx = carx + 1
	-- Check whether we exceeded screen bounds:
	local w,h = blitwiz.graphics.getWindowSize()
	if carx >= w then
		-- Set us back to the left border
		local imgwidth,imgheight = blitwiz.graphics.getImageSize("car.png")
		carx = -imgwidth
	end
end

