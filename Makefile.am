
SUBDIRS = scripts src tests

clean:
	cd libs && rm -f libblitwizard*.a
	cd libs && rm -f libimglib.a
install:
	@echo "Install is currently not supported."
	@echo "Please copy bin/blitwizard and templates/ into the project folders"
	@echo "where you wish to use blitwizard, and run it with bin/blitwizard."
	@echo "If you are on Microsoft Windows, copy Run-blitwizard aswell and"
	@echo "use that to run blitwizard."
cppcheck:
	cppcheck --enable=all -q -f src/*.c src/*.h
clanganalyze:
	cd src && clang --analyze $(CFLAGS) -DVERSION=$(VERSION) ./*.c
