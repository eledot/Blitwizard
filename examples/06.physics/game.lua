
--[[
   This example attempts to demonstrate the physics.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Physics example in blitwizard")

function blitwiz.on_init()
	-- Open a window
	blitwiz.graphics.setWindow(640, 480, "Physics", false)

	-- Load image
	blitwiz.graphics.loadImage("bg.png")
end

function blitwiz.on_draw()
	-- Draw the background image centered with our given scale factor:
	local w,h = blitwiz.graphics.getImageSize("bg.png")
	local mw,mh = blitwiz.graphics.getWindowSize()

	-- Actual drawing happens here
	blitwiz.graphics.drawImage("bg.png", mw/2 - w/2, mh/2 - h/2, 1, nil, nil, nil, nil, scalefactor, scalefactor, rotation)
end

function blitwiz.on_close()
	os.exit(0)
end

function blitwiz.on_step()

end

