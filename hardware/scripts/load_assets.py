import argparse
import yaml
from PIL import Image, ImageOps
from pathlib import Path

def main():
    parser = argparse.ArgumentParser("load_assets")
    # parser.add_argument("config", )
    # output

    config_path = "assets/assets.yml"
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)

    out_path = "src/assets.h"
    with open(out_path, 'w+') as f:
        for tileset in config["assets"]["tiles"]:
            f.write(load_tileset(tileset))

def load_tileset(opts):
    im = Image.open(Path("assets") / Path(opts["path"]))
    im = ImageOps.grayscale(im)
    data = []
    for x in range(0, im.width, opts["size"][0]):
        for y in range(0, im.height, opts["size"][1]):
            # https://gbdev.io/pandocs/Tile_Data.html#data-format
            tile = [0 for _ in range(0, 16)]
            # for i in range(16):
            #     tile[i] = sum([
            #         (v << )
            #     ])
            # for xx in range(0, opts["size"][0]):
            #     for yy in range(0, opts["size"][1]):
            #         pix = im.getpixel((xx, yy)) >> 6
            #         tile[0]