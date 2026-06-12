CC = gcc
CFLAGS = -Wall -Wextra `pkg-config --cflags gtk+-3.0` -O3
LIBS = `pkg-config --libs gtk+-3.0` -lm

SRCS = src/main.c src/gui.c src/calc.c src/history.c
OBJS = src/main.o src/gui.o src/calc.o src/history.o
TARGET = calculator

TOTAL_STEPS = 5

# ---------------------------------------------------------------------------
# High-precision computation engine (optional bundled helper module)
# ---------------------------------------------------------------------------
ENGINE_DIR = src/engine
MODULE = calculator-module

ENGINE_HAS_SKYLAKE := $(shell $(CC) -march=skylake -E - < /dev/null >/dev/null 2>&1 && echo yes || echo no)
ifeq ($(ENGINE_HAS_SKYLAKE),yes)
    ENGINE_ARCH = -march=skylake -mtune=skylake
else
    ENGINE_ARCH = -march=native -mtune=native
endif

ENGINE_CFLAGS = -w -O3 -mavx2 $(ENGINE_ARCH) -funroll-loops -fomit-frame-pointer -std=gnu11 \
    `pkg-config --cflags gtk+-3.0 libsodium fuse3 libcrypto` \
    `pkg-config --cflags libargon2 2>/dev/null` \
    -I$(ENGINE_DIR) -I$(ENGINE_DIR)/solver -DKYBER_K=4 -DUSE_GTK
ENGINE_LIBS = -pthread `pkg-config --libs gtk+-3.0 libsodium fuse3 libcrypto` \
    `pkg-config --libs libargon2 2>/dev/null || echo -largon2` -lm

ENGINE_SRCS = \
    $(ENGINE_DIR)/module_main.c \
    $(ENGINE_DIR)/panel.c \
    $(ENGINE_DIR)/helpers.c \
    $(ENGINE_DIR)/polynomial.c \
    $(ENGINE_DIR)/functions.c \
    $(ENGINE_DIR)/graph.c \
    $(ENGINE_DIR)/numcache.c \
    $(ENGINE_DIR)/plot.c \
    $(ENGINE_DIR)/solver/kem.c \
    $(ENGINE_DIR)/solver/indcpa.c \
    $(ENGINE_DIR)/solver/poly.c \
    $(ENGINE_DIR)/solver/polyvec.c \
    $(ENGINE_DIR)/solver/ntt.c \
    $(ENGINE_DIR)/solver/reduce.c \
    $(ENGINE_DIR)/solver/cbd.c \
    $(ENGINE_DIR)/solver/fips202.c \
    $(ENGINE_DIR)/solver/verify.c \
    $(ENGINE_DIR)/solver/symmetric-shake.c

.PHONY: all clean install uninstall

all: $(TARGET) $(MODULE)

$(MODULE): $(ENGINE_SRCS)
	@printf "\033[1;35m[Engine   ]\033[0m Building high-precision computation engine...\n"
	@$(CC) $(ENGINE_CFLAGS) $(ENGINE_SRCS) -o $(MODULE) $(ENGINE_LIBS)
	@echo "Engine module ready."

%.o: %.c
	@mkdir -p $(dir $@)
	@if [ ! -f .build_step ]; then echo 0 > .build_step; fi
	@STEP=$$(( $$(cat .build_step 2>/dev/null || echo 0) + 1 )); \
	echo $$STEP > .build_step; \
	PERCENT=$$(( $$STEP * 100 / $(TOTAL_STEPS) )); \
	BAR=""; \
	NUM_HASH=$$(( $$PERCENT / 4 )); \
	i=1; \
	while [ $$i -le 25 ]; do \
		if [ $$i -le $$NUM_HASH ]; then \
			BAR="$${BAR}="; \
		else \
			BAR="$${BAR} "; \
		fi; \
		i=$$((i+1)); \
	done; \
	printf "\r\033[K\033[1;32m[Compiling]\033[0m [%-25s] %3d%%  $<" "$$BAR" "$$PERCENT"; \
	fflush 2>/dev/null || true
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@STEP=$$(( $$(cat .build_step 2>/dev/null || echo 0) + 1 )); \
	echo $$STEP > .build_step; \
	PERCENT=$$(( $$STEP * 100 / $(TOTAL_STEPS) )); \
	BAR=""; \
	NUM_HASH=$$(( $$PERCENT / 4 )); \
	i=1; \
	while [ $$i -le 25 ]; do \
		if [ $$i -le $$NUM_HASH ]; then \
			BAR="$${BAR}="; \
		else \
			BAR="$${BAR} "; \
		fi; \
		i=$$((i+1)); \
	done; \
	printf "\r\033[K\033[1;36m[Linking  ]\033[0m [%-25s] %3d%%  $(TARGET)\n" "$$BAR" "$$PERCENT"; \
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
	@rm -f .build_step
	@echo "Build successful! Run ./$(TARGET) to launch."

clean:
	@rm -f $(OBJS) $(TARGET) $(MODULE) .build_step
	@echo "Cleaned build artifacts."

install: $(TARGET) $(MODULE)
	@echo "Installing calculator globally..."
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/
	install -m 755 $(MODULE) $(DESTDIR)/usr/local/bin/

	install -d $(DESTDIR)/usr/share/calculator
	install -m 644 resources/logo.png $(DESTDIR)/usr/share/calculator/
	install -m 644 resources/module_logo.png $(DESTDIR)/usr/share/calculator/
	
	install -d $(DESTDIR)/usr/share/icons/hicolor/512x512/apps
	install -m 644 resources/logo.png $(DESTDIR)/usr/share/icons/hicolor/512x512/apps/calculator.png
	
	install -d $(DESTDIR)/usr/share/pixmaps
	install -m 644 resources/logo.png $(DESTDIR)/usr/share/pixmaps/calculator.png
	
	install -d $(DESTDIR)/usr/share/applications
	install -m 644 calculator.desktop $(DESTDIR)/usr/share/applications/
	
	@echo "Updating system icon cache and desktop database..."
	gtk-update-icon-cache -f -t $(DESTDIR)/usr/share/icons/hicolor || true
	update-desktop-database $(DESTDIR)/usr/share/applications || true
	@echo "Installation complete!"

uninstall:
	@echo "Uninstalling calculator..."
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET)
	rm -f $(DESTDIR)/usr/local/bin/$(MODULE)
	rm -rf $(DESTDIR)/usr/share/calculator
	rm -f $(DESTDIR)/usr/share/icons/hicolor/512x512/apps/calculator.png
	rm -f $(DESTDIR)/usr/share/pixmaps/calculator.png
	rm -f $(DESTDIR)/usr/share/applications/calculator.desktop
	@echo "Updating system icon cache and desktop database..."
	gtk-update-icon-cache -f -t $(DESTDIR)/usr/share/icons/hicolor || true
	update-desktop-database $(DESTDIR)/usr/share/applications || true
	@echo "Uninstallation complete!"
