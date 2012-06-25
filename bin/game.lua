
-- Initialise audio first so FFmpeg is found in case we want it
pcall(blitwiz.sound.play)

-- Try to run the templates first if they are one folder up
if os.exists("samplebrowser/browser.lua") then
	-- Templates are indeed one folder up, as it seems
	local olddir = os.getcwd()
	os.chdir("../templates/")
	local function calltemplates()
		dofile("init.lua")
	end
	calltemplates()
	os.chdir(olddir)
end

-- Check if the user created a custom game we would want to run preferrably
if os.exists("../game.lua") then
	os.chdir("../")
	dofile("game.lua")
	return
end

-- We simply want to run the sample browser otherwise
os.chdir("samplebrowser")
dofile("browser.lua")

