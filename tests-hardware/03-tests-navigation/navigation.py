import sys
import asyncio
import json
import argparse
from romi.rpc import Client
from romi.rpc import create_client



class Navigation():
    def __init__(self, rpc_client):
        self.rpc_client = rpc_client

    async def move(self, distance, speed):
        await self.rpc_client.execute("navigation-move",
                                      speed=speed,
                                      distance=distance)

        
async def test(distance, speed):    
    loop = asyncio.get_event_loop()
    client = await create_client("navigation", "10.10.10.1")
    navigation = Navigation(client)
    await navigation.move(distance, speed)


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(test(1.0, 0.3))


    
