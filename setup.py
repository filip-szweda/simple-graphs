from setuptools import setup
from setuptools import Extension

setup(
    name='simple_graphs',
    version=1,
    ext_modules=[Extension('simple_graphs', ['simple_graphs.c'])],
)