
--[[
   This example attempts to demonstrate a movable character
   based on the physics.

   You can copy, alter or reuse this file any way you like.
   It is available without restrictions (public domain).
]]

print("Moving character example in blitwizard")
pixelspermeter = 30
cratesize = 64/pixelspermeter
frame = 1 -- animation frame to be displayed (can be 1, 2, 3)
leftright = 0 -- keyboard left/right input
animationstate = 0 -- used for timing the animation
flipped = false -- whether to draw the character flipped (=facing left)
jump = false -- keyboard jump input
lastjump = 0 -- remember our last jump to enforce a short no-jump time after each jump

function blitwiz.on_init()
	-- Open a window
	blitwiz.graphics.setWindow(640, 480, "Moving character", false)

	-- Load image
	blitwiz.graphics.loadImage("bg.png")
	blitwiz.graphics.loadImage("char1.png")
	blitwiz.graphics.loadImage("char2.png")
	blitwiz.graphics.loadImage("char3.png")

	-- Add base level collision
	local x,y = bgimagepos()
	levelcollision = blitwiz.physics2d.createStaticObject()
	blitwiz.physics2d.setShapeEdges(levelcollision, {
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
	blitwiz.physics2d.setFriction(levelcollision, 0.5)

	-- Add character
	char = blitwiz.physics2d.createMovableObject()
	blitwiz.physics2d.setShapeOval(char, (50+x)/pixelspermeter, (140+y)/pixelspermeter)
	blitwiz.physics2d.setMass(char, 60)
	blitwiz.physics2d.setFriction(char, 0.3)
	--blitwiz.physics2d.setLinearDamping(char, 10)
	blitwiz.physics2d.warp(char, (456+x)/pixelspermeter, (188+y)/pixelspermeter)
	blitwiz.physics2d.restrictRotation(char, true)
end

function bgimagepos()
	-- Return the position of the background image (normally just 0,0):
	local w,h = blitwiz.graphics.getImageSize("bg.png")
    local mw,mh = blitwiz.graphics.getWindowSize()
	return mw/2 - w/2, mh/2 - h/2
end

function blitwiz.on_keydown(key)
	-- Process keyboard walk/jump input
	if key == "a" then
		leftright = -1
	end
	if key == "d" then
		leftright = 1
	end
	if key == "w" then
		jump = true
	end
end

function blitwiz.on_keyup(key)
	if key == "a" and leftright < 0 then
		leftright = 0
	end
	if key == "d" and leftright > 0 then
		leftright = 0
	end
	if key == "w" then
		jump = false
	end
end

function blitwiz.on_draw()
	if flipped == nil then
		flipped = false
	end

	-- Draw the background image centered:
	local bgx,bgy = bgimagepos()
	blitwiz.graphics.drawImage("bg.png", {x=bgx, y=bgy})

	-- Draw the character
	local x,y = blitwiz.physics2d.getPosition(char)
	local w,h = blitwiz.graphics.getImageSize("char1.png")
	blitwiz.graphics.drawImage("char" .. frame .. ".png", {x=x*pixelspermeter - w/2 + bgx, y=y*pixelspermeter - h/2 + bgy, flipped=flipped})
end

function blitwiz.on_close()
	os.exit(0)
end

function blitwiz.on_step()
	local onthefloor = false
	local charx,chary = blitwiz.physics2d.getPosition(char)

	-- Cast a ray to check for the floor
	local obj,posx,posy,normalx,normaly = blitwiz.physics2d.ray(charx,chary,charx,chary+350/pixelspermeter)

	-- Check if we reach the floor:
	local charsizex,charsizey = blitwiz.graphics.getImageSize("char1.png")
	charsizex = charsizex / pixelspermeter
	charsizey = charsizey / pixelspermeter
	if obj ~= nil and posy < chary + charsizey/2 + 1/pixelspermeter then
		onthefloor = true
	end

	local walkanim = false
	-- Enable walking if on the floor
	if onthefloor == true then
		-- walk
		if leftright < 0 then
			flipped = true
			walkanim = true
			blitwiz.physics2d.impulse(char, charx + 5, chary - 3, -0.4, -0.6)
		end
		if leftright > 0 then
			flipped = false
			walkanim = true
        	blitwiz.physics2d.impulse(char, charx - 5, chary - 3, 0.4, -0.6)
		end
		-- jump
		if jump == true and lastjump + 500 < blitwiz.time.getTime() then
			lastjump = blitwiz.time.getTime()
			blitwiz.physics2d.impulse(char, charx, chary - 1, 0, -20)
		end
	end

	-- Check out how to animate
	if walkanim == true then
		-- We walk, animate:
		frame = 2
		animationstate = animationstate + 1
		if animationstate >= 13 then -- half of the animation time is reached, switch to second walk frame
			frame = 3
		end
		if animationstate >= 26 then -- wrap over at the end of the animation
			animationstate = 0
			frame = 2
		end
	else
		frame = 1
	end
end

