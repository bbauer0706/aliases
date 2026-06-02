from importlib.metadata import PackageNotFoundError, version

try:
    __version__ = version("aliases-cli")
except PackageNotFoundError:
    __version__ = "dev"
