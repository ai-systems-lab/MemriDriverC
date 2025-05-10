from ._gpio import *

class GPIO:
    def __init__(self):
        self._lines = {}
        
    def init(self):
        return init()
        
    def deinit(self):
        for line in self._lines.values():
            release(line)
        self._lines.clear()
        return deinit()
        
    def setup(self, pin):
        line = setup(pin)
        self._lines[pin] = line
        return line
        
    def set(self, pin_or_line, value):
        if isinstance(pin_or_line, int):
            line = self._lines.get(pin_or_line)
            if line is None:
                raise ValueError(f"Pin {pin_or_line} not setup")
        else:
            line = pin_or_line
        return set(line, value)
        
    def release(self, pin):
        line = self._lines.pop(pin, None)
        if line is not None:
            release(line)

__all__ = ['init', 'deinit', 'setup', 'set', 'release', 'GPIO']