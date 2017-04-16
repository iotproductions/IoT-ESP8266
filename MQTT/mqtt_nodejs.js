//16-Apri-2017: Create new project
// @reboot sudo node /home/pi/nodejs/mqtt.js
var mqtt = require('mqtt')
var schedule = require('node-schedule');
 
var client = mqtt.connect(
	{ 	
		host: '*****************', 
		port: 1873, 
		keepalive: 60000,
		username: '**********',
		password: '**********',
		protocolId: 'MQIsdp',
		protocolVersion: 3
	});

client.on('connect', function(){
    console.log('Connected to Broker');	
	var j = schedule.scheduleJob(' */5 * * * *', function(){
	  var publish_options = {
		  retain:true,
		  qos: 1
	  };	
	  client.publish('TOPIC_NAME', '\r\nDATA\r\n',publish_options);
	  console.log('Publish message to Broker');
	});
});
// Subcribe all topic
client.subscribe('#');
 
client.on('message', function (topic, message) {
  // message is Buffer
  console.log(topic.toString());
  console.log(message.toString());
  //client.end();
});

client.on('close', function () {
  // Reconnect to Broker
  client = mqtt.connect(
	{ 	
		host: '*****************', 
		port: 1873, 
		keepalive: 60000,
		username: '**********',
		password: '**********',
		protocolId: 'MQIsdp',
		protocolVersion: 3
	});
});


