var remoteCameraMount = null;
var mountControlPanel = null;

class RemoteCameraMount
{
    constructor(address) {
        this.socket = new WebSocket('ws://' + address);
        this.socket.onmessage = (event) => {
            this.handleMessage(event.data);
        };
        this.socket.onopen = (event) => {
            this.initRangeAndPosition();
        };
        this.range_xyz = [[0, 0], [0, 0], [0, 0]];
        this.range_angles = [[0, 0], [0, 0], [0, 0]];
        this.position = [0, 0, 0, 0, 0, 0];
    }

    initRangeAndPosition() {
        this.askRange();
    }

    askRange() {
        var request = { 'method': 'camera-mount:get-range' };
        var s = JSON.stringify(request);
        this.socket.send(s);
    }

    askPosition() {
        var request = { 'method': 'camera-mount:get-position' };
        var s = JSON.stringify(request);
        this.socket.send(s);
    }

    moveto(x, ax) {
        var request = {
            'method': 'camera-mount:moveto',
            'params': { 'x': x, 'y': 0, 'z': 0,
                        'ax': ax, 'ay': 0, 'az': 0,
                        'speed': 1 }};
        var s = JSON.stringify(request);
        this.socket.send(s);
    }
    
    handleMessage(buffer) {
        var response = JSON.parse(buffer);
        //console.log('Romi Camera: Response: ' + buffer);
        if (response.error) {
            this.handleErrorMessage(response);
        } else if (response.method == "camera-mount:get-range") {
            this.setRange(response.result);
            this.askPosition();
        } else if (response.method == "camera-mount:get-position") {
            this.setPosition(response.result);
            this.moveto(2.0, 0.0);
        } else {
            console.log('Romi Camera: Response: ' + buffer);
        }
    }  

    handleErrorMessage(response) {
        console.log('Romi CameraMount: Method: ' + response.method
                    + ', Error: ' + response.error.message);
    }  

    setRange(result) {
        this.range_xyz = result["xyz-range"];
        this.range_angles = result["angles-range"];
    }  

    setPosition(result) {
        this.position[0] = result['x'];
        this.position[1] = result['y'];
        this.position[2] = result['z'];
        this.position[3] = result['ax'];
        this.position[4] = result['ay'];
        this.position[5] = result['az'];
        console.log('Position: ' + this.position);
    }  
}  

class CameraMountControlPanel
{
    constructor(cameraMount) {
        this.cameraMount = cameraMount;
        this.values = {};
        this.initControls();
        this.x = 0;
        this.ax = 0;
    }
    
    initControls() {
        this.initSetting('x');
        this.initSetting('ax');
    }
    
    initSetting(name) {
        var value_id = name + '-value'
        this.values[name] = document.getElementById(value_id);
        $('#' + value_id).change(() => {
            this.updateValue(name, this.values[name].value);
        });
    }
    
    updateValue(name, value) {
        if (name == 'x') {
            this.x = parseFloat(value);
            this.cameraMount.moveto(this.x, this.ax); 
        } else if (name == 'ax') {
            this.ax = parseFloat(value);
            this.cameraMount.moveto(this.x, this.ax); 
        }
    }
}

function initCameraMount(name, registry)
{
    var registrySocket = new WebSocket('ws://' + registry + ':10101');

    registrySocket.onopen = function (event) {
        var request = { 'request': 'get', 'topic': name };
        registrySocket.send(JSON.stringify(request));
    };

    registrySocket.onmessage = function (event) {
        console.log(event.data);
        var reply = JSON.parse(event.data);
        if (reply.success) {
            registrySocket.close();
            remoteCameraMount = new RemoteCameraMount(reply.address);
            mountControlPanel = new CameraMountControlPanel(remoteCameraMount); 
        }
    }
}
