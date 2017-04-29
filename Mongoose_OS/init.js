load('api_gpio.js');
load('api_mqtt.js');
load('api_sys.js');
load('api_timer.js');
load('api_rpc.js');

RPC.addHandler('set_gpio', function(args) {
  let json;
  GPIO.set_mode(args.pin, GPIO.MODE_OUTPUT);
  GPIO.write(args.pin, args.state);
  json = {pin:args.pin, state:args.state };
  return json;
});

RPC.addHandler('get_status', function(args) {
  let json,state;
  state = GPIO.read(args.get);
  json = {pin:args.get, state: state };
  return json;
});

// Helper C function get_led_gpio_pin() in src/main.c returns built-in LED GPIO
let led = ffi('int get_led_gpio_pin()')();  
// Publish to MQTT topic on a button press. Button is wired to GPIO pin 0
GPIO.set_button_handler(4, GPIO.PULL_UP, GPIO.INT_EDGE_NEG, 200, function() {
  let topic = 'smart_plug/info';
  let message = JSON.stringify({
    total_ram: Sys.total_ram(),
    free_ram: Sys.free_ram()
  });
  let ok = MQTT.pub(topic, message, message.length);
  print('Published:', ok ? 'yes' : 'no', 'topic:', topic, 'message:', message);
}, null);
