var express = require("express");

var app = express();

function init(RED, path) {
    app.use("/", express.static(__dirname + "/static"));

    RED.httpNode.use(path, app);
    RED.log.info("Dashboard up and running");
}

module.exports = {
    init: init
};
