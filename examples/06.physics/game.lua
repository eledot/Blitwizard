
--[[
   This example attempts to demonstrate the physics.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Physics example in blitwizard")

-- Some global vars we want to keep
crates = {} -- a list of all crate objects
balls = {} -- a list of all ball objects
pixelspermeter = 30 -- meter (physics unit) to pixels factor
cratesize = 64/pixelspermeter -- size of a crate
ballsize = 32/pixelspermeter -- size of a ball

function blitwiz.on_image(image, success)
	print("blubb: " .. image .. "," .. success)
end

-- Try to load templates if we don't have them
if blitwiz.templatesinitialised ~= true and os.exists("../../templates/") == true then
	local olddir = os.getcwd()
	os.chdir("../../templates/")
	dofile("templates/init.lua")
	os.chdir(olddir)
end

function blitwiz.on_init()
	-- Open a window
	blitwiz.graphics.setWindow(640, 480, "Physics", false)

	-- Load image
	blitwiz.graphics.loadImage("bg.png")
	blitwiz.graphics.loadImage("crate.png")
	blitwiz.graphics.loadImage("shadows.png")
	blitwiz.graphics.loadImage("ball.png")

	-- Add base level collision (as seen in bg.png)
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
	blitwiz.physics.setFriction(levelcollision, 0.5)

	-- Even more basic level collision (that black rectangle part in bg.png)
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

	-- Draw all balls
	local imgw,imgh = blitwiz.graphics.getImageSize("ball.png")
    for index,ball in ipairs(balls) do
        local x,y = blitwiz.physics.getPosition(ball)
        local rotation = blitwiz.physics.getRotation(ball)
        blitwiz.graphics.drawImage("ball.png", x*pixelspermeter - imgw/2, y*pixelspermeter - imgh/2, 1, nil, nil, nil, nil, 1, 1, rotation)
    end

	-- Draw overall shadows:
	local x,y = bgimagepos()
	blitwiz.graphics.drawImage("shadows.png", x, y)

	-- Draw stats
	blitwiz.font.draw("default", "Object stats:\nCrates: " .. tostring(#crates) .. ", balls: " .. tostring(#balls), 10, 10)
end

function limitcrateposition(x,y)
	if x - cratesize/2 < 125/pixelspermeter then
		x = 125/pixelspermeter + cratesize/2
	end
	if x + cratesize/2 > 555/pixelspermeter then
		x = 555/pixelspermeter - cratesize/2
	end
	if y + cratesize/2 > 230/pixelspermeter then
		-- If we are too low, avoid getting it stuck in the floor
		y = 130/pixelspermeter - cratesize/2
	end
	return x,y
end

function limitballposition(x,y)
    if x - ballsize/2 < 125/pixelspermeter then
        x = 125/pixelspermeter + ballsize/2
    end
    if x + ballsize/2 > 555/pixelspermeter then
        x = 555/pixelspermeter - ballsize/2
    end
    if y + ballsize/2 > 230/pixelspermeter then
        -- If we are below the lowest height, avoid getting stuck
        y = 130/pixelspermeter - ballsize/2
    end
    return x,y
end

function blitwiz.on_mousedown(button, x, y)
	local imgposx,imgposy = bgimagepos()

	-- See where we can add the object
	local objectposx = (x - imgposx)/pixelspermeter
	local objectposy = (y - imgposy)/pixelspermeter

	if math.random() > 0.5 then
		objectposx,objectposy = limitcrateposition(objectposx, objectposy)

		-- Add a crate
		local crate = blitwiz.physics.createMovableObject()
		blitwiz.physics.setFriction(crate, 0.7)
		blitwiz.physics.setShapeRectangle(crate, cratesize, cratesize)
		blitwiz.physics.setMass(crate, 10)
		blitwiz.physics.warp(crate, objectposx, objectposy)
		blitwiz.physics.setAngularDamping(crate, 0.5)
		blitwiz.physics.setLinearDamping(crate, 0.3)

		crates[#crates+1] = crate
	else
		objectposx,objectposy = limitballposition(objectposx, objectposy)

		-- Add a ball
        local ball = blitwiz.physics.createMovableObject()
        blitwiz.physics.setShapeCircle(ball, ballsize / 2)
        blitwiz.physics.setMass(ball, 2)
		blitwiz.physics.setFriction(ball, 0.4)
        blitwiz.physics.warp(ball, objectposx, objectposy)
        blitwiz.physics.setAngularDamping(ball, 0.02)
		blitwiz.physics.setLinearDamping(ball, 0.01)
		blitwiz.physics.setRestitution(ball, 0.3)

        balls[#balls+1] = ball
	end
end

function blitwiz.on_close()
	os.exit(0)
end

function blitwiz.on_step()

end

