var cameraMountController = null;
var mountControlPanel = null;

class Position
{
    constructor() {
        this.x = 0;
        this.y = 0;
        this.z = 0;
        this.ax = 0;
        this.ay = 0;
        this.az = 0;
    }
}

class Range
{
    constructor() {
        this.xmin = 0;
        this.xmax = 0;
        this.ymin = 0;
        this.ymax = 0;
        this.zmin = 0;
        this.zmax = 0;
    }

    convert(list) {
	this.xmin = list[0][0];
	this.xmax = list[0][1];
	this.ymin = list[1][0];
	this.ymax = list[1][1];
	this.zmin = list[2][0];
	this.zmax = list[2][1];
    }  
}

class CameraMountController
{
    constructor(id, position, xyzRange, anglesRange, remoteController, elementID) {
        this.id = id;
        this.position = position;
        this.xyzRange = xyzRange;
        this.anglesRange = anglesRange;
        this.remoteController = remoteController;
        this.elementID = elementID;
    }

    getId() {
        return this.id;
    }

    connected() {
        console.log('CameraMountController.connected');
        this.initRangeAndPosition();
    }  

    initRangeAndPosition() {
        this.askRange();
    }

    askRange() {
        this.remoteController.invoke(this, 'camera-mount:get-range');
    }

    askPosition() {
        this.remoteController.invoke(this, 'camera-mount:get-position');
    }

    moveto(x, ax) {
	console.log("moveto " + x + " " + ax);
        this.position.x = x;
        this.position.ax = ax;
        var params = { 'x': this.position.x,
                       'y': this.position.y,
                       'z': this.position.z,
                       'ax': this.position.ax,
                       'ay': this.position.ay,
                       'az': this.position.az,
                       'speed': 1 };
        this.remoteController.invoke(this, 'camera-mount:moveto', params);
    }

    handleErrorMessage(error) {
        console.log('CameraMountController: Error: ' + error.message);
    }  
    
    handleTextMessage(response) {
        if (response.method == "camera-mount:get-range") {
            this.setRange(response.result);
            this.askPosition();
        } else if (response.method == "camera-mount:get-position") {
            this.setPosition(response.result);
            this.buildView();
        } else {
            console.log('CameraMountController: Unknown method: ' + response.method);
        }
    }  

    handleBinaryMessage(buffer) {
        console.log('CameraMountController: Not expected');
    }

    setRange(result) {
        console.log("TODO: CameraMountController.setRange: result=" + JSON.stringify(result));
	this.setXYZRange(result['xyz-range']);
	this.setAnglesRange(result['angles-range']);
    }  

    setXYZRange(list) {
	this.xyzRange.convert(list);
    }  

    setAnglesRange(list) {
	this.anglesRange.convert(list);
    }  

    setPosition(result) {
        this.position.x = result['x'];
        this.position.y = result['y'];
        this.position.z = result['z'];
        this.position.ax = result['ax'];
        this.position.ay = result['ay'];
        this.position.az = result['az'];
        console.log('Position: ' + result);
    }  

    buildView() {
        let parent = document.getElementById(this.elementID);
        let view = new CameraMountViewer(this.position, this.xyzRange,
                                         this.anglesRange, this);
        parent.appendChild(view.element);
    }
}  

class CameraMountViewer
{
    constructor(position, xyzRange, anglesRange, controller) {
        this.controller = controller;
        this.x = position.x;
        this.xmin = xyzRange.xmin;
        this.xmax = xyzRange.xmax;
        this.ax = position.ax;
        this.axmin = anglesRange.xmin;
        this.axmax = anglesRange.xmax;
        this.makeView();
    }
    
    makeView() {
        this.element = document.createElement("div");
        this.element.className = "position";
        let xview = this.makeXView();
        let axview = this.makeAXView();
        this.element.appendChild(xview);
        this.element.appendChild(axview);
    }
    
    makeXView() {
        var element = document.createElement('div');
        element.className = 'position-xyz-section';

        var text = document.createElement('span');
        text.className = 'position-x-label';
        text.innerHTML = "Move to position (m)";
        element.appendChild(text);        

        this.xview = this.makeXTextField();
        element.appendChild(this.xview.element);        
        return element;
    }
        
    makeXTextField() {
        return new TextField((target) => { let x = parseFloat(target.value);
                                           this.update(x, this.ax); },
                             'position-x', this.x, 4);
    }
    
    makeAXView(position, range) {
        var element = document.createElement('div');
        element.className = 'position-angles-section';

        var text = document.createElement('span');
        text.className = 'position-ax-label';
        text.innerHTML = "Position camera at angle (°)";
        element.appendChild(text);        

        this.axview = this.makeAXTextField();
        element.appendChild(this.axview.element);        
        return element;
    }
    
    makeAXTextField() {
        return new TextField((target) => { let ax = parseFloat(target.value);
                                           this.update(this.x, ax); },
                             'position-ax', this.ax, 4);
    }
    
    update(x, ax) {
	console.log("update " + x + " " + ax);
        if (x < this.xmin || x > this.xmax) {
	    console.log("update x out of range [" + this.xmin + "," + this.xmax + "]");
            this.xview.value = this.x;
        } else if (ax < this.axmin || ax > this.axmax) {
	    console.log("update ax out of range [" + this.axmin + "," + this.axmax + "]");
            this.axview.value = this.ax;
        } else {
	    console.log("update ok");
            this.x = x;
            this.ax = ax;
            this.controller.moveto(this.x, this.ax);
        }
    }
}

function initCameraMount(name, remoteController)
{
    var position = new Position();
    var xyzRange = new Range();
    var anglesRange = new Range();
        
    cameraMountController = new CameraMountController(name, position,
                                                      xyzRange, anglesRange,
                                                      remoteController, 'position-app');    

    remoteController.callWhenConnected(cameraMountController);
}
