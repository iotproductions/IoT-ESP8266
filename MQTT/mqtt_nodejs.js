var mqtt = require('mqtt'); 

var client = mqtt.connect(
	{ 	
		host: 'localhost', 
		port: 1883, 
		keepalive: 10000,
		username: '**********',
		password: '**********',
		protocolId: 'MQIsdp',
		protocolVersion: 3
	});

client.on('connect', function(){
    console.log('Connected to Broker');
});
client.subscribe('#');
 
client.on('message', function (topic, message) {
  // message is Buffer
  console.log(topic.toString());
  console.log(message.toString());
  client.end();
});
