def get_image_width(s):
    return 64

class _LDTKBase(object):
    def __init__(self, o):
        self._dict = o

    def get_name(self):
        if "identifier" in self._dict:
            return self._dict["identifier"].lower()
        else:
            return self._dict["__identifier"].lower()

    def get_id(self):
        return self._dict["iid"]

    def get_type(self):
        return self._dict["__type"]

class _LDTKLayerBase(_LDTKBase):
    def __init__(self, o):
        super().__init__(o)

    def get_offset(self):
        return (int(self._dict["pxOffsetX"]), int(self._dict["pxOffsetY"]))

    def get_grid_size(self):
        return self._dict["__gridSize"]

    def get_dimensions(self):
        return (int(self._dict["__cWid"]), int(self._dict["__cHei"]))

    def get_kind(self):
        return "unknown"

class LDTKCollisionLayer(_LDTKBase):
    def __init__(self, o):
        super().__init__(o)

class LDTKTileLayer(_LDTKLayerBase):
    def __init__(self, o):
        super().__init__(o)

    def get_tile_map(self):
        (w, h) = self.get_dimensions()
        grid = self.get_grid_size()
        out = [0]*(w*h)
        tileset_img = self._dict["__tilesetRelPath"]
        src_width = get_image_width(tileset_img)
        for tile in self._dict["gridTiles"]:
            [x, y] = tile["px"]
            [sx, sy] = tile["src"]
            grid_x = x//grid
            grid_y = y//grid
            ind = grid_y*w + grid_x
            v = (sy//grid)*(src_width//grid) + (sx//grid)
            out[ind] = v
            # print(f"set map [deco]: {ind} = {v} ({sx}, {sy} = {v})")
        return out

    def get_kind(self):
            return "deco"

    def get_tileset(self):
        if "__tilesetRelPath" in self._dict:
            return self._dict["__tilesetRelPath"]
        else:
            return ""

class LDTKIntGridLayer(_LDTKLayerBase):
    def __init__(self, o):
        super().__init__(o)

    def get_tile_map(self):
        (w, h) = self.get_dimensions()
        out = [0]*(w*h)
        for i, v in enumerate(self._dict["intGridCsv"]):
            if v != 0:
                print(f"set map [collision]: {i} = {v}")
            out[i] = v
        return out

    def get_kind(self):
            return "collision"

class LDTKLevel(_LDTKBase):
    def __init__(self, o):
        super().__init__(o)

    def get_world_offset(self):
        return (int(self._dict["worldX"]), int(self._dict["worldY"]))

    def textures(self):
        for layer in self.tile_layers():
            if layer.get_kind() == "deco":
                yield layer.get_tileset()
        # TODO: iterate over entities too

    def tile_layers(self):
        for layer in self._dict["layerInstances"]:
            if layer["__type"] == "Tiles":
                yield LDTKTileLayer(layer)
            elif layer["__type"] == "IntGrid":
                yield LDTKIntGridLayer(layer)

    def entity_layers(self):
        for layer in self._dict["layerInstances"]:
            if layer["__type"] == "Entities":
                yield LDTKEntityLayer(layer)

class LDTK(object):
    def __init__(self, o):
        self._dict = o

    def levels(self):
        for l in self._dict["levels"]:
            yield LDTKLevel(l)

if __name__ == "__main__":
    import sys
    import json
    with open(sys.argv[1]) as f:
        o = json.load(f)
        world = LDTK(o)
        for level in world.levels():
            tiles = list(level.tile_layers())
            textures = list(level.textures())
            print(f"level {level.get_name()} @ {level.get_world_offset()}")
            print(f" - textures: {textures}")
            print(" - tile maps:")
            for tile in tiles:
                print(f"   - size = {tile.get_dimensions()}, @ {tile.get_offset()}, type = {tile.get_kind()}")
