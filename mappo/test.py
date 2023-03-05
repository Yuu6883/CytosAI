from http.client import HTTPConnection
import json
import importlib
import importlib.util
from os import path
import platform
from PIL import Image
import time
import numpy as np

from lib.pytos import Pytos

# gc.set_debug(gc.DEBUG_LEAK)

AGENTS = 8
actions = [{"splits": 0, "ejects": 0, "lock_cursor": False, "cursor_x": 0, "cursor_y": 0} for _ in range(AGENTS)]

def main():
    game = Pytos(n_agents=AGENTS, threads=8)

    start_time = time.time()
    
    for i in range(10_000):
        results = game.act(actions, steps=4)

    end_time = time.time()

    print(f"Elapsed time: {end_time - start_time} seconds")

    for i in range(len(results)):
        state = results[i]["state"]
        img = Image.fromarray(state)
        img.save(f"pytos-agent#{i}.png")

def connect():
    conn = HTTPConnection("localhost:3000")
    conn.request("POST", f"/init?agents={AGENTS}")
    res = conn.getresponse()
    res.read()

    if res.status != 200:
        print("failed to initialize agents")
        return
    
    data = json.dumps({ "actions": actions, "steps": 4 })

    try:
        frame = 0
        while True:
            conn.request("POST", "/act", body=data)

            res = conn.getresponse()
            res.read()

            if res.status != 200:
                print("failed to act")
                return
            
            frame = frame + 1
            if frame > 50:
                break
    except:
        pass

if __name__ == "__main__":
    main()
    print("end")