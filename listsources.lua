#!/usr/bin/lua
require("lfs")

-- get the contents of the template
local f = io.open("config.ld.template", "rb")
contents = f:read("*all")
io.close(f)

-- get a list of all files
iter, dir_obj = lfs.dir("src/")
dirstr = ""
filestr = iter(dir_obj)
while filestr ~= nil do
    if string.sub(filestr, -#".c") == ".c" then
        if dirstr ~= "" then
            dirstr = dirstr .. ", "
        end
        dirstr = dirstr .. "\"src/" .. filestr .. "\""
    end
    filestr = iter(dir_obj)
end

-- replace template items:
local f = io.open("config.ld", "wb")
contents = string.gsub(contents, "INSERT_FILES_HERE", dirstr)
f:write(contents)
io.close(f)
print("Contents: " .. contents)

