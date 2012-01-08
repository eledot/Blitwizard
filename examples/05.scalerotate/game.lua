
--[[
   This example loads an image, then slowly shows it in
   increasing size to the user, while rotating it.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Scaling/rotation example in blitwizard")

function blitwiz.on_init()
	-- Open a window
	blitwiz.graphics.setWindow(640,480,"Scaling", false)

	print("We are scaling using the renderer: " .. blitwiz.graphics.getRendererName())	

	-- Load image
	blitwiz.graphics.loadImage("hello_world.png")

	-- Think of an initial scale factor	and rotation
	scalefactor = 0.7
	rotation = 0
end

function blitwiz.on_draw()
	-- Draw the image centered with our given scale factor:
	local w,h = blitwiz.graphics.getImageSize("hello_world.png")
	w = w * scalefactor
	h = h * scalefactor
	local mw,mh = blitwiz.graphics.getWindowSize()

	-- Actual drawing happens here
	blitwiz.graphics.drawImage("hello_world.png", mw/2 - w/2, mh/2 - h/2, 1, nil, nil, nil, nil, scalefactor, scalefactor, rotation)
end

function blitwiz.on_close()
	-- The user has attempted to close the window,
	-- so we want to respect his wishes and quit :-)
	os.exit(0)
end

function blitwiz.on_step()
	-- Slowly scale up the image on each step
	scalefactor = scalefactor + 0.0005
	rotation = rotation + 0.5
	if rotation > 360 then
		rotation = rotation - 360
	end
end

