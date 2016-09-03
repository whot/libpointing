var player,
    time_update_interval = 0;

var sliderWidth = 600;
var handlePosX = 0;

function onYouTubeIframeAPIReady() {
    player = new YT.Player('video-placeholder', {
        width: sliderWidth,
        height: 400,
        videoId: 'WBu2pJpgKEo',
        events: {
            onReady: initialize
        }
    });
}

function initialize(){

    // Update the controls on load
    updateProgressBar();

    // Clear any old interval.
    clearInterval(time_update_interval);

    // Start interval to update elapsed time display and
    // the elapsed part of the progress bar every second.
    time_update_interval = setInterval(function () {
        updateProgressBar();
    }, 1000);


    $('#volume-input').val(Math.round(player.getVolume()));
}

// This function is called by initialize()
function updateProgressBar(){
    // Update the value of our progress bar accordingly.
    handlePosX = player.getCurrentTime() / player.getDuration() * sliderWidth;
}

window.onload = function() {
	(function() {
	    var canvas = document.getElementById('canvas'),
	        context = canvas.getContext('2d');

		function getMousePos(canvas, evt) {
			var rect = canvas.getBoundingClientRect();
			return {
			  x: evt.clientX - rect.left,
			  y: evt.clientY - rect.top
			};
		}

		var mouse = { x: -1, y: -1 };
		mouse.isInsideCanvas = false;
	    mouse.leftButtonPressed = false;

		canvas.addEventListener('mousemove', function(evt) {
			var mousePos = getMousePos(canvas, evt);
			mouse.x = mousePos.x;
			mouse.y = mousePos.y;
		}, false);

		canvas.addEventListener('mouseenter', function(evt) {
			//console.log('mouseenter');
			mouse.isInsideCanvas = true;
		}, false);
		canvas.addEventListener('mouseleave', function(evt) {
			//console.log('mouseleave');
			mouse.isInsideCanvas = false;
		}, false);

		function isInsideSlider() {
			if (mouse.isInsideCanvas == false)
				return false;
			if (mouse.x >= 0 && mouse.x < sliderWidth) {
				if (mouse.y > 20 && mouse.y < 50)
					return true;
			}
			return false;
		}


	    var fps = 60;

	    var slider = new Image();
	    slider.src = 'slider.png';
	    slider.x = 250;
	    slider.y = 150;
	    var handle = new Image();
	    handle.src = 'handle.png';

	    function drawSlider() {
			context.drawImage(slider, 0, 25, sliderWidth, 20);
			context.drawImage(handle, handlePosX - 2.5, 20, 5, 30);
	    }


	    function drawStuff() {
	    	drawSlider();
	    }

		var manager = new pointing.PointingDeviceManager();
		var output = new pointing.DisplayDevice("any:")
		var shouldMoveVideo = false;

		manager.addDeviceUpdateCallback(function(deviceDescriptor, wasAdded) {
			if (wasAdded) {
				var pointingDevice = new pointing.PointingDevice(deviceDescriptor.devURI + "?cpi=400");
				var transferFunction = new pointing.TransferFunction("osx:?debugLevel=2", pointingDevice, output);
				transferFunction.setSubPixeling(true).setCardinalitySize(27 * 3600, sliderWidth);
				pointingDevice.applyTransferFunction(transferFunction, true);

				pointingDevice.setPointingCallback(function(timestamp, dx, dy, buttons) {
					if ((buttons & 1) == 1) {
						if (isInsideSlider()) {
							document.body.style.cursor = 'none';
							shouldMoveVideo = true;
						}
					}
					else {
						document.body.style.cursor = 'default';
						shouldMoveVideo = false;
					}
					if (shouldMoveVideo) {
						if (dx) {
							console.log(dx);
							handlePosX += dx;
							if (handlePosX > sliderWidth)
								handlePosX = sliderWidth;
							if (handlePosX < 0)
								handlePosX = 0;
						}
						var newTime = player.getDuration() * handlePosX / sliderWidth; // Skip video to new time.
    					player.seekTo(newTime);
					}
				});
			}
		});

	    var loop = setInterval(function() {
    		context.clearRect(0, 0, canvas.width, canvas.height);
    		drawStuff();
		}, 1000/fps);

	})();
}