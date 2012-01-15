
--[[
   This example attempts to demonstrate the physics.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Physics example in blitwizard")
crates = {}
pixelspermeter = 30
cratesize = 64/pixelspermeter

function blitwiz.on_init()
	-- Open a window
	blitwiz.graphics.setWindow(640, 480, "Physics", false)

	-- Load image
	blitwiz.graphics.loadImage("bg.png")
	blitwiz.graphics.loadImage("crate.png")
end

function bgimagepos()
	local w,h = blitwiz.graphics.getImageSize("bg.png")
    local mw,mh = blitwiz.graphics.getWindowSize()
	return mw/2 - w/2, mh/2 - h/2
end

function blitwiz.on_draw()
	-- Draw the background image centered with our given scale factor:
	local x,y = bgimagepos()

	-- Actual drawing happens here
	blitwiz.graphics.drawImage("bg.png", x, y, 1, nil, nil, nil, nil, scalefactor, scalefactor, rotation)
end

function limitcrateposition(x,y)
	if x - cratesize/2 < 125/pixelspermeter then
		x = 125/pixelspermeter + cratesize/2
	end
	if x + cratesize/2 > 555/pixelspermeter then
		x = 555/pixelspermeter - cratesize/2
	end
	if y + cratesize/2 > 230/pixelspermeter then
		-- If we are below the lowest height, jump up a bit
		y = 130/pixelspermeter - cratesize/2
	end
	return x,y
end

function blitwiz.on_mousedown(x, y)
	local imgposx,imgposy = bgimagepos()
	local crateposx,crateposy = (x - imgposx)/pixelspermeter, (y - imgposy)/pixelspermeter
	crateposx,crateposy = limitcrateposition(crateposx, crateposy)
	crate = blitwiz.physics.createMovableObject()
	blitwiz.physics.setShapeRectangle(crate, cratesize, cratesize)
	blitwiz.physics.setMass(crate, 10)
	blitwiz.physics.wrap(crate, crateposx, crateposy)

	crates[#crates+1] = crate
end

function blitwiz.on_close()
	os.exit(0)
end

function blitwiz.on_step()

end

