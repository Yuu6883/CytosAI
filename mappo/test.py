from http.client import HTTPConnection
import json
import importlib
import importlib.util
from os import path
import platform

os_type = platform.system()

# enable color
if os_type == "Windows":
    import ctypes
    kernel32 = ctypes.windll.kernel32
    kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)

DLL_PATH = path.join(path.dirname(__file__), "..", "cytos", "out", "build", "x64-Release", "pytos.pyd")

spec = importlib.util.spec_from_file_location("pytos", DLL_PATH)
pytos = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pytos)

AGENTS = 8
actions = [{"splits": 0, "ejects": 0, "lock_cursor": False, "cursor_x": 0, "cursor_y": 0} for _ in range(AGENTS)]

def main():
    game = pytos.create("ffa", AGENTS, 8)
    
    for _ in range(1000):
        pytos.act(game, actions, 4)

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