function imgCreate(filename) {
	var img = new Image();
	img.src = filename;
	return img;
}
var imgBallonHeadHeight = 256;
var imgBalloonWidth = 226;
var imgBalloonHeight = 640;

function random(min, max) {
	return Math.random() * (max - min) + min;
}

function randomInt(min, max) {
	return Math.floor(Math.random() * (max - min + 1)) + min;
}

var imgRed = imgCreate('img/balloon-red.png');
var imgGreen = imgCreate('img/balloon-green.png');
var imgYellow = imgCreate('img/balloon-yellow.png');

var Balloon = function(img, score, speed) {
	this.score = score;
	this.img = img;
	this.speed = speed;
}
Balloon.prototype = {
	contains: function(x, y) {
		if (x < this.x || y < this.y)
			return false;
		if (x > this.x + imgBalloonWidth * this.sizeC)
			return false;
		if (y > this.y + imgBallonHeadHeight * this.sizeC)
			return false;
		return true;
	}
}



var BalloonRed = function() {
	var speed = random(10, 18);
	Balloon.call(this, imgRed, 3, speed);
}
BalloonRed.prototype = Object.create(Balloon.prototype);
BalloonRed.prototype.constructor = BalloonRed;



var BalloonGreen = function() {
	var speed = random(6, 10);
	Balloon.call(this, imgGreen, 2, speed);
}
BalloonGreen.prototype = Object.create(Balloon.prototype);
BalloonGreen.prototype.constructor = BalloonGreen;



var BalloonYellow = function() {
	var speed = random(3, 6);
	Balloon.call(this, imgYellow, 1, speed);
}
BalloonYellow.prototype = Object.create(Balloon.prototype);
BalloonYellow.prototype.constructor = BalloonYellow;