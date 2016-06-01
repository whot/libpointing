#!/usr/bin/env node
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

var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var pointing = require("libpointing");

var manager = new pointing.PointingDeviceManager();
var dManager = new pointing.DisplayDeviceManager();

/*
app.use('/static', express.static('static'));
app.get('/', function(req, res) {
  res.sendFile(__dirname + '/index.html');
});
*/

function logPointingDevice(desc, wasAdded) {
  console.log(new Date().toString() + ': device ' + (wasAdded ? 'added' : 'removed'));
  console.log('\twith URI: ' + desc.devURI);
  console.log('\twith vendor: ' + desc.vendor + ' and product: ' + desc.product);
  console.log('\twith vendorID: ' + desc.vendorID + ' and productID: ' + desc.productID);
}

function logDisplayDevice(desc, wasAdded) {
  console.log(new Date().toString() + ': device ' + (wasAdded ? 'added' : 'removed'));
  console.log('\twith name: ' + desc.name + ' and URI: ' + desc.devURI);
}

manager.addDeviceUpdateCallback(function(desc, wasAdded) {
  io.emit('deviceUpdateCallback', desc, wasAdded);
  logPointingDevice(desc, wasAdded);
});

dManager.addDeviceUpdateCallback(function(desc, wasAdded) {
  io.emit('displayUpdateCallback', desc, wasAdded);
  logDisplayDevice(desc, wasAdded);
});

io.on('connection', function(socket) {

  var objects = {};

  socket.on('error', function (err) {
    socket.emit('pointingError', err.message);
    console.log("Error:", err.message);
  });

  socket.on('pointingDeviceList', function() {
    socket.emit('pointingDeviceList', manager.deviceList);
  });

  socket.on('displayDeviceList', function() {
    socket.emit('displayDeviceList', dManager.deviceList);
  });

  socket.on('pointingDeviceCreate', function(uri, id) {
    var input = new pointing.PointingDevice(uri ? uri : "any:");
    objects[id] = input;
    socket.emit('pointingObjectInfo', id, input);
  });

  socket.on('displayDeviceCreate', function(uri, id) {
    var output = new pointing.DisplayDevice(uri ? uri : "any:");
    objects[id] = output;
    socket.emit('pointingObjectInfo', id, output);
  });

  socket.on('transferFunctionCreate', function(uri, id, inputId, outputId) {
    var tf = new pointing.TransferFunction(uri, objects[inputId], objects[outputId]);
    objects[id] = tf;
    socket.emit('pointingObjectInfo', id, tf);
  });

  socket.on('transferFunctionClearState', function(id) {
    var tf = objects[id];
    tf.clearState();
  });

  socket.on('transferFunctionSetSubPixeling', function(id, subPixeling) {
    var tf = objects[id];
    tf.setSubPixeling(subPixeling);
    socket.emit('pointingObjectInfo', id, tf);
  });

  socket.on('transferFunctionHumanResolution', function(id, humanResolution) {
    var tf = objects[id];
    tf.setHumanResolution(humanResolution);
    socket.emit('pointingObjectInfo', id, tf);
  });

  socket.on('transferFunctionCardinalitySize', function(id, cardinality, widgetSize) {
    var tf = objects[id];
    tf.setCardinalitySize(cardinality, widgetSize);
    socket.emit('pointingObjectInfo', id, tf);
  });

  socket.on('setPointingCallback', function(id, funcId, floating) {
    var input = objects[id];
    if (funcId) {
      var func = objects[funcId];
      input.setPointingCallback(function(timestamp, dx, dy, buttons) {
        var result;
        if (floating || func.subPixeling) {
          result = func.applyd(dx, dy);
        }
        else {
          result = func.applyi(dx, dy);
        }
        socket.emit('pointingCallback', id, timestamp, result.dx, result.dy, buttons);
      });
    }
    else {
      input.setPointingCallback(function(timestamp, dx, dy, buttons) {
        socket.emit('pointingCallback', id, timestamp, dx, dy, buttons);
      });
    }
  });

  socket.on('pointingObjectDelete', function(id) {
    var obj = objects[id];
    if (obj instanceof pointing.PointingDevice)
      objects[id].setPointingCallback();
    delete objects[id];
  });
  
  socket.on('systemPointerAcceleration', function(id, argsObj) {
    var acc = new pointing.SystemPointerAcceleration();
    if (argsObj)
      socket.emit('pointingObjectInfo', id, acc.get(argsObj));
    else
      socket.emit('pointingObjectInfo', id, acc.get());
  });

  socket.on('setSystemPointerAcceleration', function(id, argsObj) {
    var acc = new pointing.SystemPointerAcceleration();
    socket.emit('pointingObjectInfo', id, acc.set(argsObj));
  });

  socket.on('setPointingCursor', function(x, y) {
    pointing.pointingCursor.setPosition(x, y);
  });

  socket.on('pointingCursor', function() {
    socket.emit('pointingCursor', pointing.pointingCursor.getPosition());
  });

  socket.on("disconnect", function() {
    for (var id in objects) {
      obj = objects[id];
      if (obj instanceof pointing.PointingDevice)
        obj.setPointingCallback(); // Unset callbacks
    }
    //objects = {};
  });
});

http.listen(3423, function() {
  console.log(new Date().toString() + ': listening on *:3423\n');
}).on( 'error', function (e) { 
  if (e.code == 'EADDRINUSE') { 
    console.log(new Date().toString() + ': specified port is already being used');
    process.exit();
  }
});
