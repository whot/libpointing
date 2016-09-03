window.onload = function() {
	(function() {
	    var canvas = document.getElementById('canvas'),
	        context = canvas.getContext('2d');

	    var fps = 60;

	    var balloonsSet = [BalloonRed, BalloonGreen, BalloonYellow];
	    var balloons = [];

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

	    function addBalloon() {
	    	var index = randomInt(0, balloonsSet.length - 1);
	    	var balloon = new balloonsSet[index]();
	    	balloon.sizeC = 1. / (Math.random() + 2);
	    	var xPos = randomInt(0, canvas.width - balloon.sizeC * imgBalloonWidth);
	    	balloon.x = xPos;
	    	balloon.y = canvas.height;
	    	balloons.push(balloon);
	    }

	    function drawBalloons() {
	    	balloons.forEach(function (balloon, i, balloons) {
				context.drawImage(balloon.img, balloon.x, balloon.y,
					balloon.sizeC * imgBalloonWidth, balloon.sizeC * imgBalloonHeight);
				balloon.y -= balloon.speed;
	    	});
	    	balloons = balloons.filter(function(balloon) {
	    		return balloon.y > -imgBalloonHeight * balloon.sizeC;
	    	});
	    }

	    function checkClicks() {
	    	mice.forEach(function (mouse, i, mice) {
	    		if (mouse.buttons > 0) {
	    			balloons = balloons.filter(function(balloon) {
	    				var result = balloon.contains(mouse.x, mouse.y);
	    				if (result)
	    					mouse.score += balloon.score;
	    				return !result;
	    			});
	    		}
	    	});
	    }

	    var frameDelay = 0;
	    function checkBalloonsDensity() {
	    	if (balloons.length < 10 && frameDelay-- < 0) {
	    		addBalloon();
	    		frameDelay = randomInt(1, 30);
	    	}
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


	    function drawMice() {
    		context.save();
	    	mice.forEach(function (mouse, i, mice) {
	    		verifyPosition(mouse);
	    		context.beginPath();
				context.moveTo(mouse.x - 20, mouse.y);
				context.lineTo(mouse.x + 20, mouse.y);
				context.stroke();
	    		context.beginPath();
				context.moveTo(mouse.x, mouse.y - 20);
				context.lineTo(mouse.x, mouse.y + 20);
				context.stroke();
	    	});
			context.restore();
	    }


	    function drawPoints() {
    		context.save();
	    	mice.forEach(function (mouse, i, mice) {
	    		context.font = "20px Arial";
	    		var text = "Player with " + mouse.name + ": " + mouse.score;
				context.strokeText(text, 10, 25 + 25 * i);
	    	});
			context.restore();
	    }


	    function drawStuff() {
	    	checkBalloonsDensity()
	    	drawBalloons();
	    	drawMice();
	    	drawPoints();
	    }

	    var loop = setInterval(function() {
    		context.clearRect(0, 0, canvas.width, canvas.height);
    		drawStuff();
    		checkClicks();
		}, 1000/fps);

	})();
}

var mice = [];
var manager = new pointing.PointingDeviceManager();

manager.addDeviceUpdateCallback(function(deviceDescriptor, wasAdded) {
	if (wasAdded) {
		var pointingDevice = new pointing.PointingDevice(deviceDescriptor.devURI);
		var mouse = {score: 0, x: 400 + 20 * mice.length, y: 400, pointingDevice: pointingDevice};
		mice.push(mouse);
		pointingDevice.setPointingCallback(function(timestamp, dx, dy, buttons) {
			mouse.x += dx;
			mouse.y += dy;
			mouse.buttons = buttons;
			mouse.name = pointingDevice.vendor + " " + pointingDevice.product;
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