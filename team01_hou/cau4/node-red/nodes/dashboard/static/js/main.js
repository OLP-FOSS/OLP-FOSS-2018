'use strict';

var DEVICE_INFO_API = '/device/api/info';
var DEVICE_LOGS_API = '/device/api/logs';

var dataTables;
var realTimeCharts = [];
var devices = [];
var currentDataUnitUpdate;
var currentDataUnitIntervalTime = 2000;

// **** main ****

$('.espx-sidebar-element').each(function (index) {
    $(this).on('click', function () {
        if (!$(this).hasClass('active')) {
            $('ul.sidebar-menu li.espx-sidebar-element.active').removeClass('active');
            $(this).addClass('active');
        }
    })
});

var create_datatables_info = function (selector, data, columns) {
    return $('#' + selector).DataTable({
        data: data,
        columns: columns
    });
};

var create_datatables_logs = function (selector, data, columns) {
    return $('#' + selector).DataTable({
        data: data,
        columns: columns
    });
};

let remove_charts_tab = function () {
    if (realTimeCharts.length) {
        for (let i = 0; i < realTimeCharts.length; i++) {
            realTimeCharts[i].destroy();
        }
        $('#sensor-data-chart').empty();
        $('#sensor-legend-chart').empty();
    }
}

let disableDashboardTab = function () {
    $('.espx-dashboard').css('display', 'none');
    $('.current-value-sensors').css('display', 'none');
    $('.avarage-temperature-chart').css('display', 'none');
    $('.things-state-container').css('display', 'none');
    if (currentDataUnitUpdate) {
        clearInterval(currentDataUnitUpdate);
    }
}

$('#devices-info').on('click', function () {

});

$('#sensors-info').on('click', function () {

});

var ajaxQuery = function (url) {
    return new Promise(function (resolve, eject) {
        $.ajax(
            {url: url}
        ).done(function (data) {
            resolve(data);
        }).fail(function (jqXHR, textStatus) {
            eject(textStatus);
        });
    });
}

var getChartInitData = function (macAddr) {
    return new Promise(function (resolve, reject) {
        $.ajax({
            url: URL_API_CHART_INIT + "?device=" + macAddr,
        })
            .done(function (data) {
                resolve(data);
            });
    });
}

// **** dashboard ****

var myChart;
$('#dashboard').on('click', function () {

    remove_charts_tab();
    $('#content-header-text').text('Dashboard');
    $('.espx-dashboard').css('display', '');
    $('.things-state-container').css('display', '');
    $('.current-value-sensors').css('display', '');
    $('.avarage-temperature-chart').css('display', '');
    $('#espx-datatables-container').css('display', 'none');
    $('#sensor-data-chart-container').css('display', 'none');


    // function random_rgba() {
    //     var o = Math.round, r = Math.random, s = 255;
    //     return 'rgba(' + o(r() * s) + ',' + o(r() * s) + ',' + o(r() * s) + ',' + r().toFixed(1) + ')';
    // }

    function random_rgba(i) {
        return "hsl(" + i * 100 + ", 100%, 50%)";
    }

    let getSensorInfo = ajaxQuery(SENSOR_COUNT_API);
    getSensorInfo.then(
        function (data) {
            $('.small-box-sensor h3').html(data);
        },
        function (error) {
            console.log(error);
        }
    );

    var deviceStatusDoughnutChart = new Chart($('#device-status-pie-chart'), {
        type: 'doughnut',
        data: {
            datasets: [
                {
                    data: [0, 0],
                    backgroundColor: [
                        '#3F9F3F', 'rgb(85, 89, 96)'
                    ],
                }
            ],
            labels: ['ONLINE', 'OFFLINE']
        },
        options: {
            pieceLabel: {
                // render: 'label'
                render: 'percentage',
                fontColor: '#fff'
            }
        }
    });

    var deviceTypesPieChart = new Chart($('#device-types-pie-chart'), {
        type: 'pie',
        data: {
            datasets: [
                {
                    data: [],
                    // data: [50, 10, 20, 20],
                    backgroundColor: [
                        // 'rgb(226, 206, 27)', 'rgb(91, 201, 36)', '#1c4bcc', '#d60e36'
                    ],
                }
            ],
            labels: []
            // labels: ['ESP8266', 'UNO', 'PI', 'ABC']
        },
        options: {
            pieceLabel: {
                render: 'percentage',
                fontColor: '#fff'
            }
        }
    });

    let deviceCount = 0;
    let deviceStatusData = [0, 0];  //ONLINE - OFFLINE

    let DeviceType = function (label, color) {
        this.label = label;
        this.count = 1;
        this.color = color;
    };
    let dataDeviceTypes = []; // [DeviceType1, DeviceType2]

    let getDeviceInfo = ajaxQuery(DEVICE_INFO_API);
    getDeviceInfo.then(
        function (data) {
            // devices = [{id: "", text: "-- Select device --"}];
            devices = [];
            for (let i = 0; i < data.length; i++) {
                let device = data[i];
                deviceCount += 1;
                if (device.status === "ONLINE") {
                    deviceStatusData[0] += 1;
                    devices.push({
                        id: data[i].macAddr,
                        text: data[i].type + " - " + data[i].macAddr
                    });
                } else {
                    deviceStatusData[1] += 1;
                }
                let isNewDeviceType = true;
                for (let j = 0; j < dataDeviceTypes.length; j++) {
                    if (device.type === dataDeviceTypes[j].label) {
                        dataDeviceTypes[j].count += 1;
                        isNewDeviceType = false;
                        break
                    }
                }
                if (isNewDeviceType) {
                    dataDeviceTypes.push(new DeviceType(device.type, random_rgba(i)));
                }
            }
            $('.small-box-device h3').html(deviceCount);
            $('#devices-number').html(deviceCount);

            //  update doughnut chart device status
            deviceStatusDoughnutChart.data.datasets[0].data = deviceStatusData;
            deviceStatusDoughnutChart.update();

            //    update pie chart device types
            for (let i = 0; i < dataDeviceTypes.length; i++) {
                let deviceType = dataDeviceTypes[i];
                deviceTypesPieChart.data.datasets[0].data.push(deviceType.count);
                deviceTypesPieChart.data.datasets[0].backgroundColor.push(deviceType.color);
                deviceTypesPieChart.data.labels.push(deviceType.label);
            }
            deviceTypesPieChart.update();

        },
        function (error) {
            console.log(error);
        }
    );

    function getLatestUnitData(url, unit, elementValue, elementDatetime) {
        let getLatestData = ajaxQuery(url + "?unit=" + unit);
        getLatestData.then(
            function (data) {
                let datetime = moment(data[0].time).format("HH:mm:ss DD-MM-YYYY");
                $(elementValue).html(data[0].latestValue);
                $(elementDatetime).html(datetime);
            },
            function (error) {
                console.log(error);
            }
        )
    }

    currentDataUnitUpdate = setInterval(function () {
        getLatestUnitData(LATEST_UNIT_DATA, "C", "#current-temperature", '#current-temperature-datetime');
        getLatestUnitData(LATEST_UNIT_DATA, "%", "#current-humidity", '#current-humidity-datetime');
    }, currentDataUnitIntervalTime);

    var ctx = document.getElementById("average-temperature-bar-chart");
    let days = ["Mon", "Tue", "Wed", "Thu", "Fri", "Sa", "Sun"];
    let today = new Date();
    let d = today.toString().split(" ")[0];
    const index = days.indexOf(d);
    let label = [];
    if (index !== -1) {
        // days.splice(index, 1);
        let l1 = days.slice(0, index);
        let l2 = days.slice(index + 1, days.length);
        label = l2.concat(l1);
    }
    console.log(label);
    let temperatureAverageBarChart = new Chart(ctx, {
        type: 'bar',
        data: {
            // labels: ["Mon", "Tue", "Wed", "Thus", "Fri", "Sa"],
            labels: label,
            datasets: [
                {
                    label: 'Temperature',
                    // data: [32.1, 32.2, 32.9, 32.93, 32.4, 31],
                    data: [],
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    borderColor: 'rgba(255,99,132,1)',
                    borderWidth: 1,
                    fill: true
                }
            ]
        },
        options: {
            maintainAspectRatio: false,
            // scales: {
            //     yAxes: [{
            //         ticks: {
            //             beginAtZero: true
            //         }
            //     }]
            // },
            // tooltips: {
            //     mode: 'nearest'
            // },
            legend: {
                display: true,
                position: 'top'
            }
        }
    });

    let temperatureAverage = ajaxQuery(TEMPERATURE_AVERAGE);
    temperatureAverage.then(
        function (data) {
            // console.log(data);
            let dayValue = [];
            for (let i = 0; i < data.length; i++) {
                if (data[i]) {
                    let value = data[i].value;
                    value = Math.round(value * 100) / 100;
                    dayValue.push(value);
                }
            }
            temperatureAverageBarChart.data.datasets[0].data = dayValue;
            temperatureAverageBarChart.update();
        },
        function (error) {
            console.log(error);
        }
    )


});

$('.small-box-device a').on('click', function () {
    $('#devices-info').click();
});

$('.small-box-sensor a').on('click', function () {
    $('#sensors-info').click();
});

// $('#room1-led1-btn-on').on('click', function () {
//
//     $.post(THINGS_ACTION_URL,
//         {
//             topic: "espx/MAC/action",
//             message:{
//                 type: "ledAction",
//                 action: "ON"
//             }
//         },
//         function (data, status) {
//             alert("Data: " + data + "\nStatus: " + status);
//         }
//     );
//     $('#room1-led1-state').html('ON');
//     $('#room1-led1-btn-on').addClass('disabled');
//     $('#room1-led1-btn-off').removeClass('disabled');
// });
// $('#room1-led1-btn-off').on('click', function () {
//     $('#room1-led1-state').html('OFF');
//     $('#room1-led1-btn-off').addClass('disabled');
//     $('#room1-led1-btn-on').removeClass('disabled');
// });


// **** line-chart ****

var sensors_chart = [];
$('#line-chart').on('click', function () {
/*
    $('#content-header-text').text('Lines chart');
    $('#sensor-data-chart-container').css('display', '');
    $('.espx-datatables-info-container').css('display', 'none');
    disableDashboardTab();

    let ctx = document.getElementById("realtime-line-chart").getContext("2d");

    let line_chart_data = {
        labels: [],
        datasets: [{
            label: "",
            fillColor: "rgba(220,220,220,0.2)",
            strokeColor: "rgba(220,220,220,1)",
            pointColor: "rgba(220,220,220,1)",
            pointStrokeColor: "#fff",
            pointHighlightFill: "#fff",
            pointHighlightStroke: "rgba(220,220,220,1)",
            data: []
        }]
    };
    let line_chart_options = {
        maintainAspectRatio: false,
        animation: false,
        //Boolean - If we want to override with a hard coded scale
        scaleOverride: true,
        //** Required if scaleOverride is true **
        //Number - The number of steps in a hard coded scale
        scaleSteps: 10,
        //Number - The value jump in the hard coded scale
        scaleStepWidth: 10,
        //Number - The scale starting value
        scaleStartValue: 0
    };


    if (realTimeCharts.length) {
        for (let i = 0; i < realTimeCharts.length; i++) {
            realTimeCharts[i].destroy();
        }
        $('#sensor-data-chart').empty();
        $('#sensor-legend-chart').empty();
    }
    if (devices.length) {
        $('#combobox-devices').css('display', "");
        $('#load-data-for-chart').css('display', "none");
        $('#chart-controller-device-input').select2({
            width: '100%',
            data: devices
        }).on("select2:select", function (e) {
            $(this).prop('disabled', true);
            let self = this;
            let macAddr = e.target.value;
            if (realTimeCharts.length) {
                for (let i = 0; i < realTimeCharts.length; i++) {
                    realTimeCharts[i].destroy();
                }
                $('#sensor-data-chart').empty();
                $('#sensor-legend-chart').empty();
            }
            ajaxQuery(SENSORS_OF_DEVICE + "?" + MAC_ADDR_PARAM + "=" + macAddr).then(
                function (data) {
                    if (data.length) {
                        sensors_chart = [];
                        $('#no-data-realtime').css("display", 'none');
                        for (let i = 0; i < data.length; i++) {
                            let sensor = data[i];
                            if (sensor.status === 'ONLINE' && sensor.unit) {
                                sensors_chart.push(
                                    {
                                        id: sensor.macAddr + "/" + sensor.name,
                                        text: sensor.name
                                    }
                                )
                            }
                        }
                        if (sensors_chart.length) {

                            $('#chart-controller-sensor-input').select2({
                                width: '100%',
                                data: sensors_chart
                            }).on("select2:select", function (e) {
                                $(this).prop('disabled', true);
                                if (realTimeCharts.length) {
                                    for (let i = 0; i < realTimeCharts.length; i++) {
                                        realTimeCharts[i].destroy();
                                    }
                                    $('#sensor-data-chart').empty();
                                    $('#sensor-legend-chart').empty();
                                }
                                line_chart_data.datasets[0].data = [];
                                line_chart_data.labels = [];
                                let self = this;
                                let key = e.target.value;
                                let macAddr = key.split("/")[0];
                                let sensorName = key.split("/")[1];
                                ajaxQuery(SENSOR_INIT_DATA + "?" + MAC_ADDR_PARAM + "=" + macAddr +
                                    "&" + SENSOR_NAME_PARAM + "=" + sensorName).then(
                                    function (data) {
                                        if (data.length) {
                                            if (data[0].unit === "C") {
                                                line_chart_data.datasets[0].label = "Temperature - C";
                                            } else {
                                                line_chart_data.datasets[0].label = "Humidity - %";
                                            }
                                            for (let i = 0; i < data.length; i++) {
                                                let sensorData = data[i];
                                                line_chart_data.datasets[0].data.push(sensorData.value);
                                                let t = moment(new Date(sensorData.time)).format('HH:mm:ss');
                                                line_chart_data.labels.push(t);
                                            }
                                            let newLineChart = new Chart(ctx, {
                                                type: 'line',
                                                data: line_chart_data,
                                                options: line_chart_options
                                            });
                                            newLineChart.intervalTime = setInterval(function () {
                                                ajaxQuery(SENSOR_LATEST_DATA + "?" + MAC_ADDR_PARAM + "=" + macAddr +
                                                    "&" + SENSOR_NAME_PARAM + "=" + sensorName).then(
                                                    function (data) {
                                                        if (data.length) {
                                                            setData(newLineChart, data[0].value);
                                                            // setData(data.datasets[1].data);
                                                            setLabels(newLineChart, data[0].time);
                                                            newLineChart.update();
                                                        }
                                                    }
                                                );
                                            }, 2000);
                                            newLineChart.destroy = function () {
                                                clearInterval(newLineChart.intervalTime);
                                            }
                                            realTimeCharts.push(newLineChart);

                                        }
                                    },
                                    function (error) {
                                        alert('Can not get sensor data!');
                                        console.log(error);
                                    }
                                );
                                console.log(key);
                                setTimeout(function () {
                                    $(self).prop('disabled', false);
                                }, 1000);
                            });
                            $('#chart-controller-sensor-input').val(sensors_chart[0].id).trigger("select2:select");
                        } else {
                            $('#no-data-realtime').css("display", '');
                        }

                    } else {
                        $('#no-data-realtime').css("display", '');
                    }
                },
                function (error) {
                    alert('Can not get sensors of this device!');
                    console.log(error);
                }
            )
            setTimeout(function () {
                $(self).prop('disabled', false);
            }, 1000);
        })

        $('#chart-controller-device-input').val(devices[0].id).trigger("select2:select");


    } else {
        $('#combobox-devices').css('display', "none");
        $('#load-data-for-chart').css('display', "").text("Have no data!");
    }


    function setLabels(myLineChart, label) {
        let l = moment(new Date(label)).format('HH:mm:ss');
        myLineChart.data.labels.push(l);
        myLineChart.data.labels.shift();
    }

    function setData(myLineChart, d) {
        myLineChart.data.datasets[0].data.push(d);
        myLineChart.data.datasets[0].data.shift();
    }

*/
});

