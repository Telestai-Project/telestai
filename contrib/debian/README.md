
Debian
====================
This directory contains files used to package ravend/telestai-qt
for Debian-based Linux systems. If you compile ravend/telestai-qt yourself, there are some useful files here.

## telestai: URI support ##


telestai-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install telestai-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your telestai-qt binary to `/usr/bin`
and the `../../share/pixmaps/raven128.png` to `/usr/share/pixmaps`

telestai-qt.protocol (KDE)

