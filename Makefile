
.PHONY: tools
tools:
	$(MAKE) -C src tools
	mkdir tools
	cp src/tools/readwad tools/readwad
	cp src/tools/packwad tools/packwad

assets/%.ps2lvl: assets/%.json
	python assets/compile_level.py $< > $@

.PHONY: wad
wad: assets/test.ps2lvl
	./tools/packwad assets/*.tga assets/*.ps2lvl > src/assets.wad

