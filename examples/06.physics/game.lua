
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
	blitwiz.graphics.loadImage("shadows.png")

	-- Add base level collision
	local x,y = bgimagepos()
	levelcollision = blitwiz.physics.createStaticObject()
	blitwiz.physics.setShapeEdges(levelcollision, {
		{(119+x)/pixelspermeter, (0+y)/pixelspermeter,
		(119+x)/pixelspermeter, (360+y)/pixelspermeter},
		
		{(119+x)/pixelspermeter, (360+y)/pixelspermeter,
		(397+x)/pixelspermeter, (234+y)/pixelspermeter},

		{(397+x)/pixelspermeter, (234+y)/pixelspermeter,
		(545+x)/pixelspermeter, (371+y)/pixelspermeter},

		{(545+x)/pixelspermeter, (371+y)/pixelspermeter,
		(593+x)/pixelspermeter, (122+y)/pixelspermeter},

		{(593+x)/pixelspermeter, (122+y)/pixelspermeter,
		(564+x)/pixelspermeter, (0+y)/pixelspermeter}
	})
	blitwiz.physics.setFriction(levelcollision, 1)
	levelcollision2 = blitwiz.physics.createStaticObject()
	blitwiz.physics.setShapeRectangle(levelcollision2, ((382 - 222) + x)/pixelspermeter, ((314 - 242) + y)/pixelspermeter)
	blitwiz.physics.warp(levelcollision2, ((222+382)/2+x)/pixelspermeter, ((242 + 314)/2+y)/pixelspermeter)
	blitwiz.physics.setFriction(levelcollision2, 0.3)
end

function bgimagepos()
	local w,h = blitwiz.graphics.getImageSize("bg.png")
    local mw,mh = blitwiz.graphics.getWindowSize()
	return mw/2 - w/2, mh/2 - h/2
end

function blitwiz.on_draw()
	-- Draw the background image centered:
	local x,y = bgimagepos()
	blitwiz.graphics.drawImage("bg.png", x, y)

	-- Draw all crates:
	local imgw,imgh = blitwiz.graphics.getImageSize("crate.png")
	for index,crate in ipairs(crates) do
		local x,y = blitwiz.physics.getPosition(crate)
		local rotation = blitwiz.physics.getRotation(crate)
		blitwiz.graphics.drawImage("crate.png", x*pixelspermeter - imgw/2, y*pixelspermeter - imgh/2, 1, nil, nil, nil, nil, 1, 1, rotation)
	end

	-- Draw overall shadows:
	local x,y = bgimagepos()
	blitwiz.graphics.drawImage("shadows.png", x, y)
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

function blitwiz.on_mousedown(button, x, y)
	local imgposx,imgposy = bgimagepos()
	local crateposx = (x - imgposx)/pixelspermeter
	local crateposy = (y - imgposy)/pixelspermeter
	crateposx,crateposy = limitcrateposition(crateposx, crateposy)
	local crate = blitwiz.physics.createMovableObject()
	blitwiz.physics.setFriction(crate, 0.7)
	blitwiz.physics.setShapeRectangle(crate, cratesize, cratesize)
	blitwiz.physics.setMass(crate, 10)
	blitwiz.physics.warp(crate, crateposx, crateposy)
	blitwiz.physics.setAngularDamping(crate, 0.5)

	crates[#crates+1] = crate
end

function blitwiz.on_close()
	os.exit(0)
end

function blitwiz.on_step()

end

