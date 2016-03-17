#!/usr/bin/env node

if (process.argv.length < 3)
	console.log("Usage: pointingserver [start | stop]");

var arg = (process.argv.length > 2) ? process.argv[2] : 'start';
var pidPath = __dirname + '/pointing.pid';
const fs = require('fs');

if (arg == 'start') {
	fs.access(pidPath, fs.F_OK, function(err) {
	    if (err) { // If file does not exist
			const spawn = require('child_process').spawn;
			const stdout = fs.openSync(__dirname + '/pointing.log', 'a');
			const stderr = fs.openSync(__dirname + '/pointing.log', 'a');

			const child = spawn('nohup', [__dirname + '/server.js', '&'], {
			 detached: true,
			 stdio: [ 'ignore', stdout, stderr ]
			});
			fs.writeFile(pidPath, child.pid);
			child.unref();
			console.log('PointingServer was started');
		}
    });
}
if (arg == 'stop') {
	fs.access(pidPath, fs.F_OK, function(err) {
		if (!err) {
			var pid = parseInt(fs.readFileSync(pidPath, 'utf8'));
			fs.unlinkSync(pidPath);
			try {
				process.kill(pid);
				console.log('PointingServer was stopped');
			}
			catch (ex) {
				if (ex.code == 'ESRCH') {
					console.log('PointingServer is not running');
				}
			}
		}
	});
}
