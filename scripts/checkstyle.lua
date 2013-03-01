
-- Blitwizard style checker
-- *----------------------*

-- Some style check options:

c_indent_spaces = 4
c_bracket_on_separate_line = false
c_allow_for_loops = false
lua_indent_spaces = 4
lua_use_semicolon = false

-- End of options.

luakeywords = { "function", "do", "end", "for", "while", "if", "then", "else", "elseif", "or", "and", "break", "return", "local" }
ckeywords = { "else", "const", "char", "int", "long", "struct", "short", "signed", "unsigned", "float", "double", "if", "switch", "case", "return", "continue", "break", "extern", "static" }

args = {...}
styleerror = false

function error_whitespace(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " ends with whitespace(s)")
end

function error_linecommentspace(file, nr, language)
    styleerror = true
    local linecomment = "//"
    if language ~= nil and string.lower(language) == "lua" then
        linecomment = "--"
    end
    print("Style: line " .. file .. ":" .. nr .. " should have a whitespace between " .. linecomment .. " and the comment text following it")
end

function error_linecommentspace2(file, nr, language)
    styleerror = true
    local linecomment = "//"
    if language ~= nil and string.lower(language) == "lua" then
        linecomment = "--"
    end
    print("Style: line " .. file .. ":" .. nr .. " should have a whitespace between " .. linecomment .. " and the code it follows")
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

function error_multiplecommandsonelineafterbracket(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " has multiple commands or statements in a single line, please put everything after { in a new line")
end

function error_multiplecommandsonelineaftersemicolon(file, nr)
    styleerror = true
    print("Style: line " .. file .. ":" .. nr .. " has multiple commands or statements in a line, please put everything after ; in a new line")
end

function error_expectedwhitespace(file, nr, char, after)
    styleerror = true
    local beforeafter = "before"
    if after then
        beforeafter = "after"
    end
    print("Style: line " .. file .. ":" .. nr .. " should have a whitespace " .. beforeafter .. " " .. char)
end

function error_tabs(file)
    styleerror = true
    print("Style: file " .. file .. " contains tab characters. Please use space for indentation and \\t for tabs in strings")
end

function checkcommentsandstrings(file, nr, language, line, insideblockcomment, insidestring, blockcommentlength, tabfound)
    -- Enable this for debugging:
    -- print("checkcommentsandstrings \"" .. line .. "\", " .. tostring(insideblockcomment) .. ", " .. tostring(insidestring) .. ", " .. tostring(blockcommentlength))
    if tabfound == nil then
        tabfound = false
    end

    -- check for tab characters:
    if string.find(line, "\t", 1, true) ~= nil and not tabfound then
        tabfound = true
        error_tabs(file)
    end

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
                local nextchar = string.sub(line, linecommentstart + #"//",
                linecommentstart + #"//")
                if nextchar ~= " " and nextchar ~= "\t" and
                (language ~= "c" or nextchar ~= "/") then
                    error_linecommentspace(file, nr, language)
                end
            end

            -- check if a space is before the line comment:
            if linecommentstart > 1 then
                local firstchar = string.sub(line, linecommentstart - 1, linecommentstart - 1)
                if firstchar ~= " " and firstchar ~= "\t" then
                    error_linecommentspace2(file, nr, language)
                end
            end

            -- return line up to the comment
            if linecommentstart > 1 then
                return string.sub(line, 1, linecommentstart-1), false, false, 0, tabfound
            else
                return "", false, false, 0, tabfound
            end
        end

        if linestringstart < blockcommentstart then
            -- string starts before block comment -> check remaining string
            local remainingstring = ""
            remainingstring,insideblockcomment,insidestring,blockcommentlength = checkcommentsandstrings(file, nr, language, string.sub(line, linestringstart+1), false, true, 0, tabfound)
            return string.sub(line, 1, linestringstart) .. remainingstring, insideblockcomment, insidestring, 0, tabfound
        end

        if blockcommentstart < 1/0 then
            -- we got a block comment -> check remaining string
            local remainingstring = ""
            remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, language, string.sub(line, blockcommentstart + #"/*"), true, false, 1, tabfound)
            return string.sub(line, 1, blockcommentstart - 1) .. remainingstring, insideblockcomment, insidestring, blockcommentlength, tabfound
        end
        return line, false, false, 0, tabfound
    else
        if insidestring ~= false then
            local stringend = string.find(line, "\"", 1, true)

            -- deal with \" occurances:
            if stringend > 1 then
                if line[stringend-1] == "\\" then
                    -- escaped, -> ignore
                    local remainingstring = ""
                    remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, language, string.sub(line, stringend), false, true, 0, tabfound)
                    return string.sub(line, 1, stringend - 1) .. remainingstring, insideblockcomment, insidestring, 0, tabfound
                end
            end

            -- deal with valid " string end:
            if stringend ~= nil then
                local remainingstring = ""
                remainingstring,insideblockcomment,insidestring = checkcommentsandstrings(file, nr, language, string.sub(line, stringend+1), false, false, 0, tabfound)
                return string.sub(line, 1, stringend) .. remainingstring, insideblockcomment, insidestring, 0
            end
            return line, false, true, nil, tabfound
        else
            local commentend = string.find(line, "*/", 1, true)
            if commentend ~= nil then
                -- check for short block comments (should be line comments!)
                if blockcommentlength <= 2 then
                    error_shortblockcomment(file, nr)
                end

                local remainingstring = ""
                remainingstring,insideblockcomment,insidestring,blockcommentlength = checkcommentsandstrings(file, nr, language, string.sub(line, commentend + #"*/"), false, false, blockcommentlength, tabfound)
                return remainingstring, insideblockcomment, insidestring, blockcommentlength, tabfound
            else
                return "", true, false, blockcommentlength + 1, tabfound
            end
        end
    end
end

function checkfile(file)
    local n = 1

    -- variables used for checkcommentsandstrings()
    local insideblockcomment = false
    local insidestring = false
    local blockcommentlength = 0

    -- variables used for char per char check
    local cpc_insidestring = false
    local cpc_bracketcount = 0
    local cpc_ininitialiser = false
    local tabfound = false

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

        -- check language:
        local language = "c"

        -- get line without line comments, block comments
        local isolatedline = ""
        isolatedline,insideblockcomment,insidestring,blockcommentlength,tabfound = checkcommentsandstrings(file, n, language, line, insideblockcomment, insidestring, blockcommentlength, tabfound)
        while string.ends(isolatedline, " ") do
            if #isolatedline > 1 then
                isolatedline = string.sub(isolatedline, 1, #isolatedline - 1)
            else
                isolatedline = ""
            end
        end

        -- check if preprocessor line (C)
        local cpc_preprocessorline = false
        if language == "c" then
            local linecopy = isolatedline
            while string.starts(linecopy, " ") do
                linecopy = string.sub(linecopy, 2)
            end
            if string.starts(linecopy, "#") then
                cpc_preprocessorline = true
            end
        end

        -- evaluate line char per char for () brackets and strings
        local i = 1
        while i <= #isolatedline do
            if cpc_insidestring == false then
                if string.sub(isolatedline, i, i) == "\"" then
                    -- check for " string start

                    -- make sure it isn't escaped:
                    local escaped = false
                    if i > 1 then
                        if string.sub(isolatedline, i-1, i-1) == "\\" then
                            escaped = true
                        end
                    end
                    
                    -- unescaped string start:
                    if escaped == false then
                        cpc_insidestring = "\""
                    end
                elseif string.sub(isolatedline, i, i) == "(" then
                    -- Check for opening brackets
                    cpc_bracketcount = cpc_bracketcount + 1
                elseif string.sub(isolatedline, i, i) == ")" then
                    -- Check for closing brackets
                    cpc_bracketcount = math.max(0, cpc_bracketcount - 1)
                elseif string.sub(isolatedline, i, i) == "=" and cpc_bracketcount == 0 then
                    -- Assignment line with possible initialiser

                    -- Make sure it's not an equality check of some sort:
                    local equality = false
                    if i > 1 then -- avoid !=, <= and >=, ==
                        local char = string.sub(isolatedline, i - 1, i - 1)
                        if char == "!" or char == "<" or char == ">" or char == "=" or char == "^" or char == "~" or char == "|" then
                            equality = true
                        end
                    end
                    if i < #isolatedline then -- avoid ==
                        if string.sub(isolatedline, i + 1, i + 1) == "=" then
                            equality = true
                        end
                    end

                    if equality == false then
                        -- it wasn't some equality check but a true assignment
                        cpc_ininitialiser = true
                    end
                elseif (string.sub(isolatedline, i, i) == ";" and cpc_bracketcount == 0) then
                    -- End of command, terminate our initialiser scope
                    cpc_ininitialiser = false
                elseif ((string.sub(isolatedline, i, i) == "{" and cpc_ininitialiser == false) or (string.sub(isolatedline, i, i) == ";" and cpc_bracketcount == 0)) then
                    -- Check for opening scopes followed by code
                    -- or terminatted lines followed by code
                    -- => multiple commands in one line which is bad

                    -- Check if there is a command in the followup:
                    local followedbynonspace = false
                    local j = i + 1
                    while j <= #isolatedline do
                        if string.sub(isolatedline, j, j) ~= " "
                        and string.sub(isolatedline, j, j) ~= "\t" then
                            followedbynonspace = true
                            break
                        end
                        j = j + 1
                    end

                    if followedbynonspace == true then
                        if string.sub(isolatedline, i, i) == "{" then
                            error_multiplecommandsonelineafterbracket(file, n)
                        else
                            error_multiplecommandsonelineaftersemicolon(file, n)
                        end
                    end
                end
            else
                -- check for end of string
                if i + #cpc_insidestring - 1 <= #isolatedline then
                    if string.sub(isolatedline, i, i + #cpc_insidestring - 1) == cpc_insidestring then
                        i = i + #cpc_insidestring - 1
                        cpc_insidestring = false
                    end
                end
            end
            i = i + 1
        end

        -- check line for various obvious problems:
        if #isolatedline > 0 then
            if insidestring == false then
                if language == "c" then
                    -- Check for bracket on separate line
                    if string.ends(isolatedline, "{") then
                        if c_bracket_on_separate_line then
                            -- Bracket SHALL be isolated
                            local linecopy = isolatedline
                            while string.starts(linecopy, " ") do
                                linecopy = string.sub(linecopy, 2)
                            end
                            if linecopy ~= "{" then
                                error_bracketnotonseparateline(file, n)
                            end
                        end
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

