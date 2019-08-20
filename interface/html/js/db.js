
var db = null;
var ws = null;
var selectedScan = null;
var selectedFileset = null;
var selectedFile = null;

function File(id, mimetype)
{
    this.id = id;
    this.mimetype = mimetype;
    this.loaded = false;
    this.metadata = {};

    this.updateMetadata = function(data) {
        this.metadata = data;
        this.loaded = true;
    }
}

function Fileset(id)
{
    this.id = id;
    this.files = [];
    this.loaded = false;

    this.getFile = function(id) {
        for (let i = 0; i < this.files.length; i++) {
            if (this.files[i].id == id)
                return this.files[i];
        }
        return null;
    }

    this.updateFiles = function(list) {
        for (let i = 0; i < list.length; i++) {
            let file = this.getFile(list[i].id);
            if (!file)
                this.files.push(new File(list[i].id, list[i].mimetype));
        }
        this.files.sort(function(a, b) { return a.id > b.id; });
        this.loaded = true;
    }
}

function Scan(id)
{
    this.id = id;
    this.filesets = [];
    this.loaded = false;

    this.getFileset = function(id) {
        for (let i = 0; i < this.filesets.length; i++) {
            if (this.filesets[i].id == id)
                return this.filesets[i];
        }
        return null;
    }

    this.updateFilesets = function(list) {
        for (let i = 0; i < list.length; i++) {
            let fileset = this.getFileset(list[i].id);
            if (!fileset)
                this.filesets.push(new Fileset(list[i].id));
        }
        this.filesets.sort(function(a, b) { return a.id > b.id; });
        this.loaded = true;
    }
}

function DB(uri, ws)
{
    this.uri = uri;
    this.ws = ws;
    this.scans = [];
    this.loaded = false;
    
    this.getScan = function(id) {
        for (let i = 0; i < this.scans.length; i++) {
            if (this.scans[i].id == id)
                return this.scans[i];
        }
        return null;
    }

    this.updateScans = function(list) {
        for (let i = 0; i < list.length; i++) {
            let scan = this.getScan(list[i].id);
            if (!scan)
                this.scans.push(new Scan(list[i].id));
        }
        this.scans.sort(function(a, b) { return a.id > b.id; });
        this.loaded = true;
    }
}

function showScan(scanID)
{
    var div = document.getElementById('scan');
    while (div.firstChild)
        div.removeChild(div.firstChild);

    var tab = document.getElementById('scan-tab');

    if (!scanID) {
        tab.innerHTML = "-";
        return;
    }

    tab.innerHTML = scanID;
    $('[href="#scan"]').tab('show');
    
    var scan = db.getScan(scanID);
    if (!scan) return;
    if (!scan.loaded) {
        $.getJSON(db.uri + '/metadata/db/' + scanID, function (data) {
            scan.updateFilesets(data.filesets);
            showScan(scanID);
        });
        return;
    }

    for (let i = 0; i < scan.filesets.length; i++) {
        let a = document.createElement("button");
        a.className = "btn  btn-light";
        a.innerHTML = scan.filesets[i].id;
        a.onclick = new Selector(scanID, scan.filesets[i].id, null).updateBrowser;
        div.appendChild(a);
    }
}

function showFilesetImages(scan, fileset, div)
{
    let columns = 0;
    
    let row = document.createElement('div');
    row.className = 'row';
    div.appendChild(row);
    
    for (let i = 0; i < fileset.files.length; i++) {
        if (fileset.files[i].mimetype == 'image/jpeg') {
            let column = document.createElement('div');
            column.className = 'col-md-2';

            let thumb = document.createElement('div');
            thumb.className = 'thumbnail';
            
            let a = document.createElement('a');
            a.onclick = new Selector(scan.id, fileset.id,
                                     fileset.files[i].id).updateBrowser;
            
            let img = document.createElement('img');
            img.className = 'preview';
            img.alt = fileset.files[i].id;
            img.src = (db.uri + '/data/db/' + scan.id + '/' + fileset.id + '/'
                       + fileset.files[i].id);
            
            let caption = document.createElement('div');
            caption.className = 'caption text-center';
            caption.innerHTML = fileset.files[i].id;
            
            a.appendChild(img);
            a.appendChild(caption);
            thumb.appendChild(a);
            column.appendChild(thumb);
            row.appendChild(column);
            
            columns += 1;
            if (columns == 6) {
                row = document.createElement('div');
                row.className = 'row';
                div.appendChild(row);
                columns = 0;
            }
        }
    }
}

function showFileset(scanID, filesetID)
{
    var div = document.getElementById('fileset');
    while (div.firstChild)
        div.removeChild(div.firstChild);

    var tab = document.getElementById('fileset-tab');

    if (!filesetID) {
        tab.innerHTML = "-";
        return;
    }

    tab.innerHTML = filesetID;
    $('[href="#fileset"]').tab('show');

    var scan = db.getScan(scanID);
    if (!scan) return;
    if (!scan.loaded) {
        $.getJSON(db.uri + '/metadata/db/' + scanID, function (data) {
            scan.updateFilesets(data.filesets);
            showFileset(scanID, filesetID);
        });
        return;
    }

    var fileset = scan.getFileset(filesetID);
    if (!fileset) return;
    if (!fileset.loaded) {
        $.getJSON(db.uri + '/metadata/db/' + scanID + '/' + filesetID,
                  function (data) {
                      fileset.updateFiles(data.files);
                      showFileset(scanID, filesetID);
                  });
        return;
    }

    showFilesetImages(scan, fileset, div);
    
    for (let i = 0; i < fileset.files.length; i++) {
        
        if (fileset.files[i].mimetype != 'image/jpeg') {
            let a = document.createElement('button');
            a.className = 'btn  btn-light';
            a.innerHTML = fileset.files[i].id;
            a.onclick = new Selector(scanID, filesetID, fileset.files[i].id).updateBrowser;
            div.appendChild(a);
        }
    }
}

function showFile(scanID, filesetID, fileID)
{
    console.log('showFile');
    
    var div = document.getElementById('file');
    while (div.firstChild)
        div.removeChild(div.firstChild);

    if (!fileID)
        return;
    
    var tab = document.getElementById('file-tab');

    if (!fileID) {
        tab.innerHTML = "-";
        return;
    }

    tab.innerHTML = fileID;
    $('[href="#file"]').tab('show');

    var scan = db.getScan(scanID);
    if (!scan) return;
    if (!scan.loaded) {
        $.getJSON(db.uri + '/metadata/db/' + scanID, function (data) {
            scan.updateFilesets(data.filesets);
            showFile(scanID, filesetID, fileID);
        });
        return;
    }

    var fileset = scan.getFileset(filesetID);
    if (!fileset) return;
    if (!fileset.loaded) {
        $.getJSON(db.uri + '/metadata/db/' + scanID + '/' + filesetID,
                  function (data) {
                      fileset.updateFiles(data.files);
                      showFile(scanID, filesetID, fileID);
                  });
        return;
    }

    var file = fileset.getFile(fileID);
    if (!file) return;
    if (!file.loaded) {
        $.getJSON(db.uri + '/metadata/db/' + scanID + '/' + filesetID + '/' + fileID,
                  function (data) {
                      file.updateMetadata(data);
                      showFile(scanID, filesetID, fileID);
                  });
        return;
    }

    if (file.mimetype == 'image/jpeg') {
        let row = document.createElement('div');
        row.className = 'row';
        div.appendChild(row);
    
        let column = document.createElement('div');
        column.className = 'col-md-12';

        let img = document.createElement('img');
        img.className = 'fullview';
        img.src = (db.uri + '/data/db/' + scan.id + '/' + fileset.id + '/' + file.id);
            
        column.appendChild(img);
        row.appendChild(column);            
    }

    let row = document.createElement('div');
    row.className = 'row';
    div.appendChild(row);
    let column = document.createElement('div');
    column.className = 'col-md-12';   
    column.innerHTML = JSON.stringify(file.metadata);
    row.appendChild(column);            
}

function updateDBBrowser()
{
    console.log('updateDBBrowser');
    var div = document.getElementById('db');
    while (div.firstChild)
        div.removeChild(div.firstChild);
    
    for (let i = 0; i < db.scans.length; i++) {
        let a = document.createElement('button');
        a.className = 'btn btn-light';
        a.innerHTML = db.scans[i].id;
        a.onclick = new Selector(db.scans[i].id, null, null).updateBrowser;
        div.appendChild(a);
    }
}

function updateBrowser()
{
    updateDBBrowser();
}

function Selector(scanId, filesetId, fileId)
{
    var self = this;
    this.scanId = scanId;
    this.filesetId = filesetId;
    this.fileId = fileId;

    this.updateBrowser = function(e) {
        
        if (fileId) {
            showFile(scanId, filesetId, fileId);
        } else if (filesetId) {
            showFileset(scanId, filesetId);
        } else if (scanId) {
            showScan(scanId);
        }
    }
}

function handleEvent(data)
{
    console.log(JSON.stringify(data));
}

function initWebSocket(uri)
{
    ws = new WebSocket(uri);

    ws.onopen = function(e) {
        console.log('Registry: websocket open');
        self.requestList();
    }

    ws.onmessage = function(e) {
        handleEvent(JSON.parse(e.data));
    }

    ws.onclose = function(e) {
        console.log('Registry: websocket closed');
    }

    ws.onerror = function(e) {
        console.log('Registry: websocket error: ' + e.toString());
    }
}

function showDB(uri, ws)
{
    console.log('showDB');
    db = new DB(uri, ws);
    $.getJSON(uri + '/metadata/db', function (data) {
        console.log('showDB > getJSON > callback');
        db.updateScans(data.scans);
        updateDBBrowser();
        initWebSocket(ws);
    });
}
