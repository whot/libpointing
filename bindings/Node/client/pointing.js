/* 
 * Initial software
 * Authors: Izzatbek Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

var pointing = new function() {

	var socket = io.connect('http://localhost:3423');
	this.pointingIsAvailable = true;
	var that = this;

	socket.on('connect_error', function() {
	    console.log('Pointing Server is not running!');
		that.pointingIsAvailable = false;
	    socket.close();
	});

	socket.on('pointingError', function(errMsg) {
	    console.log('Pointing Error:', errMsg);
	});



	var DeviceManager = function() {
		this.deviceList = [];
	};
	DeviceManager.prototype = {
		addDeviceUpdateCallback: function(updateCallback) {
			this.callback = updateCallback;
			return this;
		},
		callCallback: function(deviceDescriptor, wasAdded) {
			if (this.callback) {
				this.callback(deviceDescriptor, wasAdded);
			}
		},
		deviceUpdateCallback: function(deviceDescriptor, wasAdded) {
			if (wasAdded) {
				this.deviceList.push(deviceDescriptor);
			}
			else {
				this.deviceList = this.deviceList.filter(function(desc) {
					return desc.devURI != deviceDescriptor.devURI;
				});
			}
			this.callCallback(deviceDescriptor, wasAdded);
		},
		changeDeviceList: function(ls) {
			this.deviceList = ls;
			var that = this;
			this.deviceList.forEach(function(desc) {
				that.callCallback(desc, true);
			});
		}
	};



	var PointingDeviceManager = function() {
		var that = this;
		DeviceManager.call(this);
		socket.emit('pointingDeviceList');

		socket.on('deviceUpdateCallback', function(deviceDescriptor, wasAdded) {
			that.deviceUpdateCallback(deviceDescriptor, wasAdded);
		});

		socket.on('pointingDeviceList', function(ls) {
			that.changeDeviceList(ls);
		});
	};
	PointingDeviceManager.prototype = Object.create(DeviceManager.prototype);
	PointingDeviceManager.prototype.constructor = PointingDeviceManager;



	var DisplayDeviceManager = function() {
		var that = this;
		DeviceManager.call(this);
		socket.emit('displayDeviceList');

		socket.on('displayUpdateCallback', function(deviceDescriptor, wasAdded) {
			that.deviceUpdateCallback(deviceDescriptor, wasAdded);
		});

		socket.on('displayDeviceList', function(ls) {
			that.changeDeviceList(ls);
		});
	};
	DisplayDeviceManager.prototype = Object.create(DeviceManager.prototype);
	DisplayDeviceManager.prototype.constructor = DisplayDeviceManager;



	var dispatcher = new function() {

		this.objects = {};
		var that = this;

		socket.on('pointingCallback', function(anId, timestamp, dx, dy, buttons) {
			var input = that.objects[anId];
			if(input && input.callback)
				input.callback(timestamp, dx, dy, buttons);
		});

		socket.on('pointingObjectInfo', function(anId, objInfo) {
			var obj = that.objects[anId];
			if (obj) {
				for (var el in objInfo)
					obj[el] = objInfo[el];
				obj.isReady = true;
			    if (obj.readyCallback)
			    	obj.readyCallback();
			}
		});
	}();

	var idCount = 1;



	var PointingObject = function() {
		this.id = idCount++;
		dispatcher.objects[this.id] = this;
	};
	PointingObject.prototype = {

		dispose: function() {
			socket.emit('pointingObjectDelete', this.id);
			delete dispatcher.objects[this.id];
		},

		ready: function(readyCallback) {
			if (this.isReady)
				readyCallback();
			else {
				this.readyCallback = readyCallback;
			}
		}

	};



	var DisplayDevice = function(uri) {
		this.uri = uri;
		PointingObject.call(this);
		socket.emit('displayDeviceCreate', uri, this.id);
	};
	DisplayDevice.prototype = Object.create(PointingObject.prototype);
	DisplayDevice.prototype.constructor = DisplayDevice;



	var PointingDevice = function(uri) {
		this.uri = uri;
		PointingObject.call(this);
		socket.emit('pointingDeviceCreate', uri, this.id);
	};
	PointingDevice.prototype = Object.create(PointingObject.prototype);
	PointingDevice.prototype.constructor = PointingDevice;
	PointingDevice.prototype.setPointingCallback = function(pointingCallback) {
		this.callback = pointingCallback;
		socket.emit('setPointingCallback', this.id, this.funcId, this.floating);
		return this;
	};
	PointingDevice.prototype.applyTransferFunction = function(transferFunction, floating) {
		this.funcId = transferFunction.id;
		this.floating = floating;
		if (this.callback)
			socket.emit('setPointingCallback', this.id, this.funcId, this.floating);
	};



	var TransferFunction = function(uri, inputDevice, displayDevice) {
		this.uri = uri;
		PointingObject.call(this);
		socket.emit('transferFunctionCreate', uri, this.id, inputDevice.id, displayDevice.id);
	};
	TransferFunction.prototype = Object.create(PointingObject.prototype);
	TransferFunction.prototype.constructor = TransferFunction;
	TransferFunction.prototype.clearState = function() {
		socket.emit('transferFunctionClearState', this.id);
		return this;
	};
	TransferFunction.prototype.setSubPixeling = function(subPixeling) {
		socket.emit('transferFunctionSetSubPixeling', this.id, subPixeling);
		return this;
	};
	TransferFunction.prototype.setHumanResolution = function(humanResolution) {
		socket.emit('transferFunctionHumanResolution', this.id, humanResolution);
		return this;
	};
	TransferFunction.prototype.setCardinalitySize = function(cardinality, widgetSize) {
		socket.emit('transferFunctionCardinalitySize', this.id, cardinality, widgetSize);
		return this;
	};



	var SystemPointerAcceleration = function() {
		PointingObject.call(this);
		socket.emit('systemPointerAcceleration', this.id);
	};
	SystemPointerAcceleration.prototype = Object.create(PointingObject.prototype);
	SystemPointerAcceleration.prototype.constructor = SystemPointerAcceleration;

	SystemPointerAcceleration.prototype.set = function(argsObj) {
		socket.emit('setSystemPointerAcceleration', this.id, argsObj);
	};
	SystemPointerAcceleration.prototype.get = function(argsObj) {
		socket.emit('systemPointerAcceleration', this.id, argsObj);
	};



	var pointingCursor = {
		setPosition: function(x, y) {
			socket.emit('setPointingCursor', x, y);
		},
		getPosition: function(callback) {
			socket.emit('pointingCursor');
			socket.on('pointingCursor', function(position) {
				socket.removeAllListeners('pointingCursor');
				callback(position.x, position.y);
			});
		}
	};



	// visible outside
	this.pointingCursor = pointingCursor;
	this.PointingDeviceManager = PointingDeviceManager;
	this.DisplayDeviceManager = DisplayDeviceManager;
	this.PointingDevice = PointingDevice;
	this.DisplayDevice = DisplayDevice;
	this.TransferFunction = TransferFunction;
	this.SystemPointerAcceleration = SystemPointerAcceleration;
}();