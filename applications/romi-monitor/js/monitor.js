var startTime = Date.now() / 1000.0;

function seconds()
{
    return Date.now() / 1000.0 - startTime;
}  

function toSeconds(t)
{
    return t - startTime;
}  

function DataQueue()
{
    this.timestamps = [];
    this.values = [];

    this.append = function(t, v) {
        this.timestamps.push(t);
        this.values.push(v);
    }

    this.clear = function() {
        this.timestamps = [];
        this.values = [];
    }

    this.length = function() {
        return this.timestamps.length;
    }
}


var leftTargetSpeed = new DataQueue();
var leftCurrentSpeed = new DataQueue();
var leftMeasuredSpeed = new DataQueue();
var rightTargetSpeed = new DataQueue();
var rightCurrentSpeed = new DataQueue();
var rightMeasuredSpeed = new DataQueue();

var crossTrackError = new DataQueue();
var orientationError = new DataQueue();
var steeringRadius = new DataQueue();
var steeringLeftTargetAngle = new DataQueue();
var steeringRightTargetAngle = new DataQueue();

var batteryVoltage = new DataQueue();
var batteryCurrent = new DataQueue();
var batteryPower = new DataQueue();

var registrySocket = null;
var dataSocket = null;

/*
1631100655.067000,driver-left-target-speed,0.000000
1631100655.067000,driver-left-current-speed,0.000000
1631100655.067000,driver-left-measured-speed,0.000000
1631100655.067000,driver-right-target-speed,0.000000
1631100655.067000,driver-right-current-speed,0.000000
1631100655.067000,driver-right-measured-speed,0.000000

 */

function getData()
{
    orientation.append(seconds(), Math.random());
    return orientation;
}  

function importDataPoint(d)
{
    // Speed
    if (d[1] == "driver-left-target-speed") {
        leftTargetSpeed.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "driver-left-current-speed") {
        leftCurrentSpeed.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "driver-left-measured-speed") {
        leftMeasuredSpeed.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "driver-left-target-speed") {
        rightTargetSpeed.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "driver-right-current-speed") {
        rightCurrentSpeed.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "driver-right-measured-speed") {
        rightMeasuredSpeed.append(toSeconds(d[0]), d[2]);
    }
    // Navigation
    else if (d[1] == "l1-error-distance") {
        crossTrackError.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "l1-error-angle") {
        orientationError.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "l1-r") {
        steeringRadius.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "steering-target-angle-left") {
        steeringLeftTargetAngle.append(toSeconds(d[0]), d[2]);
    } else if (d[1] == "steering-target-angle-right") {
        steeringRightTargetAngle.append(toSeconds(d[0]), d[2]);
    }
    // Battery
    else if (d[1] == "battery-voltage") {
        batteryVoltage.append(toSeconds(d[0]), d[2]/1000.0);
    } else if (d[1] == "battery-current") {
        batteryCurrent.append(toSeconds(d[0]), -d[2]/1000.0);
    } else if (d[1] == "battery-instant-power") {
        batteryPower.append(toSeconds(d[0]), -d[2]);
    }
}

function importDataArray(data)
{
    for (let i = 0; i < data.length; i++)
        importDataPoint(data[i]);
}

function updatePlot(name, datasets)
{
    x = [];
    y = [];
    indices = [];
    
    for (let i = 0; i < datasets.length; i++) {
        dataset = datasets[i];
        if (dataset.length() > 0) {
            x.push(dataset.timestamps);
            y.push(dataset.values);
            indices.push(i);
        }
    }
    
    Plotly.extendTraces(name, { x: x, y: y }, indices);
    
    for (let i = 0; i < datasets.length; i++) {
        datasets[i].clear();
    }

    now = seconds();
    if (now > 30) {
        Plotly.relayout(name, {
            xaxis: {
                range: [now - 30, now]
            }
        });
    }
}

function updateSpeedPlot()
{
    updatePlot("speed", [leftTargetSpeed, leftCurrentSpeed, leftMeasuredSpeed,
                         rightTargetSpeed, rightCurrentSpeed, rightMeasuredSpeed]);
}

function updateNavigationPlot()
{
    updatePlot("navigation",  [crossTrackError, orientationError, steeringRadius,
                               steeringLeftTargetAngle, steeringRightTargetAngle]);
}

function updatePowerPlot()
{
    updatePlot("power",  [batteryVoltage, batteryCurrent, batteryPower]);
}

function updatePlots()
{
    updateSpeedPlot();
    updateNavigationPlot();
    updatePowerPlot();
}

function initSpeedPlot()
{
    Plotly.newPlot('speed',[{
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Left Target Speed',
        line: {color: '#80CAF6'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Left Current Speed',
        line: {color: '#DF56F1'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Left Measured Speed',
        line: {color: '#0F5031'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Right Target Speed',
        line: {color: '#80CAF6'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Right Current Speed',
        line: {color: '#DF56F1'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Right Measured Speed',
        line: {color: '#0F5031'}
    }]);
}

function initNavigationPlot()
{
    Plotly.newPlot('navigation',[{
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Cross Track Error',
        line: {color: '#80CAF6'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Orientation Error',
        line: {color: '#DF56F1'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Steering Radius',
        line: {color: '#0F5031'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Angle Left Wheel',
        line: {color: '#80CAF6'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Angle Right Wheel',
        line: {color: '#DF56F1'}
    }]);
}

function initPowerPlot()
{
    Plotly.newPlot('power',[{
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Voltage',
        line: {color: '#80CAF6'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Current',
        line: {color: '#DF56F1'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Power',
        line: {color: '#0F5031'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Battery Charge',
        line: {color: '#80CAF6'}
    }, {
        x: [0],
        y: [0],
        mode: 'lines',
        name: 'Time To Go',
        line: {color: '#DF56F1'}
    }]);
}

function initMonitor(registry)
{
    initSpeedPlot();
    initNavigationPlot();
    initPowerPlot();

    setInterval(function() { updatePlots(); }, 100);
    
    registrySocket = new WebSocket("ws://" + registry + ":10101");

    registrySocket.onopen = function (event) {
        var request = { "request": "get", "topic": "datalog" };
        registrySocket.send(JSON.stringify(request));
    };

    registrySocket.onmessage = function (event) {
        console.log(event.data);
        var reply = JSON.parse(event.data);
        if (reply.success) {
            registrySocket.close();

            dataSocket = new WebSocket("ws://" + reply.address);
            dataSocket.onmessage = function (event) {
                importDataArray(JSON.parse(event.data));
            }
        }
    }
}
