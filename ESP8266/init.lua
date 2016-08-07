-- WIFI Config--
wifi.setmode(wifi.STATIONAP)
-- Connect to specific access point
wifi.sta.config("Thanh Trieu Production", "********")
-- AP Mode config
cfg={}
cfg.ssid="Sensor Node"
cfg.pwd="*******"
cfg.beacon=330
cfg.auth=AUTH_WPA2_PSK
cfg.channel=11
cfg.hidden=0
cfg.max=4
wifi.ap.config(cfg)

ipcfg={}
ipcfg.ip="192.168.100.10";
ipcfg.netmask="255.255.255.0";
ipcfg.gateway="192.168.100.1";
wifi.ap.setip(ipcfg);
dhcp_config ={}
dhcp_config.start = "192.168.100.2"
wifi.ap.dhcp.config(dhcp_config)

-- WiFi Connection Verification --
tmr.alarm(0, 1000, 1, function()
   if wifi.sta.getip() == nil then
      --print("Connecting to AP...\n")
   else
      ip, nm, gw=wifi.sta.getip()
      print("\r\nSTA Mode Info: \nIP Address: ",ip)
      print("Netmask: ",nm)
      print("Gateway Addr: ",gw,'\n')
	  print("\r\nAP Mode Info: \r\n",wifi.ap.getip())
	  --print(node.info())
      tmr.stop(0)
	  dofile('mqtt.lua')
   end
end)
