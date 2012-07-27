
-- create a static and a movable object:
local obj1 = blitwiz.physics.createStaticObject()
local obj2 = blitwiz.physics.createMovableObject()
blitwiz.physics.setShapeOval(obj1, 3, 2)
blitwiz.physics.setShapeRectangle(obj2, 5, 4)

-- a ray is also generating new references and affecting garbage collection:
blitwiz.physics.ray(-10, 0, 0, 0)

-- collect garbage until something happens
print("Physics GC test phase 1/2")
local i = 1
while i < 1000 do
    collectgarbage()
    i = i + 1
end

-- Now nil the references explicitely:
obj1 = nil
obj2 = nil

-- collect more garbage
print("Physics GC test phase 2/2")
local i = 1
while i < 1000 do
    collectgarbage()
    i = i + 1
end
os.exit(0)

