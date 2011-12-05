
--[[
   This is an example that simply shows a moving car to
   demonstrate how to do a simple movement animation.

   It is different to example 2 by using more sophisticated
   async loading (which allows you to do other things while
   the loading is in progress).

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Simple car example in blitwizard with async image loading")


function blitwiz.on_init()
	blitwiz.graphics.setWindow(640,480,"Simple Car (Asynchronous image loading)")
	carx = 0
	
	-- Ask for images to be loaded ASYNCHRONOUSLY (in the background)
	imageCount = 0
	blitwiz.graphics.loadImageAsync("background.png")
	blitwiz.graphics.loadImageAsync("car.png")
	-- We have to be careful when our images are actually available.
	-- For this, we check the on_image event (see below).
end

function blitwiz.on_keydown(key)
	-- Quit on escape
	if  key == "escape" then
		os.exit(0)
	end
end

function blitwiz.on_draw()
	-- Draw scenery:
	
	-- We have to check whether our image is loaded first,
	-- since image loading in blitwizard doesn't hang your
	-- whole application, but instead happens in the background:
	if imageCount >= 2 then
		-- Calculate background position
		local w,h = blitwiz.graphics.getImageSize("background.png")
		local mw,mh = blitwiz.graphics.getWindowSize()

		-- Draw background
		blitwiz.graphics.drawImage("background.png", mw/2 - w/2, mh/2 - h/2)

		-- Draw car
		local carwidth,carheight = blitwiz.graphics.getImageSize("car.png");
		-- We want to draw it at the car x position, and directly on the ground of the window
		blitwiz.graphics.drawImage("car.png", carx, mh - carheight); 
	end

	-- Done!
end

function blitwiz.on_image(name, success)
	-- Count up until we know we have all images loaded we want

	if success ~= true then
		os.exit(-1)
	end
	imageCount = imageCount + 1	
end

function blitwiz.on_close()
	os.exit(0)
end


function blitwiz.on_step()
	-- We will continuously move our car here (only when images are loaded):
	if imageCount >= 2 then
		carx = carx + 1
		-- Check whether we exceeded screen bounds:
		local w,h = blitwiz.graphics.getWindowSize()
		if carx >= w then
			-- Set us back to the left border
			local imgwidth,imgheight = blitwiz.graphics.getImageSize("car.png")
			carx = -imgwidth
		end
	end
end

