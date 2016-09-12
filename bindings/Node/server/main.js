#!/usr/bin/env node

if (process.argv.length < 3)
	console.log("Usage: pointingserver [start | stop]");

var arg = (process.argv.length > 2) ? process.argv[2] : 'start';
var pidPath = __dirname + '/pointing.pid';
const fs = require('fs');
const port = 3423;

if (arg == 'start') {
	const http = require('http');
	var server = http.createServer();

	server.listen(port).on('error', function(err) {
		if (err.code === 'EADDRINUSE') {
			console.log("Port " + port + " is already in use.")
			const execSync = require('child_process').execSync;
			processId = parseInt(execSync('lsof -t -i:' + port).toString('utf8'));
			console.log("Id of process using the port:", processId);
			process.exit();
		}
	});
	server.close();

	fs.access(pidPath, fs.F_OK, function(err) {
		if (!err) { // If file exists remove it
			fs.unlinkSync(pidPath);
		}
		const spawn = require('child_process').spawn;
		const stdout = fs.openSync(__dirname + '/pointing.log', 'a');
		const stderr = fs.openSync(__dirname + '/pointing.log', 'a');

		const child = spawn('nohup', ['node', __dirname + '/server.js', port, '&'], {
			detached: true,
			stdio: [ 'ignore', stdout, stderr ]
		});
		fs.writeFile(pidPath, child.pid);
		child.unref();
		console.log('PointingServer was started');
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
