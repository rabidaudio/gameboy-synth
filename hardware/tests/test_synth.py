import ctypes
import os
import pytest

@pytest.fixture(scope="module")
def c_lib():
    """Fixture to load the compiled C shared library."""
    lib_path = os.path.abspath("./build/test/synth.so")
    lib = ctypes.CDLL(lib_path)
    
    # lib.add.argtypes = [ctypes.c_int, ctypes.c_int]
    # lib.add.restype = ctypes.c_int
    
    return lib

@pytest.fixture(scope="module")
def registers(c_lib):
    r = ctypes.c_uint8.in_dll(c_lib, "REGISTERS")
    return ctypes.cast(ctypes.pointer(r), ctypes.POINTER(ctypes.c_uint8*40))

@pytest.fixture(scope="module")
def read_register(registers):
    def _read(addr):
        return registers.contents[addr - 0xFF00]
    return _read

def test_synth_init(c_lib, read_register):
    c_lib.synth_init()
    assert read_register(0xFF10) == 0
    assert read_register(0xFF26) == 0x8F
    assert read_register(0xFF24) == 0x77
