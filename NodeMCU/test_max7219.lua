-- MaxMatrix
-- John Longworth July 2016
-- This code has been collected from various places and modified by myself

smiley = {0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C}
check  = {0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0}
alien  = {25  ,  58 , 109 , 250 , 250 , 109 ,  58 ,   25}
chess  = {85  , 170 ,  85 , 170 ,  85 , 170 ,  85 ,  170}
sprite = {smiley, check, alien, chess, ghost, arrow} 

DIN = 7  -- ESP8266 GPIO13 - MAX7219 DIN pin
CS  = 8  -- ESP8266 GPIO15 - MAX7219 CS pin 
CLK = 5  -- ESP8266 GPIO14 - MAX7219 CLK pin

gpio.mode(DIN, gpio.OUTPUT)
gpio.mode(CS,  gpio.OUTPUT) 
gpio.mode(CLK, gpio.OUTPUT)
------------------------------------------------------------------------
function displayFrame()
	for j=1,8 do
		gpio.write(CS, 0)
		writeByte(i)   
		writeByte(smiley[i]) 
		writeByte(i)   
		writeByte(check[i]) 
		writeByte(i)   
		writeByte(alien[i]) 
		writeByte(i)   
		writeByte(chess[i]) 
		gpio.write(CS, 0)    
		gpio.write(CS, 1)
	end
end
------------------------------------------------------------------------
function writeByte(data)     -- Write byte one bit at a time
   i=8
   while (i>0) do
      mask = bit.lshift(0x01,i-1)         
      gpio.write(CLK, 0)    
      dat = bit.band(data,mask)
      if (dat > 0) then 
         gpio.write(DIN, 1)        
      else 
         gpio.write(DIN, 0)   
      end     
      gpio.write(CLK, 1)    
      i=i-1
   end
end

function regSet(reg, value)   -- Set Register value (16 bits)
   gpio.write(CS, 0)
   writeByte(reg)   
   writeByte(value) 
	 writeByte(reg)   
   writeByte(value) 
	 writeByte(reg)   
   writeByte(value) 
	 writeByte(reg)   
   writeByte(value) 
	 
   gpio.write(CS, 0)    
   gpio.write(CS, 1)
end

function set_MAX_Registers()  -- Initialise MAX7219 registers
   regSet(0x0b, 0x07)        -- Set Scan Limit
   regSet(0x09, 0x00)        -- Set Decode Mode
   regSet(0x0c, 0x01)        -- Set Shut Down Mode (On)
   regSet(0x0f, 0x00)        -- Set Display Test (Off)
   regSet(0x0a, 0x0c)        -- Set LED Brightness (0 - 15)
   clearDisplay();
end    

function clearDisplay()          -- Clear matrix display (set all to 0)
   for j=1,8 do
      regSet(j,0)
   end
end 

function showSprite(index)    -- Display the sprite
   pict = sprite[index];
   for j=1,8 do
      print(pict[j])
      regSet(j, pict[j])
   end
end   

function loop(i)
   spIndex = (i % 6) + 1      -- Set this to number of sprites
   showSprite(spIndex)
   tmr.alarm(0, 2500, 0, function()
      loop(spIndex)
   end)
end

set_MAX_Registers()
loop(0)
