
-- Avoid double initialisation
if blitwiz.templatesinitialised == true then
	return
end
blitwiz.templatesinitialised = true

-- Load all the templates
if os.sysname() ~= "Android" then
	-- Crawl the templates/ folder for templates
	for index,file in ipairs(os.ls("")) do
		if os.isdir("" .. file) then
			local filepath = file .. "/" .. file .. ".lua"
			if os.exists(filepath) then
				dofile(filepath)
			end
		end
	end
else
	-- For android, we get a pre-generated Lua file
    -- (provided by the android build script in scripts/ folder
    -- which assembled the .apk package)
	dofile("templates/filelist.lua")
end

