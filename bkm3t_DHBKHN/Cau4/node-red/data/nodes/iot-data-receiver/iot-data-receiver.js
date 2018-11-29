module.exports = function (RED) {
    "use strict";

    function IotDataIn(config) {
        RED.nodes.createNode(this, config);
        this.topic = config.topic;
        this.qos = 2;
        this.mqttBroker = config.mqttBroker;
        this.brokerConn = RED.nodes.getNode(this.mqttBroker);

        var node = this;

        if (this.brokerConn) {
            this.status({
                fill: "red",
                shape: "ring",
                text: "disconnected"
            });
            node.brokerConn.register(this);
            this.brokerConn.subscribe(this.topic, this.qos, function (topic, payload, packet) {
                var sensorData = JSON.parse(payload.toString());

                var msgSensorData = {
                    payload: sensorData
                };

                node.send(msgSensorData);
            }, this.id);
            if (this.brokerConn.connected) {
                node.status({
                    fill: "green",
                    shape: "dot",
                    text: "node-red:common.status.connected"
                });
            }
            this.on('close', function (done) {
                if (node.brokerConn) {
                    node.brokerConn.unsubscribe(node.topic, node.id);
                    node.brokerConn.deregister(node, done);
                    node.log("Input node is closed!");
                }
            });
        } else {
            this.error(RED._("mqtt.errors.missing-config"));
        }
    }
    RED.nodes.registerType("iot-data-in", IotDataIn);
};