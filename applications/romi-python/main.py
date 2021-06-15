import asyncio
import argparse
from romi.rpc import Server

from unet import unet_init, unet_handle_request
from svm import svm_init, svm_handle_request


async def server_callback(websocket, path):
    global server
    await server.handle_client(websocket)
    
async def init():    
    global server
    loop = asyncio.get_event_loop()
    registration = loop.create_task(server.register())
    await registration
    await server.start(server_callback)

    
if __name__ == "__main__":
    global server

    parser = argparse.ArgumentParser()
    parser.add_argument('--model-path', type=str, nargs='?', default="models",
                        help='Set the path to the model data')
    parser.add_argument('--svm-path', type=str, nargs='?', default=".",
                        help='Set the path to the SVM data files')
    parser.add_argument('--registry', type=str, nargs='?', default="10.10.10.1",
                    help='Set the IP address of the registry')
    parser.add_argument('--ip', type=str, nargs='?', default="10.10.10.1",
                    help='The local IP address to use')
    args = parser.parse_args()

    unet_init(args.model_path)
    svm_init(args.svm_path)

    server = Server("python",
                    {
                        "unet": unet_handle_request,
                        "svm": svm_handle_request
                    },
                    args.registry, args.ip)

    loop = asyncio.get_event_loop()
    loop.run_until_complete(init())
    asyncio.get_event_loop().run_forever()
