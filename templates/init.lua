
--Load all the templates
for index,file in ipairs(os.ls("templates/")) do
	if os.isdir("templates/" .. file) then
		local filepath = "templates/" .. file .. "/" .. file .. ".lua"
		if os.exists(filepath) then
			dofile(filepath)
		end
	end
end

