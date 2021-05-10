import sys
import asyncio
import json
import argparse
from romi.rpc import Server

def unet(image_path, mask_path):
    print(f"Running Unet({image_path}, {mask_path})")
    
def handle_unet_request(params):
    image_path = params["image"]
    mask_path = params["mask"]
    return unet(image_path, mask_path)

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
    parser.add_argument('--registry', type=str, nargs='?', default="10.10.10.1",
                    help='Set the IP address of the registry')
    args = parser.parse_args()
    
    server = Server("python", { "unet": handle_unet_request }, args.registry)
    
    loop = asyncio.get_event_loop()
    loop.run_until_complete(init())
    asyncio.get_event_loop().run_forever()
    
