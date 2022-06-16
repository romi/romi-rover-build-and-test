import asyncio
import argparse
from romi.rpc import Server

def unet_handle_request(params):
    image_path = params["path"]
    output_name = params["output-name"]
    print(f"New request, image: {image_path}, output name: {output_name}")
    # now = time.time()
    # print(f"handle_unet_request: {now-start_time:0.3f} seconds")
    return True

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
    parser.add_argument('--registry', type=str, nargs='?', default="10.0.3.35",
                    help='Set the IP address of the registry')
    parser.add_argument('--ip', type=str, nargs='?', default="10.0.3.35",
                    help='The local IP address to use')
    args = parser.parse_args()

    server = Server("python",
                    {
                        "unet": unet_handle_request
                    },
                    args.registry, args.ip)

    loop = asyncio.get_event_loop()
    loop.run_until_complete(init())
    asyncio.get_event_loop().run_forever()
