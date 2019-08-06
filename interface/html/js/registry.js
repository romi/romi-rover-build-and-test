
var registry;

function Graph(nodes)
{
    this.nodes = nodes;
    this.list = [];
    this.mark = [];
    this.column = [];

    this.getIndex = function(node) {
        for (let i = 0; i < this.nodes.length; i++) 
            if (node == this.nodes[i])
                return i;
        return -1;
    }

    this.setNodeColumns = function() {
        for (let i = 0; i < this.nodes.length; i++)
            this.nodes[i].column = -1;
        this.sort();
        for (let i = 0; i < this.list.length; i++)
            this.setColumn(this.list[i]);
    }
    
    this.setColumn = function(node) {
        if (node.inputs.length == 0) {
            console.log("node " + node.name + " has zero inputs");
            node.column = 0;
        } else {
            var column = 0;
            for (let i = 0; i < node.inputs.length; i++) {
                var nextNode = node.inputs[i].fromNode;
                var c = nextNode.column;
                if (c == -1) return -1; // FAIL
                if (c > column)
                    column = nextNode.column;
            }
            node.column = column + 1;
        }
    }
    
    this.nextUnmarked = function() {
        for (let i = 0; i < this.mark.length; i++) {
            if (this.mark[i] == "no")
                return i;
        }
        return -1;
    }
    
    this.visit = function(n) {
        if (this.mark[n] == "yes")
            return 0;
        if (this.mark[n] == "temp")
            return -1; // FAIL
        this.mark[n] = "temp";
        var node = this.nodes[n];
        for (let i = 0; i < node.inputs.length; i++) {
            var nextNode = node.inputs[i].fromNode;
            var nextN = this.getIndex(nextNode);
            if (nextN == -1) return -1; // FAIL
            this.visit(nextN);
        }
        this.mark[n] == "yes";
        this.list.push(node);
    }
    
    this.sort = function() {
        for (let i = 0; i < this.nodes.length; i++)
            this.mark[i] = "no";
        while (true) {
            let n = this.nextUnmarked();
            if (n == -1) break;
            this.visit(n);
        }
    }
}


function Link(fromNode, fromEntryIndex, toEntryIndex)
{
    this.fromNode = fromNode;
    this.fromEntryIndex = fromEntryIndex;
    this.toEntryIndex = toEntryIndex;
}

function Entry(name, type, topic, addr)
{
    this.name = name;
    this.type = type;
    this.topic = topic;
    this.addr = addr;
    this.input = null;
    this.resources = [];
    
    this.appendTopicView = function(div) {
        var d = document.createElement("div");
        d.className = "entry " + this.type;
        
        var name = document.createElement("span");
        name.className = "entry-name";
        name.innerHTML = this.name;
        d.appendChild(name);
        
        div.appendChild(d);
    }

    this.appendNodeView = function(div) {
        var d = document.createElement("div");
        d.className = "entry " + this.type;
        
        var name = document.createElement("span");
        name.className = "entry-topic";
        name.innerHTML = this.topic;
        d.appendChild(name);
        
        d.appendChild(document.createElement("br"));
        
        var type = document.createElement("span");
        type.className = "entry-type";
        type.innerHTML = this.type;
        d.appendChild(type);
        
        div.appendChild(d);
    }

}

function Topic(name, type)
{
    this.convertType = function(type) {
        switch (type) {
        case "messagehub": return "messagehub"; break;
        case "messagelink": return "messagehub"; break;
        case "datahub": return "datahub"; break;
        case "datalink": return "datahub"; break;
        case "streamer": return "streamer"; break;
        case "streamerlink": return "streamer"; break;
        case "service": return "service"; break;
        default: return type; break;
        }        
    }
    
    this.matches = function(name, type) {
        type = this.convertType(type);
        return (this.name == name
                && this.type == type);
    }

    this.entries = [];
    this.name = name;
    this.type = this.convertType(type);
}

function Node(name)
{
    this.name = name;
    this.entries = [];
    this.inputs = [];
    
    this.getEntryIndex = function(topic, type) {
        for (let i = 0; i < this.entries.length; i++) {
            var e = this.entries[i];
            if (e.topic == topic && e.type == type)
                return i;
        }
        return -1;
    }
}

function IndexQuery(registry, entry)
{
    var self = this;
    this.registry = registry;
    this.entry = entry;

    this.handleResponse = function(data) {
        self.entry.resources = data.resources;
        self.registry.updateView();
    }
}

function Registry(uri)
{
    var self = this;

    this.entries = [];
    this.topics = [];
    this.nodes = [];

    this.sendRequest = function(request) {
        if (this.ws && this.ws.readyState == 1)
            this.ws.send(JSON.stringify(request))
    }

    this.requestList = function() {
        this.sendRequest({"request": "list"});
    }

    this.updateTopicsView = function() {
        var root = document.getElementById("topics-tab");
        while (root.firstChild) {
            root.removeChild(root.firstChild);
        }
        for (var i = 0; i < this.topics.length; i++) {
            var topic = this.topics[i];
            var div = document.createElement("div");
            div.className = "topic";
            var name = document.createElement("span");
            name.className = "topic-name";
            name.innerHTML = topic.name;
            div.appendChild(name);
            div.appendChild(document.createElement("br"));
            var t = document.createElement("span");
            t.className = "topic-type";
            t.innerHTML = topic.type;
            div.appendChild(t);
            
            for (var j = 0; j < topic.entries.length; j++)
                topic.entries[j].appendTopicView(div);
            
            root.appendChild(div);
        }
    }

    this.updateNodesView = function() {
        console.log("self.registry.updateNodesView");
        var root = document.getElementById("nodes-tab");
        while (root.firstChild) {
            root.removeChild(root.firstChild);
        }

        var draw = SVG('nodes-tab').size(1000, 1000);
        var y = [10, 10, 10, 10, 10, 10, 10, 10, 10, 10]; // FIXME!
        
        for (let i = 0; i < this.nodes.length; i++) {
            var node = this.nodes[i];
            var column = node.column;
            var group = draw.group();
            var height = 30 + node.entries.length * 21;
            
            var frame = group.rect(200, height).fill({color: '#fff', opacity: 0}).stroke({color: '#00aa5b', width: 5});
            group.plain(node.name).font({ family: 'Nunito', size: 18, weight: '600' }).move(8, 4);

            var h = 28;
            
            for (var j = 0; j < node.entries.length; j++) {
                var e = node.entries[j];
                
                var subframe = group.rect(190, 18).fill({color: '#fff', opacity: 0}).stroke({color: '#00aa5b', width: 1}).move(5, h);
                
                group.plain(e.topic).font({
                    family: 'Nunito',
                    size: 14, weight: '400' }).move(19, h);

                if (e.type == "messagehub"
                    || e.type == "datahub"
                    || e.type == "streamer") {
                    group.circle(6).fill('#00aa5b').move(187, h + 6);
                    e.connector = [187 + 3, h + 9];
                } else if (e.type == "messagelink"
                           || e.type == "datalink"
                           || e.type == "streamerlink") {
                    group.circle(6).fill('#00aa5b').move(7.5, h + 6);
                    e.connector = [7.5 + 3, h + 9];
                }
                
                if (e.resources && e.resources.length > 0) {
                    for (var k = 0; k < e.resources.length; k++) {
                        var r = e.resources[k];
                        h += 16;
                        var a = group.link(r.uri);
                        a.plain(r.name).font({family: 'Nunito',size: 14, weight: '400' }).move(30, h);
                    }
                }
                
                if (e.type == "streamer") {
                    subframe.size(190, 18 + 100);
                    group.image("http://" + e.addr + "/stream.html", 190, 100).move(5, h + 16);
                    h += 116;
                }
                
                h += 21;                
            }

            h += 3;
            frame.size(200, h);
            node.position = [2 + column * 250, y[column]];
            group.translate(2 + column * 250, y[column]);
            y[column] += h + 10;
        }
        for (let i = 0; i < this.nodes.length; i++) {
            console.log("lines node " + i + ": inputs " + this.nodes[i].inputs);
            var n0 = this.nodes[i];
            for (let j = 0; j < n0.inputs.length; j++) {
                console.log("lines node " + i + " entry " + j);
                var input = n0.inputs[j];
                var i0 = input.toEntryIndex;
                var e0 = n0.entries[i0];
                var n1 = input.fromNode;
                var i1 = input.fromEntryIndex;
                var e1 = n1.entries[i1];

                var x0 = n0.position[0] + e0.connector[0];
                var y0 = n0.position[1] + e0.connector[1];
                var x1 = n1.position[0] + e1.connector[0];
                var y1 = n1.position[1] + e1.connector[1];
                draw.line(x0, y0, x1, y1).stroke({ color: '#00aa5b', width: 1.5 });
            }
        }
    }

    this.updateView = function() {
        console.log("self.registry.updateView");
        this.updateTopicsView();
        this.updateNodesView();
    }
    
    this.getTopic = function(name, type) {
        for (var i = 0; i < this.topics.length; i++) {
            if (this.topics[i].matches(name, type))
                return this.topics[i];
        }
        var topic = new Topic(name, type);
        this.topics.push(topic);
        return topic;
    }

    this.updateTopics = function() {
        this.topics = [];
        for (var i = 0; i < this.entries.length; i++) {
            var e = this.entries[i];
            e = new Entry(e.name, e.type, e.topic, e.addr);
            var topic = this.getTopic(e.topic, e.type);
            if (e.type == "datahub" || e.type == "messagehub" || e.type == "streamer") 
                topic.entries.splice(0, 0, e);
            else topic.entries.push(e);
        }
    }

    this.getNode = function(name) {
        for (var i = 0; i < this.nodes.length; i++) {
            if (this.nodes[i].name == name)
                return this.nodes[i];
        }
        var node = new Node(name);
        this.nodes.push(node);
        return node;
    }

    this.findNodeEntry = function(topic, type) {
        for (var i = 0; i < this.nodes.length; i++) {
            var index = this.nodes[i].getEntryIndex(topic, type);
            if (index >= 0)
                return {node: this.nodes[i], index: index };
        }
        return false
    }
    
    this.updateNodes = function(list) {
        this.nodes = [];
        for (var i = 0; i < this.entries.length; i++) {
            var e = this.entries[i];
            e = new Entry(e.name, e.type, e.topic, e.addr);
            var node = this.getNode(e.name);
            if (e.type == "datahub" || e.type == "messagehub"
                || e.type == "streamer" || e.type == "service") 
                node.entries.splice(0, 0, e);
            else node.entries.push(e);
        }
        // FIXME
        console.log("checking inputs");
        for (var i = 0; i < this.nodes.length; i++) {
            var node = this.nodes[i];
            node.inputs = [];
            for (var j = 0; j < node.entries.length; j++) {
                var e = node.entries[j];
                if (e.type == "messagehub" || e.type == "datahub"
                    || e.type == "streamer" || e.type == "service")
                    continue;
                var type = null;
                switch (e.type) {
                case "messagelink": type = "messagehub"; break;
                case "datalink": type = "datahub"; break;
                case "streamerlink": type = "streamer"; break;
                }
                var nodeEntry = this.findNodeEntry(e.topic, type);
                if (nodeEntry)
                    node.inputs.push(new Link(nodeEntry.node, nodeEntry.index, j));
                else
                    console.log("no link for " + e.topic + ":" + type);
            }
        }
        this.graph = new Graph(this.nodes);
        this.graph.setNodeColumns();

        // Get the exported resources of the service nodes
        for (let i = 0; i < this.nodes.length; i++) {
            var node = this.nodes[i];
            node.inputs = [];
            for (var j = 0; j < node.entries.length; j++) {
                var e = node.entries[j];
                if (e.type == "service") {
                    $.getJSON("http://" + e.addr + "/index.json", new IndexQuery(this, e).handleResponse);
                }
            }
        }
    }

    this.updateList = function(list) {
        this.entries = list;
        this.updateTopics();
        this.updateNodes();
        this.updateView();
    }

    this.handleMessage = function(e) {
        if (!e.request) {
            console.log("Registry: message has no request string");

        } else if (e.request == "proxy-update-list") {
            this.updateList(e.list);

        } else if (e.request == "proxy-add") {
            this.entries.push(e.entry);
            this.updateTopics();
            this.updateNodes();
            this.updateView();

        } else if (e.request == "proxy-remove") {
            var found = false;
            for (var i = 0; i < this.entries.length; i++) {
                var entry = this.entries[i];
                if (entry.id == e.id) {
                    this.entries.splice(i, 1);
                    found = true;
                    break;
                }
            }
            if (!found) console.log("could not find entry with id " + e.id);
            this.updateTopics();
            this.updateNodes();
            this.updateView();

        } else {
            console.log("Registry: unknown request: " + e.request);
        }
    }

    if (uri) {
        this.ws = new WebSocket(uri);

        this.ws.onopen = function(e) {
            console.log("Registry: websocket open");
            self.requestList();
        }

        this.ws.onmessage = function(e) {
            console.log(e.data);
            self.handleMessage(JSON.parse(e.data));
        }

        this.ws.onclose = function(e) {
            console.log("Registry: websocket closed");
        }

        this.ws.onerror = function(e) {
            console.log("Registry: websocket error: " + e.toString());
        }
    }
}

function showRegistry(uri)
{
    registry = new Registry(uri);
}
