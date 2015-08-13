<?php
//
// By: Thanh Trieu <lethanhtrieuk36@gmail.com>
// Company: Rynan Technologies
// Date: 08/13/2015
// Description: This is a client to the echo server. It will send 10 test commands, and echo the server response.
//							Run it from the command line "php client.php".
//
set_time_limit(0); 
$address = 'lethanhtrieu.byethost11.com';
$port = '80';
$fp = fsockopen($address, $port, $errno, $errstr, 300);
if(! $fp) 
{
  echo "$errstr ($errno)\n";
} 
else 
{	
  // Send 10 test message to the server
  for($i=0; $i<= 10; $i++)
  {
  	// Send message to server
  	$out = "Test #$i\r\n";
  	fwrite($fp, $out);
  	
  	// Read the response from the server
  	$str = fread($fp, 100000);
  	echo $str;
  	
  	sleep(5);
  }
  
  fclose($fp);
}
?>
