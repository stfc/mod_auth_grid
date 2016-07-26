MODULES_DIR=$(DESTDIR)/usr/lib64/httpd/modules

APXS=/usr/sbin/apxs
INSTALL=/usr/bin/install

all: mod_auth_grid.so

mod_auth_grid.so:
	$(APXS) -c mod_auth_grid.c
	cp .libs/mod_auth_grid.so .

install:
	$(INSTALL) -d $(MODULES_DIR)
	$(INSTALL) mod_auth_grid.so $(MODULES_DIR)

clean:
	rm -rf *.o *.lo *.slo *.la *.so .libs

.PHONY: all mod_auth_grid.so install clean
