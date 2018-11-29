module.exports = function(RED) {
  "use strict";

  function MEOESPInNode(config) {
    RED.nodes.createNode(this, config);
    let node = this;
    this.topic = `esp/${config.deviceId}/out`;
    this.qos = 2;
    this.broker = config.broker;
    this.brokerConn = RED.nodes.getNode(this.broker);

    if (this.brokerConn) {
      // TODO: check for connection status, this must cooporate with
      // the device (send a ping message per 5 seconds)
      this.status({fill:"red",shape:"ring",text:"disconnected"});
      node.brokerConn.register(this);
      this.brokerConn.subscribe(this.topic, this.qos, (topic, payload, packet) => {
        payload = payload.toString();
        let msg = {
          deviceId: config.deviceId,
          payload: this.parseDeviceDataToJson(payload)
        };
        node.send(msg);
      }, this.id);

      this.on('close', function(done) {
        if (node.brokerConn) {
          node.brokerConn.unsubscribe(node.topic,node.id);
          node.brokerConn.deregister(node,done);
        }
      });
    } else {
      this.error(RED._("mqtt.errors.missing-config"));
    }

    /* Parse data from MEO ESP
    // Raw data format: D5;D6;D7;D8;A0;V1;V2;V3;V4;V5
    // Sample: 0;1;1;0;129;1;100;;;
    // Output: {
      payload: {
        D5: 0,
        D6: 1,
        D7: 1,
        D8: 0,
        A0: 129,
        V1: undefined,
        V2: undefined,
        V3: undefined,
        V4: undefined,
        V5: undefined
      }
    }
    */
    this.parseDeviceDataToJson = function(data) {
      let values, result = {};

      if (data && (values = data.split(";")).length === 10) {
          // Map raw data to JSON
          result["D5"] = parseInt(values[0]);
          result["D6"] = parseInt(values[1]);
          result["D7"] = parseInt(values[2]);
          result["D8"] = parseInt(values[3]);
          result["A0"] = parseInt(values[4]);
          result["V1"] = parseInt(values[5]);
          result["V2"] = parseInt(values[6]);
          result["V3"] = parseInt(values[7]);
          result["V4"] = parseInt(values[8]);
          result["V5"] = parseInt(values[9]);

          return result;
      } else {
        // TODO: handle wrong format
        node.error(`wrong data format received from MQTT channel "esp/${config.deviceId}/out"`, data);
      }
    }
  }

  RED.nodes.registerType("meo-esp-in", MEOESPInNode);

  function MEOESPOutNode(config) {
    RED.nodes.createNode(this, config);
    let node = this;
    this.topic = `esp/${config.deviceId}/out`;
    this.qos = 2;
    this.broker = config.broker;
    this.brokerConn = RED.nodes.getNode(this.broker);

    if (this.brokerConn) {
      // TODO: check for connection status, this must cooporate with
      // the device (send a ping message per 5 seconds)
      this.status({fill:"red",shape:"ring",text:"disconnected"});
      this.on("input", function() {
        var msg = {
          qos: 2,
          retain: false,
          topic: `esp/${config.deviceId}/in`,
          payload: this.buildMsgFromConfig(config)
        };

        this.brokerConn.publish(msg);  // send the message
      });
      if (this.brokerConn.connected) {
        node.status({fill:"green",shape:"dot",text:"node-red:common.status.connected"});
      }
      node.brokerConn.register(node);
      this.on('close', function(done) {
        node.brokerConn.deregister(node,done);
      });
    } else {
      this.error(RED._("mqtt.errors.missing-config"));
    }

    this.buildMsgFromConfig = function(config) {
      console.log(config);
      var pinValues = [];
      var msg = "";

      // Control output pin
      for (var i=0; i<5; i++) {
        var data = "";
        if (config[`D${i}`].length > 0) {
          data += config[`D${i}pwm`] ? 1 : 0;
          data += ".";
          data += config[`D${i}`];
        }
        pinValues.push(data);
      };

      // Control custom functions on ESP
      // TODO: implement this
      for (var i=1; i<=5; i++) {
        pinValues.push(config[`F${i}`] ? 1 : 0);
      }

      return pinValues.join(";");
    }
  }

  RED.nodes.registerType("meo-esp-out", MEOESPOutNode);
}
