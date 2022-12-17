var controlPanel = null;
var remoteCamera = null;
var imageViewer = null;

class ImageViewer
{
    constructor(id) {
        this.element = document.getElementById(id);
        this.active = true;
        this.reader = new FileReader();
        this.reader.onload = (e) => {
            this.element.src = this.reader.result;
        };
    }

    displayImage(data) {
        if (this.active)
            this.reader.readAsDataURL(data);
    }
    
    showImages() {
        this.active = true;
    }
    
    skipImages() {
        this.active = false;
        this.element.src = "white.png";
    }
}

class RemoteCamera
{
    constructor(id, address, viewer) {
        this.id = id;
        this.viewer = viewer;
        this.continuousUpdate = true;
        this.socket = new WebSocket('ws://' + address);
        this.socket.onmessage = (event) => {
            this.handleMessage(event.data);
        };
        this.socket.onopen = (event) => {
            this.grab();
        };
        this.encoder = new TextEncoder();
    }
    
    disconnect() {
        this.continuousUpdate = false;
        this.socket.onmessage = null;
        this.socket = null;
    }
    
    isTextMessage(buffer) {
        return (typeof buffer === 'string' && buffer.charAt(0) == '{');
    }

    handleMessage(buffer) {
        if (this.isTextMessage(buffer)) {
            this.handleTextMessage(buffer);
            
        } else {
            this.handleBinaryMessage(buffer);
        }
    }  

    handleTextMessage(buffer) {
        var response = JSON.parse(buffer);
        if (response.error)
            this.handleErrorMessage(response.error);
        else
            console.log('Romi Camera: ' + buffer);
    }  

    handleErrorMessage(err) {
        console.log('Romi Camera: Method: ' + response.method
                    + ', Error: ' + response.error.message);
    }  

    handleBinaryMessage(buffer) {
        if (buffer) {
            try {
                this.viewer.displayImage(buffer);
            } catch (error) {
                var str = (typeof buffer === 'string');
                console.error(error);
            }
        }
        if (this.continuousUpdate) {
            this.grab();
        }
    }  

    setRepeat(value) {
        this.continuousUpdate = value;
    }  
    
    grabPerhaps() {
        if (!this.continuousUpdate)
            this.grab();
    }

    grab() {
        var request = {
            'method': 'camera:grab-jpeg-binary',
            'params': {
                'object-id': this.id
            }
        };
        var s = JSON.stringify(request);
        var message = this.encoder.encode(s)
        this.socket.send(message);
    }  
    
    setValue(name, value) {
        var request = {
            'method': 'camera:set-value',
            'params': {
                'object-id': this.id,
                'name': name,
                'value': parseFloat(value)
            }
        };
        var s = JSON.stringify(request);
        this.socket.send(s);
    }  
    
    selectOption(name, value) {
        console.log('Select option: ' + name + '=' + value)
        var request = {
            'method': 'camera:select-option',
            'params': {
                'object-id': this.id,
                'name': name,
                'value': value
            }
        };
        var s = JSON.stringify(request);
        this.socket.send(s);
    }  
}  

class CameraControlPanel
{
    constructor(name, registry) {
        this.name = name;
        this.registry = registry;
        this.camera = null;
        this.sliders = {};
        this.values = {};
        this.menus = {};
        this.continuousUpdate = true;
        this.initControls();
        this.panels = ["mode", "exposure", "compression", "processing"];
        this.selectedPanel = "mode";
        this.initPanelSelector();
        this.imageFrame = document.getElementById("image-viewer");
        this.cameraImage = document.getElementById("camera");
        this.connectButton = document.getElementById("btn-connect");
        this.fixedButton = document.getElementById("btn-fixed-image");
        this.scrollButton = document.getElementById("btn-scroll-image");
        this.repeatButton = document.getElementById("btn-repeat");
        this.initConnectButton();
        this.initImageSizeButtons();
        this.initGrabButtons();
    }

    setCamera(camera) {
        if (camera) {
            this.camera = camera;
            this.connectButton.classList.add("selected");
            this.camera.setRepeat(this.continuousUpdate);
            // FIXME
            imageViewer.showImages();
        } else {
            this.clearCamera();
        }
    }

    clearCamera(camera) {
        this.camera = null;
        this.connectButton.classList.remove("selected");
        // FIXME
        imageViewer.skipImages();
    }
    
    initPanelSelector() {
        this.selectPanel(this.selectedPanel);
        $('#panel-select').change((e) => {
            this.selectPanel(e.target.value);
        });
    }

    selectPanel(name) {
        this.selectedPanel = name;
        this.hidePanels();
        this.showPanel(name);
    }

    showPanel(name) {
        $("#" + name + "-panel").show();
    }

    hidePanel(name) {
        $("#" + name + "-panel").hide();
    }

    hidePanels() {
        var select = document.getElementById('panel-select');
        for (let i = 0; i < select.options.length; i++) {
            this.hidePanel(select.options[i].value);
        }
    }
    
    initConnectButton() {
        $('#btn-connect').click((e) => {
            this.toggleConnection();
        });
    }
    
    toggleConnection() {
        if (this.camera) {
            disconnectCamera();
        } else {
            connectCamera(this.name, this.registry);
        }
    }
    
    initImageSizeButtons() {
        $('#btn-fixed-image').click((e) => {
            this.setFixedSizeImage();
        });
        $('#btn-scroll-image').click((e) => {
            this.setScrollableImage();
        });
    }
    
    setFixedSizeImage() {
        console.log("setFixedSizeImage");
        this.imageFrame.className = "image-viewer fixed-image";
        this.cameraImage.className = "fixed-image";
        this.fixedButton.classList.add("selected");
        this.scrollButton.classList.remove("selected");
    }
    
    setScrollableImage() {
        console.log("setScrollableImage");
        this.imageFrame.className = "image-viewer scroll-image";
        this.cameraImage.className = "scroll-image";
        this.fixedButton.classList.remove("selected");
        this.scrollButton.classList.add("selected");
    }
    
    initGrabButtons() {
        $('#btn-grab').click((e) => {
            if (this.camera) {
                this.camera.grabPerhaps();
            }
        });
        $('#btn-repeat').click((e) => {
            this.toggleRepeat();
        });
    }
    
    toggleRepeat() {
        this.continuousUpdate = !this.continuousUpdate;
        this.camera.setRepeat(this.continuousUpdate);
        if (this.continuousUpdate) {
            this.repeatButton.classList.add("selected");
            if (this.camera) {
                this.camera.grab();
            }
        } else { 
            this.repeatButton.classList.remove("selected");
        }
    }
    
    initControls() {
        this.initOption('mode');
        this.initOption('resolution');
        
        this.initOption('exposure-mode');
        this.initSetting('shutter-speed');
        this.initSetting('analog-gain');
        
        this.initSetting('jpeg-quality');
        this.initSetting('saturation');
        this.initSetting('sharpness');
        this.initSetting('brightness');
        this.initSetting('contrast');
        this.initSetting('iso');
    }
    
    initSetting(name) {
        var slider_id = name + '-slider'
        this.sliders[name] = document.getElementById(slider_id);
        if (this.sliders[name]) {
            $('#' + slider_id).change(() => {
                this.updateValue(name, this.sliders[name].value);
            });
        }
        
        var value_id = name + '-value'
        this.values[name] = document.getElementById(value_id);
        $('#' + value_id).change(() => {
            this.updateValue(name, this.values[name].value);
        });
    }
    
    updateValue(name, value) {
        if (this.sliders[name])
            this.sliders[name].value = value;
        this.values[name].value = value;
        if (this.camera) {
            this.camera.setValue(name, value);
        }
    }

    initOption(name) {
        var select_id = name + '-select'
        this.menus[name] = document.getElementById(select_id);
        $('#' + select_id).change(() => {
            this.updateOption(name, this.menus[name].value);
        });
    }
    
    updateOption(name, value) {
        if (this.camera) {
            this.camera.selectOption(name, value);
        }
    }
}

function connectCamera(name, registry)
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
            console.log(JSON.stringify(reply));
            remoteCamera = new RemoteCamera('camera', reply.address, imageViewer);
            if (controlPanel)
                controlPanel.setCamera(remoteCamera);
        }
    }
}

function disconnectCamera()
{
    remoteCamera.disconnect();
    remoteCamera = null;
    controlPanel.clearCamera();
}

function initCamera(name, registry, createControlPanel)
{
    imageViewer = new ImageViewer('camera');
    if (createControlPanel)
        controlPanel = new CameraControlPanel(name, registry);
    else
        connectCamera(name, registry);
}
