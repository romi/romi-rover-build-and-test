import asyncio
import argparse
from romi.rpc import Server

#from unet import unet_init, unet_handle_request
#from svm import svm_init, svm_handle_request
from svm0 import svm0_init, svm0_handle_request
from nav import nav_init, nav_handle_request
#from triple import triple_init, triple_handle_request


async def server_callback(websocket, path):
    global server
    print("Handling new request")
    await server.handle_client(websocket)
    
async def init():    
    global server
    print("Running main.init")
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
    parser.add_argument('--nav-path', type=str, nargs='?',
                        default="workshop_1000.json",
                        help='Set the path to the SVM config for navigation')
    parser.add_argument('--triple-svm-1', type=str, nargs='?',
                        default="svm0/blue_tags.json",
                        help='Set the path to the "include" SVM')
    parser.add_argument('--triple-svm-2', type=str, nargs='?',
                        default="svm0/yellow-hose_0010_1000.json",
                        help='Set the path to the "exclude" SVM')
    parser.add_argument('--registry', type=str, nargs='?', default="10.10.10.1",
                    help='Set the IP address of the registry')
    parser.add_argument('--ip', type=str, nargs='?', default="10.10.10.1",
                    help='The local IP address to use')

    print("Parsing arguments")
    args = parser.parse_args()

    print("Starting server: IP %s, registry at %s" % (args.ip, args.registry))
    server = Server("python",
                    {
                        #"unet": unet_handle_request,
                        #"svm": svm_handle_request,
                        "svm": svm0_handle_request,
                        "nav": nav_handle_request,
                        #"triple": triple_handle_request
                    },
                    args.registry, args.ip)

    #unet_init(args.model_path)
    #svm_init(args.svm_path)
    svm0_init(args.svm_path)
    nav_init(args.nav_path)
    #triple_init(args.triple_svm_1, args.triple_svm_2)

    print("Starting event handling")
    loop = asyncio.get_event_loop()
    loop.run_until_complete(init())
    asyncio.get_event_loop().run_forever()
