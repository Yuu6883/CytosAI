from http.client import HTTPConnection
import json
import importlib
import importlib.util
from os import path

DLL_PATH = path.join(path.dirname(__file__), "..", "cytos", "out", "build", "x64-Release", "test.pyd")

spec = importlib.util.spec_from_file_location("my_module", DLL_PATH)
my_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(my_module)

def main():
    arr = my_module.create_array()
    print(arr)


AGENTS = 8

def connect():
    conn = HTTPConnection("localhost:3000")
    conn.request("POST", f"/init?agents={AGENTS}")
    res = conn.getresponse()
    res.read()

    if res.status != 200:
        print("failed to initialize agents")
        return
    
    actions = [{"splits": 0, "ejects": 0, "lock_cursor": False, "cursor_x": 0, "cursor_y": 0} for _ in range(AGENTS)]
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
    
    print("end")

if __name__ == "__main__":
    main()