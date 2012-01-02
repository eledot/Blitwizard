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
blitwiz.random.mersenne_twister_obj = {}
blitwiz.random.mersenne_twister_obj.__index = blitwiz.random.mersenne_twister_obj

function blitwiz.random.mersenne_twister_obj:randomseed(s)
	if not s then s = seed() end
	self.mt[0] = normalize(s)
	for i = 1, 623 do
		self.mt[i] = normalize(0x6c078965 * bit_xor(self.mt[i-1], math_floor(self.mt[i-1] / 0x40000000)) + i)
	end
end

function blitwiz.random.mersenne_twister_obj:random(a, b)
	local y
	if self.index == 0 then
		for i = 0, 623 do   											
			--y = bit_or(math_floor(self.mt[i] / 0x80000000) * 0x80000000, self.mt[(i + 1) % 624] % 0x80000000)
			y = self.mt[(i + 1) % 624] % 0x80000000
			self.mt[i] = bit_xor(self.mt[(i + 397) % 624], math_floor(y / 2))
			if y % 2 ~= 0 then self.mt[i] = bit_xor(self.mt[i], 0x9908b0df) end
		end
	end
	y = self.mt[self.index]
	y = bit_xor(y, math_floor(y / 0x800))
	y = bit_xor(y, bit_and(normalize(y * 0x80), 0x9d2c5680))
	y = bit_xor(y, bit_and(normalize(y * 0x8000), 0xefc60000))
	y = bit_xor(y, math_floor(y / 0x40000))
	self.index = (self.index + 1) % 624
	if not a then return y / 0x80000000
	elseif not b then
		if a == 0 then return y
		else return 1 + (y % a)
		end
	else
		return a + (y % (b - a + 1))
	end
end

function blitwiz.random.twister(s)
	local temp = {}
	setmetatable(temp, blitwiz.random.mersenne_twister_obj)
	temp.mt = {}
	temp.index = 0
	temp:randomseed(s)
	return temp
end

--Linear Congruential Generator
blitwiz.random.linear_congruential_generator_obj = {}
blitwiz.random.linear_congruential_generator_obj.__index = linear_congruential_generator_obj

function blitwiz.random.linear_congruential_generator_obj:random(a, b)
	local y = (self.a * self.x + self.c) % self.m
	self.x = y
	if not a then return y / 0x10000
	elseif not b then
		if a == 0 then return y
		else return 1 + (y % a) end
	else
		return a + (y % (b - a + 1))
	end
end

function blitwiz.random.linear_congruential_generator_obj:randomseed(s)
	if not s then s = seed() end
	self.x = normalize(s)
end

function blitwiz.random.lcg(s, r)
	local temp = {}
	setmetatable(temp, blitwiz.random.linear_congruential_generator_obj)
	temp.a, temp.c, temp.m = 1103515245, 12345, 0x10000  --from Ansi C
	if r then
		if r == 'nr' then temp.a, temp.c, temp.m = 1664525, 1013904223, 0x10000 --from Numerical Recipes.
		elseif r == 'mvc' then temp.a, temp.c, temp.m = 214013, 2531011, 0x10000 end--from MVC
	end
	temp:randomseed(s)
	return temp
end

-- Multiply-with-carry
blitwiz.random.multiply_with_carry_obj = {}
blitwiz.random.multiply_with_carry_obj.__index = blitwiz.random.multiply_with_carry_obj

function blitwiz.random.multiply_with_carry_obj:random(a, b)
	local m = self.m
	local t = self.a * self.x + self.c
	local y = t % m
	self.x = y
	self.c = math_floor(t / m)
	if not a then return y / 0x10000
	elseif not b then
		if a == 0 then return y
		else return 1 + (y % a) end
	else
		return a + (y % (b - a + 1))
	end
end

function blitwiz.random.multiply_with_carry_obj:randomseed(s)
	if not s then s = seed() end
	self.c = self.ic
	self.x = normalize(s)
end

function blitwiz.random.mwc(s, r)
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

