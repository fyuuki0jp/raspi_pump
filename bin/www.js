/**
 * Module dependencies.
 */

var app = require('../app');
var debug = require('debug')('rapp:server');
var http = require('http');
var socketIO = require('socket.io');
var Control = require('../build/Release/control');
var sqlite3 = require('sqlite3');
var DB = new sqlite3.Database("./DataBase.sqlite3");
var fs = require("fs");
var rl = require("readline");
var Split = require("buffer-split");
var path = require("path");

/**
 * Get port from environment and store in Express.
 */
var Version = "";


DB.each("SELECT value FROM info WHERE id='version'",function(err,res)
{
  Version = res.value;
})

var port = normalizePort('80');
app.set('port', port);

/**
 * Create HTTP server.
 */

var server = http.createServer(app);
/**
 * Listen on provided port, on all network interfaces.
 */

server.listen(port);
server.on('error', onError);
server.on('listening', onListening);

var io = socketIO.listen(server);
/**
 * Normalize a port into a number, string, or false.
 */

function normalizePort(val) {
  var port = parseInt(val, 10);

  if (isNaN(port)) {
    // named pipe
    return val;
  }

  if (port >= 0) {
    // port number
    return port;
  }

  return false;
}

/**
 * Event listener for HTTP server "error" event.
 */

function onError(error) {
  if (error.syscall !== 'listen') {
    throw error;
  }

  var bind = typeof port === 'string'
    ? 'Pipe ' + port
    : 'Port ' + port;

  // handle specific listen errors with friendly messages
  switch (error.code) {
    case 'EACCES':
      console.error(bind + ' requires elevated privileges');
      process.exit(1);
      break;
    case 'EADDRINUSE':
      console.error(bind + ' is already in use');
      process.exit(1);
      break;
    default:
      throw error;
  }
}

/**
 * Event listener for HTTP server "listening" event.
 */

function onListening() {
  var addr = server.address();
  var bind = typeof addr === 'string'
    ? 'pipe ' + addr
    : 'port ' + addr.port;
  debug('Listening on ' + bind);
}

var first = 0;
var sendlist = [];

function ReadFiles(sock,y,m,d,h)
{
  fs.readdir("./public/img/hdd/Pictures/"+y+"/"+m+"/"+d+"/"+h+"/",function(err,files){
    var count = 0;
    for(var i in files){
      if(isNaN(files[i].substr(6,2)) && !isNaN(files[i].substr(0,1)))
      {
        sock.emit("getShot",files[i]);
        count++;
      }
    }
    sock.emit("Load","complete");
    console.log("get file count = "+count);
  });
}

var walk = function(p, fileCallback, errCallback) {

    fs.readdir(p, function(err, files) {
        if (err) {
            errCallback(err);
            return;
        }

        files.forEach(function(f) {
            var fp = path.join(p, f); // to full-path
            if(fs.statSync(fp).isDirectory()) {
                walk(fp, fileCallback); // ディレクトリなら再帰
            } else {
                fileCallback(fp); // ファイルならコールバックで通知
            }
        });
    });
};

var isLicense = false;



io.sockets.on('connection',function(socket){
  if(!Control.IsLicense())
  {
    var hw = Control.GetHWKey();
    var send = JSON.stringify({
      lisence:false,
      hwcode:hw
    });
    socket.emit("license",send);
  }
  else
  {
    var hw = Control.GetHWKey();
    var send = JSON.stringify({
      lisence:true,
      hwcode:hw
    });
    socket.emit("license",send);
  }
  socket.on("request",function(data){
    if(~data.indexOf("filelist"))
    {
      walk("./public/img/hdd/Pictures/"
        ,function(path){
          var send = path.substr(6);
          var type = path.split('.');
          if ((type[type.length - 1].toLowerCase() == 'mp4' ||type[type.length-1].toLowerCase() == 'avi')&& path.indexOf("/.") == -1) {
            socket.emit("getShot",send);
            console.log(send);
          }
        },function(err){
          console.log("err = "+err);
        })
    }
    if(~data.indexOf("config"))
    {
      fs.readFile("setting.ini",'utf-8',function(err,ini){
        var json = "{";
        var file = ini.toString().split('\n');
        //console.log(ini);
        for(let i = 0; i < file.length; i++) {
          var seq = file[i].split("=");
          if(seq.length == 2){
            if(i!=0)
            json += ',';
            json += '"' + seq[0].trim()+'":"'+seq[1].trim()+'"';
          }
        }
        json += '}';
        console.log(JSON.parse(json));
        socket.emit("config",json);
      });
    }
  });
  socket.on("command",function(data){
    if(data == "triger")
    {
      Control.Triger();
    }
    if(data=="live")
    {
      console.log("live view");
      Control.Live();
    }
    if(data=="check")
    {
      console.log("check mode");
      Control.Check();
    }
  });
  socket.on("config",function(data){
    var config = JSON.parse(data);
    var write = "fps = "+config.fps
    +"\nwidth = "+config.width
    +"\nheight = "+config.height
    +"\nBefore = "+config.Before
    +"\nAfter = "+config.After
    +"\nSavePath = "+config.SavePath
    +"\nLoginUser = "+config.LoginUser
    +"\nLoginPass = "+config.LoginPass
    +"\nuseH264 = "+config.useH264
    +"\nisRising = "+config.isRising;
    +"\nisRec = "+config.isRec;
    console.log(write);
    fs.writeFileSync("setting.ini",write,'utf-8');
    Control.UpdateConf();
  });
  socket.on("license",function(data){
    if(Control.SetLicense(data))
    {
      socket.emit("license",JSON.stringify({lisence:true}))
    }
    else
    {
      socket.emit("license",JSON.stringify({lisence:false}))
    }
  })
});
const NALseparator    = new Buffer([0,0,0,1]);//NAL break

Control.Start();
Control.SetFrameBack(function(ret,frame){
  if(ret == 1)
  {
    io.sockets.emit("jpeg",frame);
  }
  else if(ret > 1)
  {
    //console.log("callback");
    //io.sockets.emit("debug",stream);
    io.sockets.emit("h264",frame);
  }
});

setInterval(()=>{
  if(io.sockets.sockets.length > 0)
  {
    if(Control.GetError())
    {
      io.sockets.emit("error",Control.GetErrorInfo());
    }
  }
},1000);

console.log("stanby ready");

process.on('SIGINT',() =>{
  Control.Stop();
  console.log("process end");
  //server.close();
  process.exit();
});

