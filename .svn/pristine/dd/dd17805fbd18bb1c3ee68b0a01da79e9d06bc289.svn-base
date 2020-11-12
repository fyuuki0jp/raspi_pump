/**
 * Created by pengkun on 28/12/2017.
 */
const PORT = 7777


const {unfoldRTPPacket, unfoldRTCPPacket} = require("./utils");
const EventEmitter  = require("events").EventEmitter;


const readStatus  = {
    WAITING: 0,
    READING_RTSP_CHANNEL: 1,
    READING_RTSP_LENGTH: 2,
    READING_RTP:3,
    READING_RTCP:4
}
const header = new Buffer.from([0x00,0x00,0x00,0x01]);

class H264Extractor extends EventEmitter{
    constructor(client){
        super();
        this.status = readStatus.WAITING;
        this.packageBuffer = null;
        this.readPointer = 0;
        this.rtpPackages = [];
        this._client = client;
        this._rtspLength = [];
    }

    static h264Extractor(client, data){
        var extractor = new H264Extractor(client);
        var list = [];
        list.push(header);
        list.push(client.sps);
        list.push(header);
        list.push(client.pps);
        client.emit("frames", Buffer.concat(list));
        return extractor.extract.bind(extractor);
    }

    extract(data){
        var index = 0;
        while(index != data.length) {
            if (this.status === readStatus.WAITING) {
                if (data[index++] === 0x24) {
                    //this is start , which is $ character of rtp/rtcp protocol
                    this.status = readStatus.READING_RTSP_CHANNEL;
                }
            }
            else if(this.status === readStatus.READING_RTSP_CHANNEL) {
                this.channel = data[index++];
                this.status = readStatus.READING_RTSP_LENGTH
            }
            else if(this.status === readStatus.READING_RTSP_LENGTH){
                this._rtspLength.push(data[index++]);
                if(this._rtspLength.length != 2)
                    continue;
                else {
                    var length = (this._rtspLength[0] << 8) + this._rtspLength[1];
                    this._rtspLength = [];
                }
                //there is no need for safe buffer allocated, so faster version is prefered
                this.packageBuffer = Buffer.allocUnsafe(length);
                //this is start of a rtp package
                if (this.channel === 0) {
                    this.status = readStatus.READING_RTP;
                }
                //this is start of a rtcp package
                else if (this.channel === 1) {
                    this.status = readStatus.READING_RTCP;
                }
            }
            else if (this.status === readStatus.READING_RTP || this.status === readStatus.READING_RTCP) {
                if(this.readPointer != this.packageBuffer.length)
                    this.packageBuffer[this.readPointer++] =  data[index++];
                else{
                   if(this.status === readStatus.READING_RTCP){
                        var unfolded = unfoldRTCPPacket(this.packageBuffer);
                        this._client.sendEmptyReceiverReport(1);
                    }
                    else if(this.status === readStatus.READING_RTP) {
                        var unfolded = unfoldRTPPacket(this.packageBuffer);
                        this.rtpPackages.push(unfolded.payload);
                        //the end of a frame
                        if(unfolded.marker === 1){
                            this.processH264Frame(this.rtpPackages);
                            this.rtpPackages= [];
                        }

                    }
                    this.readPointer = 0;
                    this.status = readStatus.WAITING;

                }
            }
        }
    }

    processH264Frame(rtpPackets) {
        const nals = [];
        let partialNal = [];
        for (let i = 0; i < rtpPackets.length; i++) {
            const packet = rtpPackets[i];
            const nal_header_f_bit = (packet[0] >> 7) & 0x01;
            const nal_header_nri = (packet[0] >> 5) & 0x03;
            const nal_header_type = packet[0] & 0x1F;

            if (nal_header_type >= 1 && nal_header_type <= 23) { // Normal NAL. Not fragmented
                nals.push(packet);
            } else if (nal_header_type == 24) { // Aggregation type STAP-A. Multiple NAls in one RTP Packet
                let ptr = 1; // start after the nal_header_type which was '24'
                // if we have at least 2 more bytes (the 16 bit size) then consume more data
                while (ptr + 2 < (packet.length - 1)) {
                    let size = (packet[ptr] << 8) + (packet[ptr + 1] << 0);
                    ptr = ptr + 2;
                    nals.push(packet.slice(ptr,ptr+size));
                    ptr = ptr + size;
                }
            } else if (nal_header_type == 25) { // STAP-B
                // Not supported
            } else if (nal_header_type == 26) { // MTAP-16
                // Not supported
            } else if (nal_header_type == 27) { // MTAP-24
                // Not supported
            } else if (nal_header_type == 28) { // Frag FU-A
                // NAL is split over several RTP packets
                // Accumulate them in a tempoary buffer
                // Parse Fragmentation Unit Header
                const fu_header_s = (packet[1] >> 7) & 0x01;  // start marker
                const fu_header_e = (packet[1] >> 6) & 0x01;  // end marker
                const fu_header_r = (packet[1] >> 5) & 0x01;  // reserved. should be 0
                const fu_header_type = (packet[1] >> 0) & 0x1F; // Original NAL unit header

                // Check Start and End flags
                if (fu_header_s == 1 && fu_header_e == 0) { // Start of Fragment}
                    const reconstructed_nal_type = (nal_header_f_bit << 7)
                        + (nal_header_nri << 5)
                        + fu_header_type;
                    partialNal = [];
                    partialNal.push(reconstructed_nal_type);

                    // copy the rest of the RTP payload to the temp buffer
                    for (let x=2; x< packet.length;x++) partialNal.push(packet[x]);
                }

                if (fu_header_s == 0 && fu_header_e == 0) { // Middle part of fragment}
                    for (let x=2; x< packet.length;x++) partialNal.push(packet[x]);
                }

                if (fu_header_s == 0 && fu_header_e == 1) { // End of fragment}
                    for (let x=2; x< packet.length;x++) partialNal.push(packet[x]);
                    nals.push(Buffer.from(partialNal));
                }
            } else if (nal_header_type == 29) { // Frag FU-B
                // Not supported
            }
        }

        // Write out all the NALs
        var list = []
        for (let x = 0; x < nals.length; x++) {
            list.push(header);
            list.push(nals[x]);
        }
        this._client.emit("frames", Buffer.concat(list));
    }

}

module.exports = H264Extractor.h264Extractor
