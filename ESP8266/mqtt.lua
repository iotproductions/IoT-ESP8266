LED_RED 	=	8	-- GPIO15
LED_GREEN 	=	6	-- GPIO12
LED_BLUE	=	7	-- GPIO13
gpio.mode(LED_RED, gpio.OUTPUT)
gpio.mode(LED_GREEN, gpio.OUTPUT)
gpio.mode(LED_BLUE, gpio.OUTPUT)


gpio.write(LED_RED, gpio.HIGH)
gpio.write(LED_GREEN, gpio.LOW)
gpio.write(LED_BLUE, gpio.LOW)
print("System voltage (mV):", adc.read(0))

-- test with cloudmqtt.com
m_dis={}
function dispatch(m,t,pl)
	if pl~=nil and m_dis[t] then
		m_dis[t](m,pl)
	end
	gpio.write(LED_RED, gpio.HIGH)
	tmr.delay(400)
	gpio.write(LED_RED, gpio.LOW)
	tmr.delay(400)
end
function topic1func(m,pl)
	print("get1: "..pl)
end
function topic2func(m,pl)
	print("get2: "..pl)
end
m_dis["/topic1"]=topic1func
m_dis["/topic2"]=topic2func
-- Lua: mqtt.Client(clientid, keepalive, user, pass)
m=mqtt.Client("nodemcu1",60,"test","test123")
m:on("connect",function(m) 
	print("connection "..node.heap()) 
	m:subscribe("/topic1",0,function(m) print("sub done") end)
	m:subscribe("/topic2",0,function(m) print("sub done") end)
	m:publish("/topic1","hello",0,0) m:publish("/topic2","world",0,0)
	end )
m:on("offline", function(conn)
    print("disconnect to broker...")
    print(node.heap())
end)
m:on("message",dispatch )
-- Lua: mqtt:connect( host, port, secure, auto_reconnect, function(client) )
m:connect("192.168.1.9", 1883,0,1)
tmr.alarm(0,10000,1,function() local pl = "time: "..tmr.time() 
	val = adc.read(0)
	print("System voltage (mV):"..val)
	m:publish("LightSensor","Current ADC value: "..val..", Current timer value: "..pl ,0,0)
	gpio.write(LED_BLUE, gpio.HIGH)
	tmr.delay(400)
	gpio.write(LED_BLUE, gpio.LOW)
	tmr.delay(400)
	end)
