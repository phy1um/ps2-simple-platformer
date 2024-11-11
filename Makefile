
.PHONY: tools
tools:
	$(MAKE) -C src tools
	mkdir tools
	cp src/tools/readwad tools/readwad
	cp src/tools/packwad tools/packwad

.PHONY: wad
wad: levels
	./tools/packwad assets/*.tga assets/*.ps2lvl > src/assets.wad

.PHONY: levels
levels: 
	python assets/compile_ldtk.py game_world.ldtk
