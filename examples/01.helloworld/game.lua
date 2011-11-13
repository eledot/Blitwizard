
--[[
   This is a simple example that simply loads an image
   and then displays it to the user in a 640x480 window.

   Moreover, the user can close the window.
]] --

print("Hello world example in blitwizard")

function blitwiz.callback.load()
	-- Open a window
	blitwiz.graphics.setWindow(640,480,"Hello World")

	-- Ask for an image to be loaded
	imageLoaded = false
	blitwiz.graphics.loadImage("hello_world.png",
		-- This function gets called when the image is ready
		function()
			-- Note down that we have the image loaded now
			imageLoaded = true
		end
	)
end

function blitwiz.callback.draw()
	-- This gets called each time the window is redrawn
	if imageLoaded == true then
		-- When the image is loaded, draw it centered:
		local w,h = blitwiz.graphics.getImageSize("hello_world.png")
		local mw,mh = blitwiz.graphics.getWindowSize()
		blitwiz.graphics.drawImage("hello_world.png", mw/2 - w/2, mh/2 - h/2)
	end
end

function blitwiz.callback.event.close()
	-- The user has attempted to close the window - terminate the app
	blitwizard.quit()
end

function blitwiz.callback.do()
	-- This gets called with fixed 60 FPS where we would want to do
	-- any sort of game logic or physics.
	-- For this example, we don't want to do anything here.
end

