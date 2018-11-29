msg.macAddr = msg.payload.macAddr;
msg.sensorName = msg.payload.sensorName;


let endTime = Date.now();
let startTime = endTime - 5*1000;
startTime = startTime*Math.pow(10,6),
endTime = endTime*Math.pow(10,6)

msg.payload.query = "select * from data where " +
    "\"name\" = '" + msg.sensorName + "' and " +
    "\"macAddr\" = '" + msg.macAddr + "' and " +
    "time >= "+ startTime + " and " +
    "time <= " + endTime;
// return msg;