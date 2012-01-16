--[[------------------------------------
blitwiz.random, which is slightly modified RandomLua v0.3.1
Pure Lua Pseudo-Random Numbers Generator
Under the MIT license.

Copyright (C) 2011 linux-man
Copyright (C) 2012 Jonas Thiem <jonas.th@web.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--]]------------------------------------

local math_floor = math.floor

local function normalize(n) --keep numbers at (positive) 32 bits
	return n % 0x80000000
end

local function bit_and(a, b)
	local r = 0
	local m = 0
	for m = 0, 31 do
		if (a % 2 == 1) and (b % 2 == 1) then r = r + 2^m end
		if a % 2 ~= 0 then a = a - 1 end
		if b % 2 ~= 0 then b = b - 1 end
		a = a / 2 b = b / 2
	end
	return normalize(r)
end

local function bit_or(a, b)
	local r = 0
	local m = 0
	for m = 0, 31 do
		if (a % 2 == 1) or (b % 2 == 1) then r = r + 2^m end
		if a % 2 ~= 0 then a = a - 1 end
		if b % 2 ~= 0 then b = b - 1 end
		a = a / 2 b = b / 2
	end
	return normalize(r)
end

local function bit_xor(a, b)
	local r = 0
	local m = 0
	for m = 0, 31 do
		if a % 2 ~= b % 2 then r = r + 2^m end
		if a % 2 ~= 0 then a = a - 1 end
		if b % 2 ~= 0 then b = b - 1 end
		a = a / 2 b = b / 2
	end
	return normalize(r)
end

local function seed()
	--return normalize(tonumber(tostring(os.time()):reverse()))
	return normalize(os.time())
end

blitwiz.random = {}

--Mersenne twister
blitwiz.random.mersenne_twister = {}
function blitwiz.random.mersenne_twister.randomseed(twister, s)
	if not s then s = seed() end
	twister.mt[0] = normalize(s)
	for i = 1, 623 do
		twister.mt[i] = normalize(0x6c078965 * bit_xor(twister.mt[i-1], math_floor(twister.mt[i-1] / 0x40000000)) + i)
	end
end

function blitwiz.random.mersenne_twister.random(twister, a, b)
	local y
	if twister.index == 0 then
		for i = 0, 623 do   											
			--y = bit_or(math_floor(twister.mt[i] / 0x80000000) * 0x80000000, twister.mt[(i + 1) % 624] % 0x80000000)
			y = twister.mt[(i + 1) % 624] % 0x80000000
			twister.mt[i] = bit_xor(twister.mt[(i + 397) % 624], math_floor(y / 2))
			if y % 2 ~= 0 then twister.mt[i] = bit_xor(twister.mt[i], 0x9908b0df) end
		end
	end
	y = twister.mt[twister.index]
	y = bit_xor(y, math_floor(y / 0x800))
	y = bit_xor(y, bit_and(normalize(y * 0x80), 0x9d2c5680))
	y = bit_xor(y, bit_and(normalize(y * 0x8000), 0xefc60000))
	y = bit_xor(y, math_floor(y / 0x40000))
	twister.index = (twister.index + 1) % 624
	if not a then return y / 0x80000000
	elseif not b then
		if a == 0 then return y
		else return 1 + (y % a)
		end
	else
		return a + (y % (b - a + 1))
	end
end

function blitwiz.random.mersenne_twister.new(s)
	local temp = {}
	temp.mt = {}
	temp.index = 0
	blitwiz.random.mersenne_twister.randomseed(temp, s)
	return temp
end

--Linear Congruential Generator
blitwiz.random.linear_congruential_generator = {}

function blitwiz.random.linear_congruential_generator.random(gen, a, b)
	local y = (gen.a * gen.x + gen.c) % gen.m
	gen.x = y
	if not a then return y / 0x10000
	elseif not b then
		if a == 0 then return y
		else return 1 + (y % a) end
	else
		return a + (y % (b - a + 1))
	end
end

function blitwiz.random.linear_congruential_generator.randomseed(gen, s)
	if not s then s = seed() end
	gen.x = normalize(s)
end

function blitwiz.random.linear_congruential_generator.new(s, r)
	local temp = {}
	temp.a, temp.c, temp.m = 1103515245, 12345, 0x10000  --from Ansi C
	if r then
		if r == 'nr' then temp.a, temp.c, temp.m = 1664525, 1013904223, 0x10000 --from Numerical Recipes.
		elseif r == 'mvc' then temp.a, temp.c, temp.m = 214013, 2531011, 0x10000 end--from MVC
	end
	blitwiz.random.linear_congruential_generator.randomseed(temp, s)
	return temp
end

-- Multiply-with-carry
blitwiz.random.multiply_with_carry = {}

function blitwiz.random.multiply_with_carry.random(mcarry, a, b)
	local m = mcarry.m
	local t = mcarry.a * mcarry.x + mcarry.c
	local y = t % m
	mcarry.x = y
	mcarry.c = math_floor(t / m)
	if not a then return y / 0x10000
	elseif not b then
		if a == 0 then return y
		else return 1 + (y % a) end
	else
		return a + (y % (b - a + 1))
	end
end

function blitwiz.random.multiply_with_carry.randomseed(mcarry, s)
	if not s then s = seed() end
	mcarry.c = mcarry.ic
	mcarry.x = normalize(s)
end

function blitwiz.random.multiply_with_carry.new(s, r)
	local temp = {}
	setmetatable(temp, blitwiz.random.multiply_with_carry_obj)
	temp.a, temp.c, temp.m = 1103515245, 12345, 0x10000  --from Ansi C
	if r then
		if r == 'nr' then temp.a, temp.c, temp.m = 1664525, 1013904223, 0x10000 --from Numerical Recipes.
		elseif r == 'mvc' then temp.a, temp.c, temp.m = 214013, 2531011, 0x10000 end--from MVC
	end
	temp.ic = temp.c
	temp:randomseed(s)
	return temp
end

function blitwiz.random.generator(name, ...)
	if name == nil or name == "mwc" then
		return blitwiz.random.mwc(unpack(arg))
	elseif name == "twister" then
		return blitwiz.random.twister(unpack(arg))
	elseif name == "lcg" then
		return blitwiz.random.lcg(unpack(arg))
	else
		error ("Random generator \"" .. name .. "\" isn't supported or known")
	end
end

