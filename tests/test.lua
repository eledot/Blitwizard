local outputline = ""
os.chdir("../templates/")
filelist = {}

-- get all files (unsorted)
for index,file in ipairs(os.ls("")) do
    filelist[#filelist+1] = file
end

-- sort files and turn into string:
table.sort(filelist)
for index,file in ipairs(filelist) do
    if #outputline > 0 then
        outputline = outputline .. " "
    end
    outputline = outputline .. file
end

print("xx" .. outputline .. "yy")
