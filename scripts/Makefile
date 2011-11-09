all:
	sh builddeps.sh
	cd ../src/lua/ && make `cat ../../scripts/.luatarget`
clean:
	rm -rf ./.depsarebuilt
	rm -rf ../libs/*.a
