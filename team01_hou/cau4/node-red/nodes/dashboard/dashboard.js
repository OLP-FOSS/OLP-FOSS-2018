module.exports = function (RED) {
    "use strict";

    var fs = require('fs');

    function Dashboard (config) {
        RED.nodes.createNode(this, config);
        var node = this;

		var express = require("express");
		var app = express();

	    app.use("/", express.static(__dirname + "/static"));
	    RED.httpNode.use("/static/", app);

        node.on("input", function (msg) {
            try {
                fs.readFile(__dirname + '/index.html', "utf8", function (err, data) {
                    msg.payload = data;
                    node.send(msg);
                });
            } catch (err) {
                node.error("dashboard: " + err.message);
            }
        });

    }

    RED.nodes.registerType("dashboard", Dashboard);
};