//var socket = io();
//socket.emit('pointing list');

/*if (navigator.userAgent.toLowerCase().indexOf('chrome') != -1) {
    var socket = io.connect('http://localhost:3000', {'transports': ['xhr-polling']});
  } else {
    var socket = io.connect('http://localhost:3000');
  }
*/

var manager = new pointing.PointingDeviceManager();

var leftInput, rightInput;

manager.addDeviceUpdateCallback(function(deviceDescriptor, wasAdded) {
  if (wasAdded) {
    $('#player1list').append($('<option>', {value:deviceDescriptor.devURI, text:deviceDescriptor.name}));
    $('#player2list').append($('<option>', {value:deviceDescriptor.devURI, text:deviceDescriptor.name}));
  }
});

setTimeout(function() {
  $("#player1list").prop('selectedIndex', 0);
  $("#player2list").prop('selectedIndex', 1);
  if ($("#player1list option").length > 0)
    $('#player1list').trigger('change');
  if ($("#player2list option").length > 1)
    $('#player2list').trigger('change');
}, 500);


$('#player1list').change(function() {
  if (leftInput)
    leftInput.dispose();
  leftInput = new pointing.PointingDevice($(this).val());
  leftInput.setPointingCallback(function(timestamp, dx, dy) {
    if (!window.pong.leftPaddle.auto)
      window.pong.leftPaddle.moveFor(dy);
  });
});

$('#player2list').change(function() {
  if (rightInput)
    rightInput.dispose();
  rightInput = new pointing.PointingDevice($(this).val());
  rightInput.setPointingCallback(function(timestamp, dx, dy) {
    if (!window.pong.rightPaddle.auto)
      window.pong.rightPaddle.moveFor(dy);
  });
});