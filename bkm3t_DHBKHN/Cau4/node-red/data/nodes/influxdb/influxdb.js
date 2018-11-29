module.exports = function (RED) {
    "use strict";
    var Influx = require('influx');

    function InfluxdbServerNode(config) {
        RED.nodes.createNode(this, config);
        this.host = config.host;
        this.port = config.port;
        this.serverUrl = this.host.toString() + ":" + this.port.toString();
        this.database = config.database;
        this.influxClient = null;
        this.isConnected = false;
        var node = this;
        this.users = {};
        this.register = function (influxdbNode) {
            node.users[influxdbNode.id] = influxdbNode;
            if (Object.keys(node.users).length === 1) {
                node.connectToServer();
            }
        };
        // console.log("Create Influxdb Server " + this.serverUrl);
        this.connectToServer = function () {
            // console.log("create interval checking!");
            node.influxClient = new Influx.InfluxDB({
                database: node.database,
                host: node.host,
                port: node.port,
            });
            node.checkServerConnection = setInterval(function () {
                node.influxClient.ping(5000).then(hosts => {
                    // node.debug('Interval for checking influxdb connection is running!');
                    var isConnected = true;
                    var id = null;
                    hosts.forEach(host => {
                        if (host.online) {
                            isConnected = true;
                        } else {
                            isConnected = false;
                        }
                    });
                    if (node.isConnected == false && isConnected == true) {
                        node.influxClient
                            .getDatabaseNames()
                            .then(dbNames => {
                                if (!dbNames.includes(node.database)) {
                                    node.influxClient.createDatabase(node.database).then(
                                        function () {
                                            node.isConnected = true;
                                            // node.log(RED._("Connection to influxdb server " + node.serverUrl + " is opened!"));
                                            for (id in node.users) {
                                                if (node.users.hasOwnProperty(id)) {
                                                    node.users[id].status({
                                                        fill: "green",
                                                        shape: "dot",
                                                        text: "node-red:common.status.connected"
                                                    });
                                                }
                                            }
                                        }
                                    );
                                } else {
                                    node.isConnected = true;
                                    // node.log(RED._("Connection to influxdb server " + node.serverUrl + " is opened!"));
                                    for (id in node.users) {
                                        if (node.users.hasOwnProperty(id)) {
                                            node.users[id].status({
                                                fill: "green",
                                                shape: "dot",
                                                text: "node-red:common.status.connected"
                                            });
                                        }
                                    }
                                }
                            });

                    } else if (node.isConnected == true && isConnected == false) {
                        node.isConnected = false;
                        node.log(RED._("Connection to influxdb server " + node.serverUrl + " is closed!"));
                        for (id in node.users) {
                            if (node.users.hasOwnProperty(id)) {
                                node.users[id].status({
                                    fill: "red",
                                    shape: "ring",
                                    text: "node-red:common.status.disconnected"
                                });
                            }
                        }
                    }
                });
            }, 500);
        };
        this.pushData = function (influxdbClient, measurement, data) {
            if (node.isConnected) {
                let pointList = [];                            
                if (Array.isArray(data)) {
                    for (var i = 0; i < data.length; i++) {
                        let dataPoint = data[i];
                        pointList.push({
                            measurement: measurement,
                            tags: dataPoint.tags,
                            fields: dataPoint.fields,
                            timestamp: dataPoint.time_stamp
                        });

                    }
                } else {
                    pointList.push({
                        measurement: measurement,
                        tags: data.tags,
                        fields: data.fields,
                        timestamp: data.time_stamp,
                    });
                }
                influxdbClient.writePoints(pointList).catch(err => {
                    node.error('Error saving data to InfluxDB!' + err.toString());
                    return false;
                });
                return true;
            } else {
                node.error("Connection to influxdb server " + node.serverUrl + " is closed!");
                return false;
            }
        };
        this.queryData = function (influxdbClient, queryString) {
            return new Promise((resolve, reject) => {
                // console.log("query string: "+queryString);
                if (node.isConnected) {
                    influxdbClient.query(queryString)
                        .then(results => {
                            resolve(results);
                        }).catch(err => {
                            node.error('Error while query data from InfluxDB! ' + err.toString());
                            resolve(false);
                        });
                } else {
                    node.error("Connection to influxdb server " + node.serverUrl + " is closed!");
                    resolve(false);
                }
            });
        };
        this.disconnectToServer = function () {
            if (node.influxClient !== null) {
                node.influxClient = null;
                clearInterval(node.checkServerConnection);
                node.isConnected = false;
                node.log(RED._("Connection to influxdb server " + node.serverUrl + " is closed!"));
            }
        };
        this.deregister = function (influxdbNode) {
            delete node.users[influxdbNode.id];
            if (Object.keys(node.users).length === 0) {
                node.disconnectToServer();
            }
        };

        this.on('close', function () {
            console.log('influxdb Server node is closed!');
            if (node.influxClient !== null) {
                node.disconnectToServer();
                for (var id in node.users) {
                    if (node.users.hasOwnProperty(id)) {
                        node.users[id].status({
                            fill: "red",
                            shape: "ring",
                            text: "node-red:common.status.disconnected"
                        });
                    }
                }
            }

        });
    }
    RED.nodes.registerType("influxdb-server", InfluxdbServerNode);

    function InfluxdbOutputNode(config) {
        RED.nodes.createNode(this, config);
        this.enableSchema = config.enableSchema;
        this.measurement = config.measurement;
        this.dbTags = config.dbTags;
        this.dbFields = config.dbFields;
        this.influxClient = null;
        var node = this;
        // Retrieve the config node
        this.influxdbServer = RED.nodes.getNode(config.influxdbServer);
        if (this.influxdbServer) {
            this.status({
                fill: "red",
                shape: "ring",
                text: "node-red:common.status.disconnected"
            });
            if (this.enableSchema == false) {
                node.influxClient = new Influx.InfluxDB({
                    database: node.influxdbServer.database,
                    host: node.influxdbServer.host,
                    port: node.influxdbServer.port,
                });
            } else {
                var i = 0;
                var fields = {};
                for (i = 0; i < node.dbFields.length; i++) {
                    fields[node.dbFields[i].name] = Influx.FieldType[node.dbFields[i].type];
                }
                var tags = node.dbTags;
                node.influxClient = new Influx.InfluxDB({
                    database: node.influxdbServer.database,
                    host: node.influxdbServer.host,
                    port: node.influxdbServer.port,
                    schema: [{
                        fields: fields,
                        measurement: node.measurement,
                        tags: node.dbTags,
                    }]
                });
            }
            this.on("input", function (msg) {
                node.influxdbServer.pushData(node.influxClient, node.measurement, msg.payload);
            });
            if (this.influxdbServer.isConnected) {
                node.status({
                    fill: "green",
                    shape: "dot",
                    text: "node-red:common.status.connected"
                });
            }
            node.influxdbServer.register(node);
            this.on('close', function () {
                console.log('influxdb output node is closed!');
                node.influxdbServer.deregister(node);
            });

        } else {
            this.error("Missig Influxdb Server config node!");
        }
    }
    RED.nodes.registerType("influxdb-out", InfluxdbOutputNode);



    function InfluxdbQueryNode(config) {
        RED.nodes.createNode(this, config);
        this.influxClient = null;
        this.queryString = config.queryString.replace(/(\r\n|\n|\r)/gm, " ");
        this.outputTo = config.outputTo;
        var node = this;
        // Retrieve the config node
        this.influxdbServer = RED.nodes.getNode(config.influxdbServer);
        if (this.influxdbServer) {
            this.status({
                fill: "red",
                shape: "ring",
                text: "node-red:common.status.disconnected"
            });
            node.influxClient = new Influx.InfluxDB({
                database: node.influxdbServer.database,
                host: node.influxdbServer.host,
                port: node.influxdbServer.port,
            });

            this.on("input", function (msg) {
                var query = node.queryString.length > 0 ? node.queryString : msg.payload.query;
                if (query != null && query.length > 0) {
                    var converted_query = "";
                    var error_syntax = false;
                    var convert_err_msg = "";
                    var begin_markup_index = query.indexOf("{{");
                    var end_markup_index = -1;
                    while (begin_markup_index != -1) {
                        end_markup_index = query.indexOf("}}");
                        if (end_markup_index == -1) {
                            error_syntax = true;
                            convert_err_msg += "Embedded code is not closed! ";
                            break;
                        }
                        converted_query += query.substr(0, begin_markup_index);
                        var embedded_code = query.substr(begin_markup_index + 2, end_markup_index - (begin_markup_index + 2));
                        try {
                            converted_query += eval(embedded_code).toString();
                        } catch (error) {
                            error_syntax = true;
                            convert_err_msg += "Error when execute: " + embedded_code + " Check your query again!";
                            break;
                        }
                        query = query.substr(end_markup_index + 2, query.length);
                        begin_markup_index = query.indexOf("{{");
                    }
                    if (error_syntax == true) {
                        node.error(convert_err_msg);
                    } else {
                        converted_query += query;
                        node.influxdbServer
                            .queryData(node.influxClient, converted_query)
                            .then(function (queryResult) {
                                if (queryResult !== false) {
                                    // console.log(converted_query);
                                    // console.log(queryResult);
                                    // console.log("done");
                                    var outMsg = msg;
                                    outMsg[node.outputTo] = queryResult;
                                    node.send(outMsg);
                                }
                            });
                    }
                } else {
                    node.error("Query string must not be empty!");
                }

            });
            if (this.influxdbServer.isConnected) {
                node.status({
                    fill: "green",
                    shape: "dot",
                    text: "node-red:common.status.connected"
                });
            }
            node.influxdbServer.register(node);
            this.on('close', function () {
                console.log('influxdb output node is closed!');
                node.influxdbServer.deregister(node);
            });

        } else {
            this.error("Missig Influxdb Server config node!");
        }
    }
    RED.nodes.registerType("influxdb-query", InfluxdbQueryNode);


    function InfluxdbWriteDataNode(config) {
        RED.nodes.createNode(this, config);
        // this.enableSchema = config.enableSchema;
        this.influxdbServer = RED.nodes.getNode(config.influxdbServer);
        this.dataInput = config.dataInput;
        this.measurement = config.measurement;
        this.outputTo = config.outputTo;
        this.influxClient = null;
        // if(this.dataInput!=null){
        //     this.dbTags = config.dbTags;
        //     this.dbFields = config.dbFields;    
        // }
        var node = this;
        // Retrieve the config node
        if (this.influxdbServer) {
            this.status({
                fill: "red",
                shape: "ring",
                text: "node-red:common.status.disconnected"
            });
            // if (this.enableSchema == false) {
            node.influxClient = new Influx.InfluxDB({
                database: node.influxdbServer.database,
                host: node.influxdbServer.host,
                port: node.influxdbServer.port,
            });
            // else {
            //     var i = 0;
            //     var fields = {};
            //     for (i = 0; i < node.dbFields.length; i++) {
            //         fields[node.dbFields[i].name] = Influx.FieldType[node.dbFields[i].type];
            //     }
            //     var tags = node.dbTags;
            //     node.influxClient = new Influx.InfluxDB({
            //         database: node.influxdbServer.database,
            //         host: node.influxdbServer.host,
            //         port: node.influxdbServer.port,
            //         schema: [{
            //             fields: fields,
            //             measurement: node.measurement,
            //             tags: node.dbTags,
            //         }]
            //     });
            // }
            this.on("input", function (msg) {
                var writeDataInput = msg[node.dataInput];
                let measurement = node.measurement;
                if (node.measurement.length < 1) {
                    measurement = msg.influx_measurement;
                }
                // console.log(writeDataInput);
                // console.log(measurement);
                // console.log(writeResult);

                var writeResult = node.influxdbServer.pushData(node.influxClient, measurement, writeDataInput);
                msg[node.outputTo] = writeResult;
                setTimeout(function () {
                    node.send(msg);
                }, 500);
            });
            if (this.influxdbServer.isConnected) {
                node.status({
                    fill: "green",
                    shape: "dot",
                    text: "node-red:common.status.connected"
                });
            }
            node.influxdbServer.register(node);
            this.on('close', function () {
                console.log('influxdb output node is closed!');
                node.influxdbServer.deregister(node);
            });

        } else {
            this.error("Missig Influxdb Server config node!");
        }
    }
    RED.nodes.registerType("influxdb-write-data", InfluxdbWriteDataNode);
};