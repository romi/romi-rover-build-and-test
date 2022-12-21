var settingsPanel = null;
var remoteCamera = null;

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
        this.reader.readAsDataURL(data);
    }
}

class RemoteCamera
{
    constructor(id, controller, viewer) {
        this.id = id;
        this.controller = controller;
        this.viewer = viewer;
        this.continuousUpdate = true;
        this.encoder = new TextEncoder();
    }

    getId() {
        return this.id;
    }

    handleTextMessage(buffer) {
        var response = JSON.parse(buffer);
        if (response.error)
            this.handleErrorMessage(response.error);
        else
            console.log('RemoteCamera: ' + buffer);
    }  

    handleErrorMessage(error) {
        console.log('RemoteCamera: Error: ' + error.message);
    }  

    handleBinaryMessage(buffer) {
        if (buffer) {
            try {
                this.viewer.displayImage(buffer);
            } catch (error) {
                //var str = (typeof buffer === 'string');
                console.error(error);
            }
        }
        if (this.continuousUpdate) {
            this.tryGrab();
        }
    }  

    tryGrab() {
        try {
            this.grab();
        } catch (error) {
            console.error(error);
            setTimeout(() => this.tryGrab(), 1000);
        }
    }

    grab() {
        this.controller.invokeBinary(this, 'camera:grab-jpeg-binary');
    }  

    connected() {
        this.tryGrab();
    }  
    
    setValue(name, value) {
        var params = {
            'name': name,
            'value': parseFloat(value)
        };
        this.controller.invoke(this, 'camera:set-value', params);
    }  
    
    selectOption(name, value) {
        var params = {
            'name': name,
            'value': value
        };
        this.controller.invoke(this, 'camera:select-option', params);
    }  
}  

class CameraSettingsPanel
{
    constructor(camera) {
        this.camera = camera;
        this.sliders = {};
        this.values = {};
        this.menus = {};
        this.initControls();
        this.panels = ["mode", "exposure", "compression", "processing"];
        this.selectedPanel = "mode";
        this.initPanelSelector();
        this.imageFrame = document.getElementById("image-viewer");
        this.cameraImage = document.getElementById("camera");
        this.fixedButton = document.getElementById("btn-fixed-image");
        this.scrollButton = document.getElementById("btn-scroll-image");
        this.initImageSizeButtons();
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

function initCamera(name, remoteController, createControlPanel)
{
    let imageViewer = new ImageViewer('camera');
    remoteCamera = new RemoteCamera(name, remoteController, imageViewer);
    remoteController.callWhenConnected(remoteCamera);
    if (createControlPanel)
        settingsPanel = new CameraSettingsPanel(remoteCamera);
}
