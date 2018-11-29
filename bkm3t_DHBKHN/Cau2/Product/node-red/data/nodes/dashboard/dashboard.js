module.exports = function (RED) {
    "use strict";
    // var nodemailer = require('nodemailer');

    var fs = require('fs');
    var staticServer = require("./server.js");

    function Dashboard(config) {
        RED.nodes.createNode(this, config);
        var node = this;
        staticServer.init(RED, '/dashboard/static/');

        node.on("input", function (msg) {
            try {
                // msg = __dirname;
                // node.send(msg);
                fs.readFile(__dirname + '/templates/ui-dashboard.html', "utf8", function (err, data) {
                    msg.payload = data;
                    node.send(msg);
                });
            } catch (err) {
                node.error(err.message);
            }
        });

    }

    RED.nodes.registerType("dashboard", Dashboard);
};