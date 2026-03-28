CC = gcc
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`
TARGET = site-blocker
SOURCES = src/main.c src/blocker.c
OBJECTS = $(SOURCES:.c=.o)
PREFIX = /usr

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	install -Dm644 site-blocker.desktop $(DESTDIR)/usr/share/applications/site-blocker.desktop
	install -Dm644 site-blocker.png $(DESTDIR)/usr/share/pixmaps/site-blocker.png
	install -Dm644 site-blocker.policy $(DESTDIR)/usr/share/polkit-1/actions/com.site-blocker.policy
	mkdir -p $(DESTDIR)/etc/site-blocker

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -f $(DESTDIR)/usr/share/applications/site-blocker.desktop
	rm -f $(DESTDIR)/usr/share/pixmaps/site-blocker.png
	rm -f $(DESTDIR)/usr/share/polkit-1/actions/com.site-blocker.policy
	rmdir $(DESTDIR)/etc/site-blocker 2>/dev/null || true

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)

.PHONY: all install uninstall clean run
