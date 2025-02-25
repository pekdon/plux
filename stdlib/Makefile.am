stdlibdir = $(pkgdatadir)/stdlib
stdlib_DATA = nothing.pluxinc \
	sh-eval.pluxinc \
	sh-if.pluxinc \
	sh-if-else.pluxinc \
	sh-nok.pluxinc \
	sh-ok.pluxinc \
	sh-which.pluxinc

dist_noinst_DATA = stdlib_builtins.hh

stdlib_builtins.hh: $(stdlib_DATA)
	./gen_stdlib_builtins.sh . stdlib_builtins.hh

CLEANFILES = stdlib_builtins.hh
EXTRA_DIST = CMakeLists.txt gen_stdlib_builtins.sh.in $(stdlib_DATA)
