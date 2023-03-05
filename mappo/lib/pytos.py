from http.client import HTTPConnection
import json

import importlib
import importlib.util

from os import path
import platform

from PIL import Image
import time
import numpy as np

os_type = platform.system()

# enable color
if os_type == "Windows":
    import ctypes
    kernel32 = ctypes.windll.kernel32
    kernel32.SetConsoleMode(kernel32.GetStdHandle(-11), 7)

DLL_PATH = path.join(path.dirname(__file__), "..", "..", "cytos", "out", "build", "x64-Release", "pytos.pyd")

spec = importlib.util.spec_from_file_location("pytos", DLL_PATH)
C_API = importlib.util.module_from_spec(spec)
spec.loader.exec_module(C_API)

class Pytos:
    def __init__(self, n_agents: int, threads: int, mode="ffa") -> None:
        self._handle = C_API.create(mode, n_agents, threads)

    def act(self, actions, steps=4):
        return C_API.act(self._handle, actions, steps)