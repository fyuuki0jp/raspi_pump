/**
 * Created by pengkun on 24/12/2017.
 */
const net = require("net");
const urlParse = require("url");
const async = require("async");
const sdpParser = require("sdp-transform");
const debug = require("debug")("rtsp-client")
const EventEmitter  = require("events").EventEmitter;
const { getMD5Hash} = require("./utils");
const  h264Extractor = require("./h264");
const WWW_AUTH_REGEX = new RegExp(/([a-z]+)=\"([^,\s]+)\"/g);

//const PORT = 8080
//const WebSocket = require("ws");
//const wss = new WebSocket.Server({ port: PORT});


class  RtspClient extends EventEmitter{
    constructor(options){
        super();
        this._url = options.url;
        this._seq = 0;
        this._headers = {"User-Agent": "oceanai/0.1.0"};
        this._session = null;
        this._supportedMethod = [];
        this._serverName = "";
        this._username = options.username;
        this._password = options.password;
        this._id = options.id || 'zelda';
        this._codec = "";
        this._nance = "";
        this._realm = "";
        this._digestRquired = false;
        this._basicRequired = false;
    }

    connect(cb) {
        var self = this
        const { hostname, port } = urlParse.parse(this._url);
        debug(`${this._id}: request host is ${hostname}, and port is ${port}`)
        this._client = net.connect(port || 554, hostname, () => {
            debug(`${this._id} successfully connect to camera server!`);
            async.series({
                 options: (done) => self.sendRequest("OPTIONS", self._headers, done),
                 describe: (done) => {
                     var headers = Object.assign({"Accept": "application/sdp"}, self._headers);
                     self.sendRequest("DESCRIBE", headers, done);
                 },
                 setup: (done) => {
                     var  headers = Object.assign({"Transport": "RTP/AVP/TCP;unicast;interleaved=0-1"}, self._headers);
                     self.sendRequest("SETUP", headers, done);

                 },
                 play: (done) =>{
                     var header = Object.assign({"Session": this._session}, this._headers);
                     this.sendRequest("PLAY", header, done);
                 }

            }, (err) => {
                if(err) {
                    debug(`${this._id}: error is ${err}`);
                    this.emit("error", err);
                }
                else {
                    debug(`${this._id}: initial stages are finished, ready to accept rtp packets`);
                    var processor = cb(this);
                    this._client.on("data", (data)=>{
                            processor(data);
                    })
                   }
            });
        });
        this._client.on("end", () =>{
            debug(`${this._id}: connection is closed`);
        });
        return this;

    }
    sendRequest(requestName, headers, cb){
        this._seq += 1;
        var rq = `${requestName} ${this._url} RTSP/1.0\r\nCSeq: ${this._seq}\r\n`;
        var authString;
        if(this._basicRequired){
            authString = 'Basic ' + new Buffer(`${this._username}:${this._password}`).toString('base64');
            headers["Authorization"] = authString;
        }
        else if (this._digestRquired){
            const ha1 = getMD5Hash(`${this._username}:${this._realm}:${this._password}`);
            const ha2 = getMD5Hash(`${requestName}:${this._url}`);
            const ha3 = getMD5Hash(`${ha1}:${this._nance}:${ha2}`);
            authString = `Digest username="${this._username}",realm="${this._realm}",nonce="${this._nance}",uri="${this._url}",response="${ha3}"`;
            headers["Authorization"] = authString;
        }
        debug(`${this._id}: ${rq}`);
        Object.keys(headers).forEach((header) => {
            rq += `${header}: ${headers[header].toString()}\r\n`;
        });
        this._client.write(`${rq}\r\n`);
        this._client.once('data', (data) =>{
            this.parseResponse(requestName, headers, data.toString(), cb);
        })
    }

    parseResponse(requestName, reqHeaders, response, cb) {
        var resArr = response.split("\r\n");
        var resHeaders = {};
        debug(`${this._id}: ${response}`)
        resArr.slice(1).forEach((line) => {
            var kv = line.split(": ");
            resHeaders[kv[0]] = kv[1]
        })
        var statusCode = resArr[0].split(" ")[1];
        if (statusCode === "200") {
            if (requestName === "OPTIONS") {
                this._servername = resHeaders["Server"];
                this._supportedMethod = resHeaders["Public"].split(", ");
            }
            else if (requestName === "DESCRIBE") {
                this._contentType = resHeaders["Content-Type"];
                var sdpInfo = sdpParser.parse(resArr.slice(11, resArr.length).join("\r\n"));
                this._codec = sdpInfo.rtp[0].codec;
                //get sps and pps info from rtsp protocol
                var fmtpInfo = sdpParser.parseFmtpConfig(sdpInfo.fmtp[0].config);
                var splitSpropParameterSets = fmtpInfo['sprop-parameter-sets'].split(',');
                var sps_base64 = splitSpropParameterSets[0];
                var pps_base64 = splitSpropParameterSets[1];
                this.sps = new Buffer(sps_base64, 'base64');
                this.pps = new Buffer(pps_base64, 'base64');
                if (this._codec != "H264")
                     cb("only h264 is supported currently");

            }
            else if (requestName === "SETUP") {
                this._session = resHeaders["Session"].split(";")[0];
            }
            cb(null, "success");
        }
        else if (statusCode === "401") {
            if (!this._username || !this._password)
                throw new Error("authentication is required while username or password is empty");
            else {
                var type = resHeaders["WWW-Authenticate"].split(" ")[0];
                var authHeaders = {};
                var match = WWW_AUTH_REGEX.exec(resHeaders["WWW-Authenticate"]);
                while (match) {
                    authHeaders[match[1]] = match[2];
                    match = WWW_AUTH_REGEX.exec(resHeaders["WWW-Authenticate"]);
                }
                // mutable, but only assigned to by if block
                if (type === "Digest") {
                    this._digestRquired = true;
                    this._nance = authHeaders.nonce;
                    this._realm = authHeaders.realm;
                } else if (type === "Basic") {
                    this._basicRequired = true;
                }
                this.sendRequest(requestName, reqHeaders, cb);
            }

        }
    }

    sendInterleavedData(channel, buffer) {
            let header = new Buffer(4);
            header[0] = 0x24; // ascii $
            header[1] = channel;
            header[2] = (buffer.length >> 8) & 0xFF;
            header[3] = (buffer.length >> 0) & 0xFF;

            const data = Buffer.concat([header,buffer]);
            this._client.write(data);
        }

    sendEmptyReceiverReport(channel) {
            let report = new Buffer(8);
            const version = 2;
            const paddingBit = 0;
            const reportCount = 0; // an empty report
            const packetType = 201; // Receiver Report
            const length = (report.length/4) - 1; // num 32 bit words minus 1
            report[0] = (version << 6) + (paddingBit << 5) + reportCount;
            report[1] = packetType;
            report[2] = (length >> 8) & 0xFF;
            report[3] = (length >> 0) & 0XFF;
            report[4] = (this.clientSSRC >> 24) & 0xFF;
            report[5] = (this.clientSSRC >> 16) & 0xFF;
            report[6] = (this.clientSSRC >> 8) & 0xFF;
            report[7] = (this.clientSSRC >> 0) & 0xFF;
            this.sendInterleavedData(channel, report);
    }


    start(){
        this.connect(h264Extractor)
        return this;
    }
}


//var client = new RtspClient({url: "rtsp://192.168.1.71:554/unicast/c1/s0/live", username:"admin", password:"123456"}).connect(h264Extractor)
//client.on("frames", (data) => {
//	console.log(data);
//})
//wss.on("connection", (ws, req) =>{
//	console.log("connected");
//      client.on("frames", (data) =>{
//          ws.send(data);
//      })
//})
module.exports = RtspClient
