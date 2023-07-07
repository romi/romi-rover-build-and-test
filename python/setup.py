from setuptools import setup, find_packages

with open("Readme.md", "r") as fh:
    long_description = fh.read()

setup (
    name = "romi",
    packages = find_packages(include=['romi', 'romi.*']),
    version = '0.1.0',
    author = 'Peter Hanappe',
    description = 'Python package for the ROMI hardware devices.',
    long_description = long_description,
    long_description_content_type = "text/markdown",
    url = "https://github.com/romi/libromi/",
    install_requires=[
        'pyserial',
        'asyncio',
        'websockets'
    ],
    python_requires = '>=3.6',
    classifiers=[
        "Programming Language :: Python :: 3",
        "Development Status :: 4 - Beta",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)"
    ]
)
