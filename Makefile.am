
bin_PROGRAMS = fyfsp

fyfsp_LDADD = $(x11_LIBS)

if CACHING
fyfsp_SOURCES = fyfsp.c window.c xquery.c \
				window.h contrib/uthash.h global-types.h xquery.h cache.h cache.c
debug_conf_opt=
else
fyfsp_SOURCES = fyfsp.c window.c xquery.c \
				window.h contrib/uthash.h global-types.h xquery.h
debug_conf_opt=--without-tree-caching
endif

if MOUSE_TRACKING
debug_conf_opt+=--with-mouse-tracking
endif

man1_MANS = fyfsp.1
EXTRA_DIST = map-20-clocks.pl render-clock.pl README.pod fyfsp.pod upstart.conf \
			 fyfsp.1 contrib/keystats.pl colorize-debugs ubuntu-system-install.sh

if UPSTART
upstartdir = $(UPSTARTDIR)
upstart_DATA = fyfsp.conf
endif

README: README.pod
	podchecker README.pod
	pod2text README.pod > README

.pod.1:
	pod2man --release $(distdir) -c "Annoyed User Commands" --utf8 $< $@

.pod.html:
	pod2html $< > $@

edit = sed \
	-e 's|[@]bindir[@]|$(bindir)|g'

fyfsp.conf: upstart.conf
	$(edit) $(srcdir)/upstart.conf >$@

clean-local:
	-chmod u+w -R fyfsp-*.* 2>/dev/null; rm -rf fyfsp-*.*
	-[ -f tmp-dir ] && rm -rvf $$( cat tmp-dir || echo /tmp/nononono )
	-rm -f fyfsp.conf *.bc *.ll README tmp-dir $(man1_MANS) fyfsp-debug keyfile

tmp-dir:
	mktemp -d > tmp-dir

test-install: tmp-dir
	+ make --no-print-directory prefix=$$(< tmp-dir) install
	@ echo; ls --color=always -Fhal $$(find $$(< tmp-dir) -type f)

test-dist:
	+ CONFIG_SITE= make --no-print-directory distcheck

list-dist: dist-xz
	echo;echo "+---------------"; \
		for i in *.xz; do echo "| $$i"; echo "+-------"; \
		tar -Jtvvf $$i; echo; done

sc superclean: clean
	git clean -dfx

mylog=./fyfsp.log
fyfsp-log: $(fyfsp_SOURCES) Makefile config.h
	grep define.DEBUG_LOG_TO_FILE config.h || ./configure $(debug_conf_opt) --with-debug-log=$(mylog) DEBUG_ALL=1
	+ make --no-print-directory fyfsp
	mv fyfsp $@

runlog logrun: fyfsp-log
	- killall fyfsp-log
	./$< &
	tail -n 0 -f $(mylog) | ./colorize-debugs

fyfsp-debug: $(fyfsp_SOURCES) Makefile config.h
	grep define.DEBUG_LOG_TO_FILE config.h && ./configure $(debug_conf_opt) DEBUG=1 || /bin/true
	grep define.DEBUG config.h || ./configure $(debug_conf_opt) DEBUG=1
	+ make --no-print-directory fyfsp
	mv fyfsp $@

debug run: fyfsp-debug
	gdb --batch --eval-command run --eval-command bt ./$< 2>&1 | ./colorize-debugs
