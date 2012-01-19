
--[[
   This example attempts to demonstrate a movable character
   based on the physics.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Moving character example in blitwizard")
pixelspermeter = 30
cratesize = 64/pixelspermeter
frame = 1

function blitwiz.on_init()
	-- Open a window
	blitwiz.graphics.setWindow(640, 480, "Moving character", false)

	-- Load image
	blitwiz.graphics.loadImage("bg.png")
	blitwiz.graphics.loadImage("crate.png")
	blitwiz.graphics.loadImage("char1.png")
	blitwiz.graphics.loadImage("char2.png")
	blitwiz.graphics.loadImage("char3.png")

	-- Add base level collision
	local x,y = bgimagepos()
	levelcollision = blitwiz.physics.createStaticObject()
	blitwiz.physics.setShapeEdges(levelcollision, {
		{(0+x)/pixelspermeter, (245+y)/pixelspermeter,
        (0+x)/pixelspermeter, (0+y)/pixelspermeter},

		{(0+x)/pixelspermeter, (245+y)/pixelspermeter,
		(93+x)/pixelspermeter, (297+y)/pixelspermeter},

		{(93+x)/pixelspermeter, (297+y)/pixelspermeter,
		(151+x)/pixelspermeter, (293+y)/pixelspermeter},

		{(151+x)/pixelspermeter, (293+y)/pixelspermeter,
		(198+x)/pixelspermeter, (306+y)/pixelspermeter},

		{(198+x)/pixelspermeter, (306+y)/pixelspermeter,
		(268+x)/pixelspermeter, (375+y)/pixelspermeter},
	
		{(268+x)/pixelspermeter, (375+y)/pixelspermeter,
		(309+x)/pixelspermeter, (350+y)/pixelspermeter},

		{(309+x)/pixelspermeter, (350+y)/pixelspermeter,
		(357+x)/pixelspermeter, (355+y)/pixelspermeter},

		{(357+x)/pixelspermeter, (355+y)/pixelspermeter,
		(416+x)/pixelspermeter, (427+y)/pixelspermeter},

		{(416+x)/pixelspermeter, (427+y)/pixelspermeter,
		(470+x)/pixelspermeter, (431+y)/pixelspermeter},

		{(470+x)/pixelspermeter, (431+y)/pixelspermeter,
        (512+x)/pixelspermeter, (407+y)/pixelspermeter},

		{(512+x)/pixelspermeter, (407+y)/pixelspermeter,
        (558+x)/pixelspermeter, (372+y)/pixelspermeter},

		{(558+x)/pixelspermeter, (372+y)/pixelspermeter,
        (640+x)/pixelspermeter, (364+y)/pixelspermeter},

		{(640+x)/pixelspermeter, (364+y)/pixelspermeter,
        (640+x)/pixelspermeter, (0+y)/pixelspermeter}
	})
	blitwiz.physics.setFriction(levelcollision, 0.5)

	-- Add character
	char = blitwiz.physics.createMovableObject()
	blitwiz.physics.setShapeRectangle(char, (50+x)/pixelspermeter, (140+y)/pixelspermeter)
	blitwiz.physics.setMass(char, 60)
	blitwiz.physics.setFriction(char, 0.5)
	blitwiz.physics.warp(char, (456+x)/pixelspermeter, (288+y)/pixelspermeter)
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

	-- Draw the character
	local x,y = blitwiz.physics.getPosition(char)
	local w,h = blitwiz.graphics.getImageSize("char1.png")
	blitwiz.graphics.drawImage("char" .. frame .. ".png", x*pixelspermeter - w/2, y*pixelspermeter - h/2)
end

function blitwiz.on_close()
	os.exit(0)
end

function blitwiz.on_step()

end

