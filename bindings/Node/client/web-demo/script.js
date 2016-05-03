window.onload = function() {
	(function() {
	    var canvas = document.getElementById('canvas'),
	            context = canvas.getContext('2d');

	    // resize the canvas to fill browser window dynamically
	    window.addEventListener('resize', resizeCanvas, false);

	    function resizeCanvas() {
	            canvas.width = window.innerWidth;
	            canvas.height = window.innerHeight;

	            /**
	             * Your drawings need to be inside this function otherwise they will be reset when 
	             * you resize the browser window and the canvas goes will be cleared.
	             */
	            drawStuff(); 
	    }
	    resizeCanvas();

	    var cursor = { x: 400, y: 400 };
	    var colors = ["#aa0000", "#00aa00", "#0000aa", "#aaaa00", "#aa00aa", "#00aaaa"]

	    function drawPointer(x, y, color) {
	    	context.save();
		    context.beginPath();
	    	context.moveTo(x, y);
	    	context.lineTo(x, y + 19);
	    	context.lineTo(x + 4, y + 16);
	    	context.lineTo(x + 7, y + 23);
	    	context.lineTo(x + 10, y + 21);
	    	context.lineTo(x + 7, y + 15);
	    	context.lineTo(x + 13, y + 15);
	    	context.lineTo(x, y);
		    context.closePath();
	    	context.fillStyle = color;
			context.strokeStyle = "#ffffff";
		    context.stroke();
	    	context.fill();
	    	context.restore();
	    }

	    function verifyPosition(mouse) {
	    	if (mouse.x < 0)
	    		mouse.x = 0;
	    	else if (mouse.x > canvas.width) {
	    		mouse.x = canvas.width;
	    	}
	    	if (mouse.y < 0)
	    		mouse.y = 0;
	    	else if (mouse.y > canvas.height) {
	    		mouse.y = canvas.height;
	    	}
	    }

	    function drawInfo() {
	    	context.save();
	    	context.font = "15px Arial";
	    	output.ready(function () {
		    	var text = "Display Device: " + output.bounds.size.width + " x " + output.bounds.size.height + " pixels, ";
		    	text += output.size.width.toFixed(2) + " x " + output.size.height.toFixed(2) + " mm, ";
		    	text += output.resolution.hppi.toFixed(2) + " x " + output.resolution.vppi.toFixed(2) + " PPI, ";
		    	text += output.refreshRate + " Hz";
		    	context.fillText(text, 15, 27);
	    	});
	    	for (var i = 0; i < mice.length; i++) {
	    		var input = mice[i].pointingDevice;
	    		context.fillStyle = colors[i];
	    		var y = 37 + 20*i;
				context.fillText(input.vendor + " - " + input.product, 65, y + 12);
	    		context.fillRect(15, y, 40, 15);
	    	}
	    	context.restore();
	    }

	    function drawFuncs(name, uri, x, y) {
	    	context.save();
	    	context.font = "14px Arial";
			context.fillText(name, x, y + 38);
			context.fillStyle = "#008888";
			context.fillText(uri, x, y + 53);
	    	context.restore();
	    }

	    function drawStuff() {
    		context.clearRect(0, 0, canvas.width, canvas.height);
		    drawInfo();
		    for (var i = 0; i < mice.length; i++) {
		    	verifyPosition(mice[i]);
	    		drawPointer(mice[i].x, mice[i].y, colors[i]);
	    		var fInd = (funcIndex + i) % tFuncs.length;
	    		drawFuncs(tFuncs[fInd].name, tFuncs[fInd].uri, mice[i].x, mice[i].y);
		    }
		    requestAnimationFrame(drawStuff);
	    }

	    function onMouseMove(ev) {
	    	cursor.x = ev.pageX;
	    	cursor.y = ev.pageY;
	    }

	    function onKeyDown(ev) {
	    	if (ev.keyCode == 32) {
			    for (var i = 0; i < mice.length; i++) {
		    		mice[i].x = cursor.x;
		    		mice[i].y = cursor.y;
			    }
	    	}
	    	else if (ev.keyCode == 13) {
	    		funcIndex += mice.length;
			    for (var i = 0; i < mice.length; i++) {
	    			var fInd = (funcIndex + i) % tFuncs.length;
	    			var tFunc = new pointing.TransferFunction(tFuncs[fInd].uri, mice[i].pointingDevice, output);
	    			mice[i].pointingDevice.applyTransferFunction(tFunc);
			    }
	    	}
	    }

		document.addEventListener('mousemove', onMouseMove, false);
	    document.addEventListener("keydown", onKeyDown, false);
	})();
}

var mice = [];
var manager = new pointing.PointingDeviceManager();

var output = new pointing.DisplayDevice("any:?");

var tFuncs = [
{name: "OS X El Capitan with default slider",
 uri: "osx:?setting=0.6875"}
,{name: "OS X El Capitan with slider at maximum",
 uri: "osx:?setting=3.00"}
,{name: "OS X Yosemite with default slider",
 uri: "osx:darwin-14"}
,{name: "Windows with default slider",
 uri: "windows:7?slider=0"}
,{name: "Windows with slider at maximum",
 uri: "windows:7?slider=5"}
,{name: "Windows with slider at minimum",
 uri: "windows:7?slider=-5"}
,{name: "Linux default transfer function",
 uri: "xorg:?"}
,{name: "Resolution-aware constant function",
 uri: "constant:?gain=1"}
,{name: "Naive constant function with gain 5",
 uri: "naive:?gain=5"}
,{name: "No transfer function",
 uri: "naive:?gain=1"}
,{name: "Sigmoid transfer function",
 uri: "sigmoid:?"}
];

var funcIndex = 0;

manager.addDeviceUpdateCallback(function(deviceDescriptor, wasAdded) {
	if (wasAdded) {
		var pointingDevice = new pointing.PointingDevice(deviceDescriptor.devURI);
		var mouse = {x: 400 + 20 * mice.length, y: 400, pointingDevice: pointingDevice};
		mice.push(mouse);
		var fInd = (funcIndex + mice.length) % tFuncs.length;
		var tFunc = new pointing.TransferFunction(tFuncs[fInd].uri, mouse.pointingDevice, output);
		mouse.pointingDevice.applyTransferFunction(tFunc);
		pointingDevice.setPointingCallback(function(timestamp, dx, dy, buttons) {
			mouse.x += dx;
			mouse.y += dy;
		});
	}
	else {
    	for (var i = 0; i < mice.length; i++) {
    		if (mice[i].pointingDevice.uri == deviceDescriptor.devURI) {
    			mice[i].pointingDevice.dispose();
    			mice.splice(i, 1);
    			break;
    		}
    	}
	}
});