
-- Blitwizard style checker
-- *----------------------*

-- Some style check options:

c_indent_spaces = 4
c_bracket_on_separate_line = true
lua_indent_spaces = 4
lua_use_semicolon = false

-- End of options.

args = {...}
styleerror = false

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

function error_linecommentspace2(file, nr)
    styleerror = true
    local linecomment = "//"
    if string.lower(language) == "lua" then
        linecomment = "--"
    end
    print("Style: line " .. file .. ":" .. nr .. " should have a whitespace between the code it follows and " .. linecomment)
end

function error_shortblockcomment(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " has a short block comment, please convert to line comment(s)")
end

function error_bracketnotonseparateline(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " has a bracket on the same line as other code, put it isolated into the next line")
end

function error_bracketonseparateline(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " has an isolated bracket, please put it on the previous line")
end

function checkcommentsandstrings(file, nr, language, line, insideblockcomment, insidestring, blockcommentlength)
    -- Get start position of block comments:
    local blockcommentstart = 1/0
    if language == "c" then
        local v = string.find(line, "/*", 1, true)
        if v ~= nil then
            blockcommentstart = v
        end
    elseif language == "lua" then
        local v = string.find(line, "--[[", 1, true)
        if v ~= nil then
            blockcommentstart = v
        end
    end

    -- Get start position of line comments:
    local linecommentstart = 1/0
    if language == "c" then
        local v = string.find(line, "//", 1, true)
        if v ~=  nil then
            linecommentstart = v
        end
    elseif language == "lua" then
        local v = string.find(line, "--", 1, true)
        if v ~= nil then
            if blockcommentstart > v then -- don't interpret as block comment
                linecommentstart  = v
            end
        end
    end

    -- Get the position of string start:
    local linestringstart = string.find(line, "\"", 1, true)
    if linestringstart == nil then
        linestringstart = 1 / 0
    end
    -- Skip escaped string starts
    while linestringstart > 1 and linestringstart < 1 / 0 do
        if string.sub(line, linestringstart - 1, linestringstart - 1) == "\"" then
            linestringstart = string.find(line, "\"", linestringstart + 1, true)
            if linestringstart == nil then
                linestringstart = 1 / 0
            end
        else
            break
        end
    end

    -- Check second string notation type for lua:
    if language == "lua" then
        local linestringstart2 = string.find(line, "[[", 1, true)
        if linestringstart2 ~= nil then
            if linestringstart2 < linestringstart then
                linestringstart = linestringstart2
            end
        end
    end

    -- Now lets go scan for line comments, block comments, strings:
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

            -- check if a space is before the line comment:
            if linecommentstart > 1 then
                if string.sub(line, linecommentstart - 1, linecommentstart - 1) ~= " " then
                    error_linecommentspace2(file, nr)
                end
            end

            -- return line up to the comment
            if linecommentstart > 1 then
                return string.sub(line, 1, linecommentstart-1), false, false, 0
            else
                return "", false, false, 0
            end
        end

        if linestringstart < blockcommentstart then
            -- string starts before block comment -> check remaining string
            local remainingstring = ""
            remainingstring,insideblockcomment,insidestring,blockcommentlength = checkcommentsandstrings(file, nr, language, string.sub(line, linestringstart+1), false, true, 0)
            return string.sub(line, 1, linestringstart) .. remainingstring, insideblockcomment, insidestring, 0
        end

        if blockcommentstart < 1/0 then
            -- we got a block comment -> check remaining string
            local remainingstring = ""
            remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, language, string.sub(line, blockcommentstart + #"/*"), true, false, 1)
            return string.sub(line, 1, blockcommentstart - 1) .. remainingstring, insideblockcomment, insidestring, blockcommentlength
        end
        return line, false, false, 0
    else
        if insidestring ~= false then
            local stringend = string.find(line, "\"", 1, true)

            -- deal with \" occurances:
            if stringend > 1 then
                if line[stringend-1] == "\\" then
                    -- escaped, -> ignore
                    local remainingstring = ""
                    remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, language, string.sub(line, stringend), false, true, 0)
                    return string.sub(line, 1, stringend - 1) .. remainingstring, insideblockcomment, insidestring, 0
                end
            end

            -- deal with valid " string end:
            if stringend ~= nil then
                local remainingstring = ""
                remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, language, string.sub(line, stringend+1), false, false, 0)
                return string.sub(line, 1, stringend) .. remainingstring, insideblockcomment, insidestring, 0
            end
            return line, false, true
        else
            local commentend = string.find(line, "*/", 1, true)
            if commentend ~= nil then
                -- check for short block comments (should be line comments!)
                if blockcommentlength <= 2 then
                    error_shortblockcomment(file, nr)
                end

                local remainingstring = ""
                remainingstring,insideblockcomment,insidestring,blockcommentlength = checkcommentsandstrings(file, nr, language, string.sub(line, commentend + #"*/"), false, false, blockcommentlength)
                return remainingstring, insideblockcomment, insidestring, blockcommentlength
            else
                return "", true, false, blockcommentlength + 1
            end
        end
    end
end

function checkfile(file)
    local n = 1
    local insideblockcomment = false
    local insidestring = false
    local blockcommentlength = 0
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
        isolatedline,insideblockcomment,insidestring,blockcommentlength = checkcommentsandstrings(file, n, "c", line, insideblockcomment, insidestring, blockcommentlength)
        while string.ends(isolatedline, " ") do
            if #isolatedline > 1 then
                isolatedline = string.sub(isolatedline, 1, #isolatedline - 1)
            else
                isolatedline = ""
            end
        end
        if #isolatedline > 0 then
            if insidestring == false then
                if language == "c" then
                    -- Check for bracket on separate line
                    if string.ends(isolatedline, "{") then
                        if c_bracket_on_separate_line then
                            local linecopy = isolatedline
                            while string.starts(linecopy, " ") do
                                linecopy = string.sub(linecopy, 2)
                            end
                            if linecopy ~= "{" then

                        end
                    end
            end
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

