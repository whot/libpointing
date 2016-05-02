var pointing = require('./libpointing/build/Release/pointing.node');

if (process.argv.length < 5)
	console.log("Usage: node", process.argv[1], "[inputdeviceURI [outputdeviceURI [transferfunctionURI]]]");

var input = new pointing.PointingDevice((process.argv[2]) ? process.argv[2] : "any:");
var output = new pointing.DisplayDevice((process.argv[3]) ? process.argv[3] : "any:");
var tFunc = new pointing.TransferFunction((process.argv[4]) ? process.argv[4] : "system:", input, output);

input.setPointingCallback(function(timestamp, dx, dy, buttons) {
	var pixels = tFunc.applyi(dx, dy, timestamp);
	console.log(timestamp, dx, dy, buttons, " -> ", pixels.dx, pixels.dy);
});

var manager = new pointing.PointingDeviceManager().addDeviceUpdateCallback(
	function(deviceDescriptor, wasAdded) {
		console.log(deviceDescriptor, wasAdded);
	}
);