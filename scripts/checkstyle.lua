
-- Blitwizard style checker
-- *----------------------*

args = {...}
styleerror = false

language = "c"


function error_whitespace(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " ends with whitespace(s)")
end

function error_linecommentspace(file, nr)
    styleerror = true
    local linecomment = "//"
    if string.lower(language) == "lua" then
        linecomment = "--"
    end
    print("Style: line " .. file .. ":" .. nr .. " should have a whitespace between " .. linecomment .. " and the comment text")
end

function checkcommentsandstrings(file, nr, line, insideblockcomment, insidestring)
    -- Get start positions of comments, strings:
    local blockcommentstart = string.find(line, "/*", 1, true)
    local linecommentstart = string.find(line, "//", 1, true)
    local linestringstart = string.find(line, "\"", 1, true)
    if blockcommentstart == nil then
        blockcommentstart = 1/0
    end
    if linecommentstart == nil then
        linecommentstart = 1/0
    end
    if linestringstart == nil then
        linestringstart = 1/0
    end

    -- Now lets go split:
    if insideblockcomment == false and insidestring == false then
        if linecommentstart < linestringstart
        and linecommentstart < blockcommentstart then
            -- the line comment takes precedence over strings, block comments
            
            -- check for white space following the line comment
            if linecommentstart + #"//" <= #line then
                if string.sub(line, linecommentstart + #"//", linecommentstart + #"//") ~= " " then
                    error_linecommentspace(file, nr)
                end
            end

            -- return line up to the comment
            if linecommentstart > 1 then
                return string.sub(line, 1, linecommentstart-1), false, false
            else
                return "", false, false
            end
        end

        if linestringstart < blockcommentstart then
            -- string starts before block comment -> check remaining string
            local remainingstring = ""
            remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, string.sub(line, linestringstart+1), false, true)
            return string.sub(line, 1, linestringstart) .. remainingstring, insideblockcomment, insidestring
        end

        if blockcommentstart < 1/0 then
            -- we got a block comment -> check remaining string
            local remainingstring = ""
            remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, string.sub(line, blockcommentstart + #"/*"), true, false)
            return string.sub(line, 1, blockcommentstart - 1) .. remainingstring, insideblockcomment, insidestring
        end
        return line, false, false
    else
        if insidestring ~= false then
            local stringend = string.find(line, "\"", 1, true)

            -- deal with \" occurances:
            if stringend > 1 then
                if line[stringend-1] == "\\" then
                    -- escaped, -> ignore
                    local remainingstring = ""
                    remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, string.sub(line, stringend), false, true)
                    return string.sub(line, 1, stringend - 1) .. remainingstring, insideblockcomment, insidestring
                end
            end

            -- deal with valid " string end:
            if stringend ~= nil then
                local remainingstring = ""
                remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, string.sub(line, stringend+1), false, false)
                return string.sub(line, 1, stringend) .. remainingstring, insideblockcomment, insidestring
            end
            return line, false, true
        else
            local commentend = string.find(line, "*/", 1, true)
            if commentend ~= nil then
                local remainingstring = ""
                remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, string.sub(line, commentend + #"*/"), false, false)
                return remainingstring, insideblockcomment, insidestring
            else
                return "", true, false
            end
        end
    end
end

function checkfile(file)
    local n = 1
    local insideblockcomment = false
    local insidestring = false
    for line in io.lines(file) do
        local line_without_comments = ""

        -- strip line breaks
        while string.ends(line, "\n") or string.ends(line, "\r") do
            line = string.sub(line, 1, #line - 1)
        end

        -- check white space at the end of line
        if string.ends(line, " ") then
            error_whitespace(file, n)
        end

        -- get line without line, block comments
        local isolatedline = ""
        isolatedline,insideblockcomment,insidestring = checkcommentsandstrings(file, n, line, insideblockcomment, insidestring)
        if #isolatedline > 0 then
            --print(file .. ":" .. n .. ": " .. isolatedline)
        end

        n = n + 1
    end
end

function run()
    -- Run the style checker
    local i = 1
    while i <= #args do
        checkfile(args[i])
        i = i + 1
    end
    if styleerror then
        os.exit(1)
    else
        os.exit(0)
    end
end

return run()

