var sensors_chart = [];

$('#line-chart').on('click', function () {
    $('#content-header-text').text('Lines chart');
    $('#sensor-data-chart-container').css('display', '');
    $('.bkcloud-datatables-info-container').css('display', 'none');
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


});