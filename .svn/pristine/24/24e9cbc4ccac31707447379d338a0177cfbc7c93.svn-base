/**
 * Created by pengkun on 25/12/2017.
 */
const { createHash } = require("crypto");

function unfoldRTPPacket(buffer) {
    const version =  buffer[0] >> 6;
    const hasPadding = buffer[0] >> 5 & 0x1;
    const hasExtensions = (buffer[0] >> 4) & 0x1;
    const csrc_count = buffer[0] & 0x0F;
    const marker = buffer[1] >> 7;
    const payloadType =  buffer[1] & 0x3F;
    const sequence = buffer.readUInt16BE(2);
    const timestamp =  buffer.readUInt32BE(4);
    const ssrc = buffer.readUInt32BE(8);

    const payload = buffer.slice((csrc_count * 4) + (hasExtensions ? 16 : 12));

    return {
        id: sequence,
        timestamp: timestamp,
        marker: marker,
        payload: payload,
        length: payload.length
    };
}

function unfoldRTCPPacket(buffer) {
    const packetType = buffer[1];
    const timestamp = buffer.readUInt32BE(16);

    return {
        timestamp,
        packetType,
        buffer,
        get payload() {
            return buffer;
        }
    };
}

function getMD5Hash(str) {
    const md5 = createHash("md5");
    md5.update(str);

    return md5.digest("hex");
}

module.exports = {unfoldRTPPacket, getMD5Hash, unfoldRTCPPacket}
