/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

var db = null;
var dbViewer = null;
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

    this.insertFile = function(file) {
        this.files.push(file);
        this.files.sort(function(a, b) { return a.id > b.id; });
    }

    this.updateFiles = function(list) {
        for (let i = 0; i < list.length; i++) {
            let file = this.getFile(list[i].id);
            if (!file)
                this.insertFile(new File(list[i].id, list[i].mimetype));
        }
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

    this.insertFileset = function(fileset) {
        this.filesets.push(fileset);
        this.filesets.sort(function(a, b) { return a.id > b.id; });
    }
    
    this.updateFilesets = function(list) {
        for (let i = 0; i < list.length; i++) {
            let fileset = this.getFileset(list[i].id);
            if (!fileset)
                this.insertFileset(new Fileset(list[i].id));
        }
        this.loaded = true;
    }
}

function DB()
{
    this.uri = "/service/db";
    this.ws = "ws://" + window.location.host + "/messagehub/db";
    this.scans = [];
    this.loaded = false;
    
    this.getScan = function(id) {
        for (let i = 0; i < this.scans.length; i++) {
            if (this.scans[i].id == id)
                return this.scans[i];
        }
        return null;
    }

    this.insertScan = function(scan) {
        this.scans.push(scan);
        this.scans.sort(function(a, b) { return a.id > b.id; });    
    }
    
    this.updateScans = function(list) {
        for (let i = 0; i < list.length; i++) {
            let scan = this.getScan(list[i].id);
            if (!scan)
                this.insertScan(new Scan(list[i].id));
        }
        this.loaded = true;
    }
    
    this.handleNewFile = function(e) {
        var scan = this.getScan(e.scan);
        if (!scan) {
            console.log("New file: couldn't find scan " + e.scan);
            return;
        }
        if (!scan.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + e.scan, function (data) {
                scan.updateFilesets(data.filesets);
                handleEvent(e);
            });
            return;
        }
        var fileset = scan.getFileset(e.fileset);
        if (!fileset) {
            console.log("New file: couldn't find fileset " + e.fileset);
            return;
        }
        if (!fileset.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + e.scan + '/' + e.fileset,
                      function (data) {
                          fileset.updateFiles(data.files);
                          handleEvent(e);
                      });
            return;
        }
        var file = fileset.getFile(e.file);
        if (!file)
            fileset.insertFile(new File(e.file, e.mimetype));
        else
            file.mimetype = e.mimetype;
    }

    this.handleNewFileset = function(e) {
        var scan = this.getScan(e.scan);
        if (!scan) {
            console.log("New file: couldn't find scan " + e.scan);
            return;
        }
        if (!scan.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + e.scan, function (data) {
                scan.updateFilesets(data.filesets);
                handleEvent(e);
            });
            return;
        }
        var fileset = scan.getFileset(e.fileset);
        if (!fileset) {
            fileset = new Fileset(e.fileset)
            scan.insertFileset(fileset);
        }
        if (!fileset.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + e.scan + '/' + e.fileset,
                      function (data) {
                          fileset.updateFiles(data.files);
                      });
        }
    }

    this.handleNewScan = function(e) {
        var scan = this.getScan(e.scan);
        if (!scan) {
            scan = new Scan(e.scan);
            this.insertScan(scan);
        }
        if (!scan.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + e.scan, function (data) {
                scan.updateFilesets(data.filesets);
            });
        }
    }

    this.handleUpdate = function(e) {
        if (e.event == "new-file")
            this.handleNewFile(e);
        else if (e.event == "new-fileset")
            this.handleNewFileset(e);
        else if (e.event == "new-scan")
            this.handleNewScan(e);
    }
}

function DBViewer(db)
{
    this.db = db;
    this.selectedScan = "-";
    this.selectedFileset = "-";
    this.selectedFile = "-";
    this.selectedTab = "db";

    this.updateDBView = function() {
        var div = document.getElementById('db');
        while (div.firstChild)
            div.removeChild(div.firstChild);

        for (let i = 0; i < this.db.scans.length; i++) {
            let id = this.db.scans[i].id;
            var a = document.createElement('button');
            a.className = 'btn btn-light';
            a.innerHTML = id;
            a.onclick = new Selector(id, null, null).select;
            div.appendChild(a);
        }
    }

    this.updateScanView = function() {
        var div = document.getElementById('scan');
        while (div.firstChild)
            div.removeChild(div.firstChild);

        var tab = document.getElementById('scan-tab');
        if (!this.selectedScan) {
            tab.innerHTML = "-";
            return;
        }
        tab.innerHTML = this.selectedScan;
        
        let scan = this.db.getScan(this.selectedScan);
        if (!scan)
            return;
        if (!scan.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + scan.id, function (data) {
                scan.updateFilesets(data.filesets);
                updateView();
            });
            return;
        }

        for (let i = 0; i < scan.filesets.length; i++) {
            let a = document.createElement("button");
            a.className = "btn  btn-light";
            a.innerHTML = scan.filesets[i].id;
            a.onclick = new Selector(scan.id, scan.filesets[i].id, null).select;
            div.appendChild(a);
        }
    }

    this.showFilesetImages = function(scan, fileset, div) {
        let columns = 0;
        
        let row = document.createElement('div');
        row.className = 'row';
        div.appendChild(row);
        
        for (let i = 0; i < fileset.files.length; i++) {
            if (fileset.files[i].mimetype == 'image/jpeg'
                || fileset.files[i].mimetype == 'image/png'
                || fileset.files[i].mimetype == 'image/svg+xml') {
                let column = document.createElement('div');
                column.className = 'col-md-2';

                let thumb = document.createElement('div');
                thumb.className = 'thumbnail';
                
                let a = document.createElement('a');
                a.onclick = new Selector(scan.id, fileset.id,
                                         fileset.files[i].id).select;
                
                let img = document.createElement('img');
                img.className = 'preview';
                img.alt = fileset.files[i].id;
                img.src = (db.uri + '/data/db/' + scan.id + '/' + fileset.id + '/'
                           + fileset.files[i].id + "?thumbnail");
                
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

    this.updateFilesetView = function() {
        var div = document.getElementById('fileset');
        while (div.firstChild)
            div.removeChild(div.firstChild);

        tab = document.getElementById('fileset-tab');
        if (!this.selectedFileset) {
            tab.innerHTML = "-";
            return;
        }
        tab.innerHTML = this.selectedFileset;

        var scan = this.db.getScan(this.selectedScan);
        if (!scan || !scan.loaded)
            return;
        
        var fileset = scan.getFileset(this.selectedFileset);
        if (!fileset)
            return;
        if (!fileset.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + scan.id + '/' + fileset.id,
                      function (data) {
                          fileset.updateFiles(data.files);
                          updateView();
                      });
            return;
        }

        this.showFilesetImages(scan, fileset, div);
        
        for (let i = 0; i < fileset.files.length; i++) {
            
            if (fileset.files[i].mimetype != 'image/jpeg'
                && fileset.files[i].mimetype != 'image/png'
                && fileset.files[i].mimetype != 'image/svg+xml') {
                let a = document.createElement('button');
                a.className = 'btn  btn-light';
                a.innerHTML = fileset.files[i].id;
                a.onclick = new Selector(scan.id, fileset.id, fileset.files[i].id).select;
                div.appendChild(a);
            }
        }
    }
    
    this.updateFileView = function() {
        var div = document.getElementById('file');
        while (div.firstChild)
            div.removeChild(div.firstChild);
        
        var tab = document.getElementById('file-tab');
        if (!this.selectedFile) {
            tab.innerHTML = "-";
            return;
        }
        tab.innerHTML = this.selectedFile;

        var scan = this.db.getScan(this.selectedScan);
        if (!scan || !scan.loaded)
            return;
        var fileset = scan.getFileset(this.selectedFileset);
        if (!fileset || !fileset.loaded)
            return;

        var file = fileset.getFile(this.selectedFile);
        if (!file)
            return;
        if (!file.loaded) {
            $.getJSON(db.uri + '/metadata/db/' + scan.id + '/' + fileset.id + '/' + file.id,
                      function (data) {
                          file.updateMetadata(data);
                          updateView();
                      });
            return;
        }

        if (file.mimetype == 'image/jpeg'
            || file.mimetype == 'image/png'
            || file.mimetype == 'image/svg+xml') {
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
    
    this.updateView = function() {
        this.updateDBView();
        this.updateScanView();
        this.updateFilesetView();
        this.updateFileView();
    }

    this.selectTab = function(tab) {
        this.selectedTab = tab;
        $('[href="#' + tab + '"]').tab('show');
    }

    this.select = function(scanID, filesetID, fileID) {
        this.selectedScan = scanID;
        this.selectedFileset = filesetID;
        this.selectedFile = fileID;
        this.updateView();
        if (fileID)
            this.selectTab('file');
        else if (filesetID)
            this.selectTab('fileset');
        else if (scanID)
            this.selectTab('scan');
        else 
            this.selectTab('db');
    }

    this.handleUpdate = function(e) {
        this.db.handleUpdate(e);
        if (e.event == "new-file") {
            if (e.fileset == this.selectedFileset
                && this.selectedTab == "fileset")
                this.updateFilesetView();
        } else if (e.event == "new-fileset") {
            this.select(e.scan, e.fileset, null);
        } else if (e.event == "new-scan") {
            this.select(e.scan, null, null);
        }
    }
}

function updateView()
{
    dbViewer.updateView();
}

function handleEvent(e)
{
    dbViewer.handleUpdate(e);
}

function Selector(scanId, filesetId, fileId)
{
    var self = this;
    this.scanId = scanId;
    this.filesetId = filesetId;
    this.fileId = fileId;

    this.select = function(e) {
        dbViewer.select(scanId, filesetId, fileId);
    }
}

function initWebSocket(uri)
{
    ws = new WebSocket(uri);

    ws.onopen = function(e) {
        console.log('Registry: websocket open');
        //self.requestList();
    }

    ws.onmessage = function(e) {
        console.log(e.data);
        handleEvent(JSON.parse(e.data));
    }

    ws.onclose = function(e) {
        console.log('Registry: websocket closed');
    }

    ws.onerror = function(e) {
        console.log('Registry: websocket error: ' + e.toString());
    }
}

function showDB()
{
    db = new DB();
    dbViewer = new DBViewer(db);
    
    $.getJSON(db.uri + '/metadata/db', function (data) {
        console.log(JSON.stringify(data));
        db.updateScans(data.scans);
        updateView();
        initWebSocket(db.ws);
    });
}
