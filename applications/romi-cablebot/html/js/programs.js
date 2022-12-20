var programListController = null;

class Program
{
    constructor(id, name, observationId, hour, minute,
                start, length, interval,
                tilt, enabled) {
        this.id = id;
        this.name = name;
        this.observationId = observationId;
        this.hour = hour;
        this.minute = minute;
        this.start = start;
        this.length = length;
        this.interval = interval;
        this.tilt = tilt;
        this.enabled = enabled;
    }

    
}

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
        let obsview = this.makeObservationIdView();
        let timeview = this.makeTimeView();
        let travel = this.makeTravelView();
        let enabled = this.makeEnabledView();
        this.element.appendChild(nameview);
        this.element.appendChild(obsview);
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

    makeObservationIdView() {
        var element = document.createElement('div');
        element.className = 'program-observationid-section';

        var text = document.createElement('span');
        text.className = 'program-observationid-label';
        text.innerHTML = "Observation ID";
        element.appendChild(text);        
        
        this.observationId = this.makeObservationIdTextField();
        element.appendChild(this.observationId.element);        
        return element;
    }
    
    makeObservationIdTextField() {
        return new TextField((target) => { this.program.observationId = target.value,
                                           this.update(); },
                             'program-observationid', this.program.observationId, 20);
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
        for (let i = 0; i < 60; i = i+1) {
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

    getObjectId() {
        return 'programs';
    }

    connected() {
        console.log('ProgramListController.connected');
        this.remoteController.invoke(this, 'get');
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
            let program = new Program(p['id'], p['name'], p['observation-id'],
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
        let parent = document.getElementById(this.elementID);
        let view = new ProgramListView(this.programs, this);
        parent.appendChild(view.element);
    }

    updateProgram(program) {
        let params = {
            'program': {
                'id': program.id,
                'name': program.name,
                'observation-id': program.observationId,
                'hour': program.hour,
                'minute': program.minute,
                'start-at': program.start,
                'length': program.length,
                'interval': program.interval,
                'tilt': program.tilt,
                'enabled': program.enabled
            }};
        this.remoteController.invoke(this, 'update', params);
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

function initProgramList(name, registry, remoteAddress)
{
    var remoteController = new RemoteController(name, registry, remoteAddress);
    var programList = new ProgramList();
    
    programListController = new ProgramListController(programList, remoteController,
                                                     'program-app');

    remoteController.callWhenConnected(programListController);
}
