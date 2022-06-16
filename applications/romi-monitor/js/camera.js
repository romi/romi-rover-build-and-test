var remoteCamera = null;

function grab()
{
    var request = { "method": "camera-grab-jpeg-binary" };
    var s = JSON.stringify(request);
    const encoder = new TextEncoder()
    var message = encoder.encode(s)
    remoteCamera.send(message);
}  

function showImage(buffer)
{
    if (typeof buffer === 'string'
        && buffer.charAt(0) == '{') {

        var response = JSON.parse(buffer);
        console.log("showImage: error " + response.error.message);
        
    } else {
        var image = document.getElementById('camera');
        
        var reader = new FileReader();
        reader.onload = function(e) {
            image.src = reader.result;
        };
        reader.readAsDataURL(buffer);
    }
}

function initCamera(name, registry)
{
    registrySocket = new WebSocket("ws://" + registry + ":10101");

    registrySocket.onopen = function (event) {
        var request = { "request": "get", "topic": name };
        registrySocket.send(JSON.stringify(request));
    };

    registrySocket.onmessage = function (event) {
        console.log(event.data);
        var reply = JSON.parse(event.data);
        if (reply.success) {
            registrySocket.close();

            remoteCamera = new WebSocket("ws://" + reply.address);
            remoteCamera.onmessage = function (event) {
                showImage(event.data);
                grab();
            };
            remoteCamera.onopen = function (event) {
                grab();
            };
        }
    }
}
