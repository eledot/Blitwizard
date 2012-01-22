
-- Initialise audio first so FFmpeg is found in case we want it
pcall(blitwiz.audio.play)

-- Check if the user created a custom game we would want to run preferrably
if os.exists("../game.lua") then
	os.chdir("../")
	dofile("templates/init.lua")
	dofile("game.lua")
	return
end

-- We simply want to run the sample browser otherwise
os.chdir("samplebrowser")
dofile("browser.lua")

