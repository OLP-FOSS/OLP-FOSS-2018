module.exports = function (RED) {

    function ThesholdOperatorNode(config) {
        RED.nodes.createNode(this, config);
        this.temperatureLimit = config.temperatureLimit;
        this.humidityLimit = config.humidityLimit;
        this.luxLimit = config.luxLimit;
        this.actionTopic = config.actionTopic;
        var node = this;
        this.on("input", function (msg) {
            if (node.actionTopic.length == 0) {
                node.actionTopic = msg.actionTopic;
            }

            //	C: [1, 100]; 	%: [0, 100],	lux: [1, 65535]
            let sensorData = msg.payload;
            let sensorValue = sensorData.value;
            let sensorInfo = msg.query_result[0];

            let ledID = 0;
            let action = "ON";
            if (sensorData.unit == 'C') {
                ledID = 1;
            } else if (sensorData.unit == '%' || sensorData.unit == 'lux') {
                ledID = 2;
            }
            if (
                (sensorData.unit == 'lux' && 1 <= sensorValue &&
                    sensorValue <= 65535 && sensorValue <= node.luxLimit) ||
                (sensorData.unit == 'C' && 1 <= sensorValue &&
                    sensorValue <= 100 && sensorValue <= node.temperatureLimit) ||
                (sensorData.unit == '%' && 0 <= sensorValue &&
                    sensorValue <= 100 && sensorValue <= node.humidityLimit)
            ) {
                action = "OFF";
            }
            let sendMSg = {
                payload: {
                    'type': 'ledAction',
                    'ledID': ledID,
                    'action': action
                },
                topic: node.actionTopic
            };
            node.send(sendMSg);

            // console.log(node);
        });
        this.on('close', function () {});
    }
    RED.nodes.registerType("threshold-operator", ThesholdOperatorNode);
};