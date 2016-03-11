var NwBuilder = require('nw-builder');
var fs = require('fs');

fs.createReadStream('./libpointing/nw/Release/pointing.node').pipe(fs.createWriteStream('./nw/pointing.node'));
fs.readFile('./server/server.js', 'utf8', function (err, data) {
  if (err) {
    return console.log(err);
  }
  originalFile = data;
  var result = originalFile.replace(/require\("libpointing"\)/g, 'require("./pointing.node")');
  var result = result.replace(/#!\/usr\/bin\/env node/g, '');

  fs.writeFile('./nw/server.js', result, 'utf8', function (err) {
    if (err) return console.log(err);
    
    var nw = new NwBuilder({
        files: ['./nw/**'],// use the glob format
        platforms: ['win64'],
        version: '0.12.3'
    });

    nw.on('log', console.log);

    // Build returns a promise
    nw.build().then(function () {
      // The executable needs to be named nw.exe (This is a bug in Node Webkit)
      var dir = "./build/pointingserver/win64/";
      process.chdir(dir);
      fs.rename("pointingserver.exe", "nw.exe", function(err) {
        if (err) console.log('ERROR: ' + err);
      });
    }).catch(function (error) {
        console.error(error);
    });
  });
});
