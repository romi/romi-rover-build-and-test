var remoteController = null;
var programList = null;
var programListController = null;

class Selector
{
    constructor(callback, classname, options) {
        this.callback = callback;
        this.element = document.createElement('select');
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
        this.makeOptions(options);
        this.value = null;
    }

    makeOptions(options) {
        for (const option of options) {
            let element = document.createElement('option');
            element.value = option.value;
            element.label = option.label;
            if (option.selected) {
                element.selected = true;
                this.value = option.value;
            }
            this.element.appendChild(element);
        }
    }

    updateValue(e) {
        this.value = e.target.value;
        this.callback(this);
    }
}

class TextField
{
    constructor(callback, classname, value, length) {
        this.callback = callback;
        this.value = value;
        this.element = document.createElement('input');
        this.element.type = 'text';
        this.element.value = value;
        this.element.size = length;
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
    }

    updateValue(e) {
        this.value = e.target.value;
        this.callback(this);
    }
}

class Checkbox
{
    constructor(callback, classname, value) {
        this.callback = callback;
        this.element = document.createElement('input');
        this.element.type = 'checkbox';
        this.element.checked = value;
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
        this.value = value;
    }

    updateValue(e) {
        this.value = e.target.checked;
        this.callback(this);
    }
}

//

class Program
{
    constructor(id, name, hour, minute,
                start, length, interval,
                tilt, enabled) {
        this.id = id;
        this.name = name;
        this.hour = hour;
        this.minute = minute;
        this.start = start;
        this.length = length;
        this.interval = interval;
        this.tilt = tilt;
        this.enabled = enabled;
    }

    
}

/*
Program.prototype.toString = function() {
    return `Program[id=${this.id}, name=${this.name}]: at ${this.hour}:${this.minute} start at ${this.start} m image every ${this.interval} m over ${this.length}, tilt ${this.tilt}, , enabled: ${this.enabled}`;
};
*/

class ProgramList
{
    constructor() {
        console.log('ProgramList');
        this.programs = [];
    }

    clear() {
        this.programs = [];
    }

    add(program) {
        this.programs.push(program);
    }

    count() {
        return this.programs.length;
    }

    get(index) {
        return this.programs[index];
    }

    getId(id) {
        for (let program of this.programs) {
            if (program.id == id) {
                return program;
            }
        }
    }
}

//

class ProgramView
{
    constructor(program, controller) {
        this.program = program;
        this.controller = controller;
        this.element = document.createElement("div");
        this.element.className = "program";
        this.makeView();
    }

    makeView() {
        let nameview = this.makeNameView();
        let timeview = this.makeTimeView();
        let travel = this.makeTravelView();
        let enabled = this.makeEnabledView();
        this.element.appendChild(nameview);
        this.element.appendChild(timeview);
        this.element.appendChild(travel);
        this.element.appendChild(enabled);
    }

    makeNameView() {
        var element = document.createElement('div');
        element.className = 'program-name-section';

        var text = document.createElement('span');
        text.className = 'program-name-label';
        text.innerHTML = "Program";
        element.appendChild(text);        
        
        this.name = this.makeNameTextField();
        element.appendChild(this.name.element);        
        return element;
    }
    
    makeNameTextField() {
        return new TextField((target) => { this.program.name = target.value,
                                           this.update(); },
                             'program-name', this.program.name, 20);
    }

    update() {
        this.controller.updateProgram(this.program);
    }
    
    makeTimeView() {
        var element = document.createElement('div');
        element.className = 'program-time-section';

        var text = document.createElement('span');
        text.className = 'program-time-label';
        text.innerHTML = "Wake up at";
        element.appendChild(text);        
        
        this.hour = this.makeHourSelector();
        this.minute = this.makeMinuteSelector();
        element.appendChild(this.hour.element);        
        element.appendChild(this.minute.element);
        return element;
    }
    
    makeHourSelector() {
        let options = [];
        for (let i = 0; i < 24; i++) {
            let selected = (this.program.hour == i);
            options.push({"value": i.toString(),
                          "label": i.toString(),
                          "selected": selected});
        }
        return new Selector((target) => { this.program.hour = parseInt(target.value);
                                          this.update(); },
                            'program-hour', options);
    }
    
    makeMinuteSelector() {
        let options = [];
        for (let i = 0; i < 60; i = i+5) {
            let selected = (this.program.minute == i);
            options.push({"value": i.toString(),
                          "label": i.toString(),
                          "selected": selected});
        }
        return new Selector((target) => { this.program.minute = parseInt(target.value);
                                          this.update(); },
                            'program-minute', options);
    }

    makeTravelView() {
        var element = document.createElement('div');
        element.className = 'program-travel-section';
        
        this.start = this.makeStartTextField();
        this.length = this.makeLengthTextField();
        this.interval = this.makeIntervalTextField();
        this.tilt = this.makeTiltTextField();
        
        let wrapper = this.makeTravelSection(this.start.element, 'start',
                                             'Start at position (m)');
        element.appendChild(wrapper);
        
        wrapper = this.makeTravelSection(this.length.element, 'length',
                                         'Travel over distance (m)');
        element.appendChild(wrapper);

        wrapper = this.makeTravelSection(this.interval.element, 'interval',
                                         'Take picture at interval (m)');
        element.appendChild(wrapper);

        wrapper = this.makeTravelSection(this.tilt.element, 'tilt',
                                         'Tilt camera at (°)');
        element.appendChild(wrapper);

        return element;
    }

    makeTravelSection(element, name, text) {
        let wrapper = document.createElement('div');
        wrapper.className = 'program-' + name + '-section';
        let label = document.createElement('span');
        label.className = 'program-' + name + '-label';
        label.innerHTML = text;
        wrapper.appendChild(label);
        wrapper.appendChild(element);
        return wrapper;
    }
    
    makeStartTextField() {
        return new TextField((target) => { this.program.start = parseFloat(target.value),
                                           this.update(); },
                             'program-start', this.program.start, 4);
    }
    
    makeLengthTextField() {
        return new TextField((target) => { this.program.length = parseFloat(target.value),
                                           this.update(); },
                             'program-length', this.program.length, 4);
    }
    
    makeIntervalTextField() {
        return new TextField((target) => { this.program.interval = parseFloat(target.value),
                                           this.update(); },
                             'program-interval', this.program.interval, 4);
    }
    
    makeTiltTextField() {
        return new TextField((target) => { this.program.tilt = parseFloat(target.value),
                                           this.update(); },
                             'program-tilt', this.program.tilt, 4);
    }
    
    makeEnabledView() {
        var element = document.createElement('div');
        element.className = "program-enabled-section";

        var text = document.createElement("span");
        text.className = "program-enabled-label";
        text.innerHTML = "Enabled";
        element.appendChild(text);        
        
        this.enabled = this.makeEnabledCheckbok();
        element.appendChild(this.enabled.element);        
        return element;
    }
    
    makeEnabledCheckbok() {
        return new Checkbox((target) => { this.program.enabled = target.value; 
                                          this.update(); },
                            'program-enabled', this.program.enabled);
    }
}

class ProgramListView
{
    constructor(programs, controller) {
        console.log('ProgramListView');
        this.programs = programs;
        this.controller = controller;
        this.element = document.createElement('div');
        this.element.className = 'program-list';
        this.updateProgramList();
    }

    updateProgramList() {
        console.log('ProgramListView.updateProgramList');
        this.clear();
        for (let i = 0; i < this.programs.count(); i++) {
            this.showProgram(this.programs.get(i));
        }
    }
    
    clear() {
        while (this.element.firstChild) {
            this.element.removeChild(this.element.firstChild);
        }
    }
    
    showProgram(program) {
        console.log('ProgramListView.showProgram');
        let view = new ProgramView(program, this.controller);
        this.element.appendChild(view.element);
    }
}

//

class ProgramListController
{
    constructor(programs, remoteController, elementID) {
        console.log('ProgramListController');
        this.programs = programs;
        this.remoteController = remoteController;
        this.elementID = elementID;
    }

    setValue(objectId, fieldId, value) {
        /*
        console.log("Set: " + fieldId + " " + value);
        let program = this.getProgram(objectId);
        if (program) {
            switch (fieldId) {
            case 'hour': program.hour = value:
                break;
            case 'hour': program.hour = value:
                break;
            }
            this.updateProgramWithId(id);
        }*/
    }

    connected() {
        console.log('ProgramListController.connected');
        this.remoteController.send({'method': 'get', 'params': {'object-id': 'programs'}});
    }  

    handleTextMessage(response) {
        if (response.method == "get") {
            console.log(JSON.stringify(response));
            if (response.method == 'get') {
                this.loadPrograms(response.result);
                this.displayPrograms();
            }
        } else if (response.method == "update") {
            console.log(JSON.stringify(response));
        } else {
            console.log('ProgramListController: unknown method: '
                        + JSON.stringify(response));
        }
    }  

    handleErrorMessage(error) {
        console.log('RemoteController: Error: ' + error.message);
    }  

    handleBinaryMessage(buffer) {
        console.log('ProgramListController.handleBinaryMessage: Not expected');
    }

    loadPrograms(list) {
        this.programs.clear();
        for (const p of list) {
            let program = new Program(p['id'], p['name'],
                                      p['hour'], p['minute'], 
                                      p['start-at'], p['length'], p['interval'],
                                      p['tilt'], p['enabled']);
            console.log(p['id'], p['name'],
                        p['start-at'], p['length'], p['interval'],
                        p['tilt'], p['enabled']);
            this.programs.add(program);
        }
    }

    displayPrograms() {
        let view = new ProgramListView(this.programs, this);
        let parent = document.getElementById(this.elementID);
        parent.appendChild(view.element);
    }

    updateProgram(program) {
        let request = {
            'method': 'update',
            'params': {
                'object-id': 'programs',
                'program': {
                    'id': program.id,
                    'name': program.name,
                    'hour': program.hour,
                    'minute': program.minute,
                    'start-at': program.start,
                    'length': program.length,
                    'interval': program.interval,
                    'tilt': program.tilt,
                    'enabled': program.enabled
                }
            }};
        console.log('ProgramListController.update: ' + JSON.stringify(request));
        this.remoteController.send(request);
    }
    
    updateProgramWithId(id) {
        let program = this.getProgram(id);
        this.updateProgram(program);
    }
    
    getProgram(id) {
        return this.programs.getId(id);
    }
}

//

class RemoteController
{
    constructor(name, registry) {
        this.name = name;
        this.registryIP = registry;
        this.remoteAddress = null;
        this.socket = null;
        this.connected = false;
        this.connectionHandlers = [];
        this.connect();
    }
    
    connect() {
        var registrySocket = new WebSocket('ws://' + this.registryIP + ':10101');

        registrySocket.onopen = (event) => {
            var request = { 'request': 'get', 'topic': this.name };
            registrySocket.send(JSON.stringify(request));
        };
        
        registrySocket.onmessage = (event) => {
            console.log(event.data);
            var reply = JSON.parse(event.data);
            if (reply.success) {
                registrySocket.close();
                this.remoteAddress = reply.address;
                this.socket = new WebSocket('ws://' + this.remoteAddress);
                this.socket.onmessage = (event) => {
                    this.tryHandleMessage(event.data);
                };
                this.socket.onopen = (event) => {
                    this.ignition();
                };
            }
        }
    }

    callWhenConnected(handler) {
        if (this.connected) {
            handler.connected();
        } else {
            this.connectionHandlers.push(handler);
        }
    }

    ignition() {
        this.connected = true;
        for (const handler of this.connectionHandlers) {
            handler.connected();
        }
        this.connectionHandlers = [];
    }

    tryHandleMessage(buffer) {
        try {
            this.handleMessage(buffer);
        } catch (error) {
            console.error(error);
        }
    }

    handleMessage(buffer) {
        if (this.isTextMessage(buffer)) {
            this.handleTextMessage(buffer);
        } else {
            this.handleBinaryMessage(buffer);
        }
    }
    
    isTextMessage(buffer) {
        return (typeof buffer === 'string' && buffer.charAt(0) == '{');
    }

    handleTextMessage(buffer) {
        let handler = this.getHandler(); 
        var response = JSON.parse(buffer);
        if (response.error) {
            this.handleErrorMessage(response);
        } else if (handler) {
            handler.handleTextMessage(response);
        } else {
            console.log('RemoteController: No handler');
        }
    }  

    handleErrorMessage(response) {
        let handler = this.getHandler(); 
        console.log('RemoteController: Method: ' + response.method
                    + ', Error: ' + response.error.message);
        if (handler) {
            handler.handleErrorMessage(response.error);
        } else {
            console.log('RemoteController: No handler');
        }
    }  

    handleBinaryMessage(buffer) {
        let handler = this.getHandler(); 
        if (handler) {
            handler.handleBinaryMessage(buffer);
        } else {
            console.log('RemoteControlle: No handler');
        }
    }  

    getHandler() {
        return programListController; // FIXME
    }
    
    send(request) {
        if (this.connected) {
            var s = JSON.stringify(request);
            this.socket.send(s);
        } else {
            throw 'The RemoteController is not connected!';
        }
    }
}

function initProgramList(name, registry)
{
    remoteController = new RemoteController(name, registry);
    programList = new ProgramList();

    {
        programList.add(new Program(0, "Test", 9, 30,
                                    1.0, 10.0, 0.1,
                                    45, true));
        
        programList.add(new Program(0, "Test2", 12, 30,
                                    1.0, 10.0, 0.1,
                                    45, true));
    }
    
    programListController = new ProgramListController(programList, remoteController,
                                                     'program-app');

    remoteController.callWhenConnected(programListController);
    
    
    //let app = document.getElementById('program-app');
    //app.appendChild(programListView.element);

    //remoteController.registerHandler('programs', programListController);
}
