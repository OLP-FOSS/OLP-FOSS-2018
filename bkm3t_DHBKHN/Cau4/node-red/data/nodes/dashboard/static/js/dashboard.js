var myChart;
$('#dashboard').on('click', function () {

    remove_charts_tab();
    $('#content-header-text').text('Dashboard');
    $('.bkcloud-dashboard').css('display', '');
    $('.things-state-container').css('display', '');
    $('.current-value-sensors').css('display', '');
    $('.average-temperature-chart').css('display', '');
    $('#bkcloud-datatables-container').css('display', 'none');
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

    function getLatestUnitData(url, unit, elementValue, elementDatetime, location_id) {
        let getLatestData = ajaxQuery(url + "?unit=" + unit + "&location_id=" + location_id);
        getLatestData.then(
            function (data) {
                if (data.length) {
                    let datetime = moment(data[0].time).format("HH:mm:ss DD-MM-YYYY");
                    $(elementValue).html(data[0].latestValue);
                    $(elementDatetime).html(datetime);
                }
            },
            function (error) {
                console.log(error);
            }
        )
    }

    currentDataUnitUpdate = setInterval(function () {
        getLatestUnitData(LATEST_UNIT_DATA, "C", "#current-temperature-hn", '#current-temperature-datetime-hn', "HN");
        getLatestUnitData(LATEST_UNIT_DATA, "%", "#current-humidity-hn", '#current-humidity-datetime-hn', "HN");
    }, currentDataUnitIntervalTime);

    let ctx = document.getElementById("average-temperature-bar-chart-hn");
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
    let temperatureAverageBarChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: label,
            datasets: [
                {
                    label: 'Temperature',
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
            legend: {
                display: true,
                position: 'top'
            }
        }
    });

    function getAverageTem(location_id) {
        let temperatureAverage = ajaxQuery(TEMPERATURE_AVERAGE + "?location_id=" + location_id);
        temperatureAverage.then(
            function (data) {
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
    }

    getAverageTem("HN");


    $('#dn').on('click', function () {
        if (currentDataUnitUpdate) {
            clearInterval(currentDataUnitUpdate);
        }
        currentDataUnitUpdate = setInterval(function () {
            getLatestUnitData(LATEST_UNIT_DATA, "C", "#current-temperature-dn", '#current-temperature-datetime-dn', "DN");
            getLatestUnitData(LATEST_UNIT_DATA, "%", "#current-humidity-dn", '#current-humidity-datetime-dn', "DN");
        }, currentDataUnitIntervalTime);

        ctx = document.getElementById("average-temperature-bar-chart-dn");

        temperatureAverageBarChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: label,
                datasets: [
                    {
                        label: 'Temperature',
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
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        });

        getAverageTem('DN');
    });

    $('#hcm').on('click', function () {
        if (currentDataUnitUpdate) {
            clearInterval(currentDataUnitUpdate);
        }
        currentDataUnitUpdate = setInterval(function () {
            getLatestUnitData(LATEST_UNIT_DATA, "C", "#current-temperature-hcm", '#current-temperature-datetime-hcm', "HCM");
            getLatestUnitData(LATEST_UNIT_DATA, "%", "#current-humidity-hcm", '#current-humidity-datetime-hcm', "HCM");
        }, currentDataUnitIntervalTime);

        ctx = document.getElementById("average-temperature-bar-chart-hcm");

        temperatureAverageBarChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: label,
                datasets: [
                    {
                        label: 'Temperature',
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
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        });

        getAverageTem('HCM');
    });

    $('#hn').on('click', function () {
        if (currentDataUnitUpdate) {
            clearInterval(currentDataUnitUpdate);
        }
        currentDataUnitUpdate = setInterval(function () {
            getLatestUnitData(LATEST_UNIT_DATA, "C", "#current-temperature-hn", '#current-temperature-datetime-hn', "HN");
            getLatestUnitData(LATEST_UNIT_DATA, "%", "#current-humidity-hn", '#current-humidity-datetime-hcm', "HN");
        }, currentDataUnitIntervalTime);

        ctx = document.getElementById("average-temperature-bar-chart-hn");

        temperatureAverageBarChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: label,
                datasets: [
                    {
                        label: 'Temperature',
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
                legend: {
                    display: true,
                    position: 'top'
                }
            }
        });

        getAverageTem('HN');
    });


});

$('.small-box-device a').on('click', function () {
    $('#devices-info').click();
});

$('.small-box-sensor a').on('click', function () {
    $('#sensors-info').click();
});

