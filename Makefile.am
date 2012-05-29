
SUBDIRS = scripts src tests

clean:
	cd libs && rm -f libblitwizard*.a
	cd libs && rm -f libimglib.a
install:
cppcheck:
	cppcheck --enable=all -q -f src/*.c src/*.h
clanganalyze:
	cd src && clang --analyze $(CFLAGS) -DVERSION=$(VERSION) ./*.c
