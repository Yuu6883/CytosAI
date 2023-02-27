from http.client import HTTPConnection
import json
import time

AGENTS = 8

def main():
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
        while True:
            conn.request("POST", "/act", body=data)

            res = conn.getresponse()
            res.read()

            if res.status != 200:
                print("failed to act")
                return
            
    except:
        print("end")

if __name__ == "__main__":
    main()