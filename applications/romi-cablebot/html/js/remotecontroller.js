
class RemoteController
{
    constructor(name, registryIP, remoteAddress) {
        this.name = name;
        this.registryIP = registryIP;
        this.remoteAddress = remoteAddress;
        this.socket = null;
        this.connected = false;
        this.connectionHandlers = [];
        this.handler = null;
        this.connect();
    }
    
    connect() {
        if (this.remoteAddress)
            this.connectToDevice();
        else
            this.findRemoteAddress();
    }

    findRemoteAddress() {
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
                this.connectToDevice();
            }
        }
    }

    connectToDevice() {
        this.socket = new WebSocket('ws://' + this.remoteAddress);
        this.socket.onmessage = (event) => {
            this.tryHandleMessage(event.data);
        };
        this.socket.onopen = (event) => {
            this.ignition();
        };
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
        return this.handler;
        // return programListController; // FIXME
    }
    
    invoke(obj, method, params) {
        if (!params)
            params = {};
        params['object-id'] = obj.getObjectId();
        this.handler = obj; // FIXME
        this.send({'method': method, 'params': params });
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
