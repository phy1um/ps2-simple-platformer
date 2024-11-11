from types import SimpleNamespace
import struct

import ldtk

FILE_EXT = ".ps2lvl"
FMT_ID = b"PS2L"
VERSION = 0
NAME_LEN = 24
TILE_DEF_SIZE = 26
ASSET_DEF_SIZE = 10
AREA_DEF_SIZE = 24

def encode_tilemap_kind(i):
    if i == "collision":
        return 0
    elif i == "deco":
        return 1
    else:
        raise Exception(f"invalid tilemap kind: {i}")

def encode_area_event_kind(i):
    if i == "load_level":
        return 1
    else:
        raise Exception(f"invalid trigger area kind: {i}")
        
class Area(object):
    @staticmethod
    def from_ldtk(o):
        (x, y) = o.get_offset()
        (w, h) = o.get_dimensions() 
        event_kind = encode_area_event_kind(o.get_field("event_type"))
        return Area(x, y, w, h, event_kind, o.get_field("arg"))

    def __init__(self, x, y, w, h, kind, arg):
        self.bounds = (x, y, w, h)
        self.kind = kind
        self.arg = arg

class Map(object):
    @staticmethod
    def from_ldtk(o):
        (tiles_w, tiles_h) = o.get_dimensions()
        (ox, oy) = o.get_offset()
        m = Map(
                ox,
                oy,
                o.get_name(),
                tiles_w,
                tiles_h,
                0)
        m.map = o.get_tile_map()
        return m

    def __init__(self, ox, oy, kind, w, h, texture):
        self.offset = (ox, oy)
        self.kind = kind
        self.dimensions = (w, h)
        self.map = [0]*(w*h)
        self.texture = texture

    def set_tile(self, x, y, v):
        if x < 0 or x >= self.dimensions[0] or y < 0 or y >= self.dimensions[1]:
            raise Exception(f"invalid coordinate {x},{y}")
        index = self.dimensions[0]*y + x
        self.map[index] = v

    def get_tile(self, x, y):
        if x < 0 or x >= self.dimensions[0] or y < 0 or y >= self.dimensions[1]:
            raise Exception(f"invalid coordinate {x},{y}")
        index = self.dimensions[0]*y + x
        return self.map[index]

class DataChunk(object):
    def __init__(self, offset_start=0):
        self.offset_start = offset_start
        self.offset = offset_start
        self.allocs = {}
        self.chunks = {}

    def alloc(self, name, size):
        if name in self.allocs:
            raise Exception(f"double alloc: {name}")
        self.allocs[name] = (self.offset, size)
        out = self.offset 
        self.offset += size
        return out

    def write_to(self, name, bs):
        if name not in self.allocs:
            raise Exception(f"not allocated: {name}")
        if len(bs) > self.allocs[name][1]:
            raise Exception(f"data too big for allocation: {name}")
        self.chunks[name] = bs

    def to_blob(self):
        alloc_list = []
        for k, v in self.allocs.items():
            alloc_list.append((k, v[0]))
        alloc_list.sort(key=lambda x: x[1])
        last_offset = alloc_list[-1][1]
        # size = biggest offset + size of that allocation
        size = last_offset + self.allocs[alloc_list[-1][0]][1]
        bs = bytearray(size)
        for name, offset in alloc_list:
            local_offset = offset - self.offset_start
            if local_offset < 0:
                raise Exception(f"allocated chunk {name} before start: {local_offset}")
            bs[local_offset : local_offset + self.allocs[name][1]] = self.chunks[name]
        return bs

class Level(object):
    @staticmethod
    def from_ldtk(o):
        (ox, oy) = o.get_world_offset()
        level = Level(
                o.get_name(),
                ox, 
                oy)
        for asset in o.textures():
            as_tga = asset.replace(".png", ".tga")
            print(f"replaced texture name: {as_tga}")
            level.add_texture_asset(as_tga)
        for area in o.trigger_areas():
            level.add_area(Area.from_ldtk(area))
        for m in o.tile_layers():
            level.add_map(Map.from_ldtk(m))
        return level
   
    def __init__(self, name, ox, oy):
        self.name = name
        self.offset = (ox, oy)
        self.assets = []
        self.maps = []
        self.areas = []

    def add_texture_asset(self, a):
        self.assets.append(a)

    def add_map(self, m):
        self.maps.append(m)

    def add_area(self, a):
        self.areas.append(a)

    def _serialize_header(self):
        return struct.pack(f"<4s5H2i{NAME_LEN}s",
                         FMT_ID,
                         VERSION,
                         len(self.maps),
                         len(self.assets),
                         0,
                         len(self.areas),
                         self.offset[0],
                         self.offset[1],
                         bytes(self.name, "ascii"),
                        )

    def _serialize_tilemap_definitions(self, dc):
        bs = []
        for i,m in enumerate(self.maps):
            key = f"MAP:{i}"
            map_offset = dc.alloc(key, m.dimensions[0]*m.dimensions[1])
            bs.append(struct.pack("<2i2IHiI", m.offset[0], m.offset[1],
                               m.dimensions[0], m.dimensions[1],
                               encode_tilemap_kind(m.kind), m.texture, map_offset))
            dc.write_to(key, bytes(m.map))
        return b"".join(bs)

    def _serialize_asset_definitions(self, dc):
        bs = []
        for i,a in enumerate(self.assets):
            key = f"ASSET:{i}"
            name_offset = dc.alloc(key, len(a))
            bs.append(struct.pack("<HII", 0, name_offset, len(a)))
            dc.write_to(key, bytes(a, "ascii"))
        return b"".join(bs)

    def _serialize_area_definitions(self, dc):
        bs = []
        for i,a in enumerate(self.areas):
            key = f"AREA:{i}"
            arg_offset = dc.alloc(key, len(a.arg))
            print(f"arg len {a.arg} = {len(a.arg)}")
            bs.append(struct.pack("<ii5I", *a.bounds, a.kind, arg_offset, len(a.arg)))
            dc.write_to(key, bytes(a.arg, "ascii"))

        return b"".join(bs)

    def write_to_buffer(self, to):
        header_bytes = self._serialize_header()
        data_chunk = DataChunk(
                len(header_bytes) 
                + len(self.maps)*TILE_DEF_SIZE 
                + len(self.assets)*ASSET_DEF_SIZE
                + len(self.areas)*AREA_DEF_SIZE
                + 4)
        to.write(header_bytes)
        to.write(self._serialize_tilemap_definitions(data_chunk))
        to.write(self._serialize_asset_definitions(data_chunk))
        to.write(self._serialize_area_definitions(data_chunk))
        to.write(data_chunk.to_blob())

if __name__ == "__main__":
    import json
    import sys
    with open(sys.argv[1]) as f:
        o = json.load(f)
        world = ldtk.LDTK(o)
        for level in world.levels():
            lvl = Level.from_ldtk(level)
            path = "assets/" + lvl.name + FILE_EXT
            print(f"writing level to: {path}")
            with open(path, "wb") as f:
                lvl.write_to_buffer(f)

