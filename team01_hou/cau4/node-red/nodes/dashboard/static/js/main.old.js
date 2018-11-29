'use strict';

var DEVICE_INFO_API = '/device/api/info';
var SENSOR_INFO_API = '/sensor/api/info';
var DEVICE_LOGS_API = '/device/api/logs';
var SENSOR_LOGS_API = '/sensor/api/logs';
var SENSOR_COUNT_API = '/sensor/api/count';
var INIT_DATA_API = '/realtime-chart/api/device/initData';
var SENSORS_OF_DEVICE = '/device/sensors';
var LATEST_UNIT_DATA = '/latestUnitData';
var TEMPERATURE_AVERAGE = '/temperatureAvarage';
var LATEST_DATA_API = '/realtime-chart/api/sensor/latestData';
var THINGS_ACTION_URL = '/actionToThings';
var MAC_ADDR_PARAM = 'macAddr';
var SENSOR_ID_PARAM = 'sensorIDs';
var SENSOR_INIT_DATA = '/sensor/chart/initData';
var SENSOR_LATEST_DATA = '/sensor/chart/latestData';
var SENSOR_NAME_PARAM = 'sensorName';

var dataTables;
var realTimeCharts = [];
var devices = [];
var currentDataUnitUpdate;
var currentDataUnitIntervalTime = 2000; // 2s

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

    remove_charts_tab();

    $('#content-header-text').text('Devices information');
    disableDashboardTab();
    $('.espx-datatables-info-container').css('display', '');
    $('#sensor-data-chart-container').css('display', 'none');
    $('#load-data-for-table').css('display', '').text("Loading data ...");
    let getDeviceInfo = ajaxQuery(DEVICE_INFO_API);
    getDeviceInfo.then(
        function (data) {
            // console.log(data);
            devices = [];
            let data_display = [];
            for (let i = 0; i < data.length; i++) {
                let date_created = moment(data[i].created_at).format('HH:mm:ss DD-MM-YYYY');
                // console.log(date_created);
                data_display.push([
                    data[i].macAddr,
                    data[i].device_id,
                    data[i].type,
                    date_created,
                    data[i].status
                ])
                if (data[i].status === "ONLINE") {
                    devices.push({
                        id: data[i].macAddr,
                        text: data[i].type + " - " + data[i].macAddr
                    });
                }
            }
            let columns = [
                {title: "MAC address"},
                {title: "Device ID"},
                {title: "Device type"},
                {title: "Created at"},
                {title: "Status"}
            ];
            if (dataTables) {
                dataTables.destroy();
                $('#espx-datatables-info').empty();
            }
            dataTables = create_datatables_info('espx-datatables-info', data_display, columns);

            $('#load-data-for-table').css('display', 'none');
        },
        function (error) {
            alert("Can't get device info: " + error);
            $('#load-data-for-table').css('display', '').text("Can't get data from server!")
        }
    )
});

$('#sensors-info').on('click', function () {

    remove_charts_tab();

    $('#content-header-text').text('Sensors information');
    disableDashboardTab();
    $('.espx-datatables-info-container').css('display', '');
    $('#sensor-data-chart-container').css('display', 'none');
    $('#load-data-for-table').css('display', '').text("Loading data ... ");
    let getSensorInfo = ajaxQuery(SENSOR_INFO_API);
    getSensorInfo.then(
        function (data) {
            // console.log(data);
            let data_display = [];
            for (let i = 0; i < data.length; i++) {
                let date_created = moment(data[i].created_at).format('HH:mm:ss DD-MM-YYYY');
                data_display.push([
                    data[i].macAddr,
                    data[i].name,
                    data[i].unit,
                    date_created,
                    data[i].status
                ])
            }
            let columns = [
                {title: "Device's mac address"},
                {title: "Sensor name"},
                {title: "Unit"},
                {title: "Created at"},
                {title: "Status"}
            ];
            if (dataTables) {
                dataTables.destroy();
                $('#espx-datatables-info').empty();
            }
            dataTables = create_datatables_info('espx-datatables-info', data_display, columns);
            $('#load-data-for-table').css('display', 'none');
        },
        function (error) {
            alert("Can't get sensor info: " + error);
            $('#load-data-for-table').css('display', '').text("Can't get data from server!")
        }
    )
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

var getInitDeviceData = function (macAddr) {
    return new Promise(function (resolve, reject) {
        $.ajax({
            url: INIT_DATA_API + "?" + MAC_ADDR_PARAM + "=" + macAddr,
        })
            .done(function (data) {
                resolve(data);
            });
    });
}


