var timer = 0;

// Function to send a message to the Pebble using AppMessage API
function send_message(status, msg) {
	Pebble.sendAppMessage({'status': status, 'message': msg});
	
	// PRO TIP: If you are sending more than one message, or a complex set of messages, 
	// it is important that you setup an ackHandler and a nackHandler and call 
	// Pebble.sendAppMessage({ /* Message here */ }, ackHandler, nackHandler), which 
	// will designate the ackHandler and nackHandler that will be called upon the Pebble 
	// ack-ing or nack-ing the message you just sent. The specified nackHandler will 
	// also be called if your message send attempt times out.
}

// Called when JS is ready
Pebble.addEventListener("ready", function(e) {
	send_message(0, "Hello World!");
});
												
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", function(e) {
	console.log("Received Status: " + e.payload.status);
	console.log("Received Message: " + e.payload.message);
	if (e.payload.status == 3) {
		timer += 5;
		send_message(2, timer + " Seconds!");
	}
	else send_message(2, e.payload.message + (e.payload.status > 0 ? " pressed" : " released"));
});