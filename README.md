Session
=======

[![License](https://img.shields.io/badge/license-GPLv3.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![GitHub release](https://img.shields.io/github/release/lirios/session.svg)](https://github.com/lirios/session)
[![Build Status](https://travis-ci.org/lirios/session.svg?branch=develop)](https://travis-ci.org/lirios/session)
[![GitHub issues](https://img.shields.io/github/issues/lirios/session.svg)](https://github.com/lirios/session/issues)

The session is responsible for bootstrapping the initial environment and
launching critical desktop services, including the shell, as well as
XDG autostart applications.

## Dependencies

Qt >= 5.10.0 with at least the following modules is required:

 * [qtbase](http://code.qt.io/cgit/qt/qtbase.git)

The following modules and their dependencies are required:

 * [cmake](https://gitlab.kitware.com/cmake/cmake) >= 3.10.0
 * [cmake-shared](https://github.com/lirios/cmake-shared.git) >= 1.0.0
 * [qtgsettings](https://github.com/lirios/qtgsettings) >= 1.1.0
 * [libliri](https://github.com/lirios/libliri)

## Installation

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
make
make install # use sudo if necessary
```

Replace `/path/to/prefix` to your installation prefix.
Default is `/usr/local`.

You can also append the following options to the `cmake` command:

 * `-DLIRI_SESSION_DEVELOPMENT_BUILD:BOOL=ON`: Remove restrictions that gets in your
   way during development, such as ptrace being forbidden.
 * `-DLIRI_ENABLE_SYSTEMD:BOOL=OFF`: Disable systemd support.
 * `-DINSTALL_SYSTEMDUSERUNITDIR=/path/to/systemd/user`: Path to install systemd user units (default: `/usr/local/lib/systemd/user`).
 * `-DINSTALL_SYSTEMDUSERGENERATORSDIR=/path/to/systemd/user-generators`: Path to install systemd user generators (default: `/usr/local/lib/systemd/user-generators`).

## Licensing

Licensed under the terms of the GNU General Public License version 3 or,
at your option, any later version.

## Notes

### Logging categories

Qt 5.2 introduced logging categories and Liri Shell takes advantage of
them to make debugging easier.

Please refer to the [Qt](http://doc.qt.io/qt-5/qloggingcategory.html) documentation
to learn how to enable them.

### Available categories

 * Compositor:
   * **liri.session:** Session manager.

## Components

*liri-session*

Brings up the D-Bus session if $DBUS_SESSION_BUS_ADDRESS is not set,
starts the shell, runs autostart programs and launches apps.

It's extended by the following session modules:

 * **autostart:** Runs autostart programs.
 * **locale:** Sets locale environment variables based on settings.
 * **shell:** Starts the shell and waits for io.liri.Shell to be available.

You can disable some session modules, for example if you don't want to
set locale and run the autostart programs:

```sh
liri-session --disable-modules=autostart,locale
```

## Running on another window system

The platform plugin to use is automatically detected based on the environment,
it can be one of the following:

 * wayland: Run inside another Wayland compositor
 * xcb: Run inside a X11 session
 * liri: Uses DRM/KMS or a device specific EGL integration

You can override the automatic detection, if it doesn't work as intended.

### X11

To run windowed inside a X11 session:

```sh
liri-session -- -platform xcb
```

### Wayland

To run windowed inside a Wayland session:

```sh
liri-session -- -platform wayland
```

Some compositors, such as Weston, support the fullscreen-shell protocol that
allows a compositor to be nested into another compositor.

Let's take Weston as an example. First you need to run it with the fullscreen-shell
protocol enabled:

```sh
weston --shell=fullscreen-shell.so
```

Then you need to run `liri-session` like this:

```sh
QT_WAYLAND_SHELL_INTEGRATION=fullscreen-shell liri-session -- -platform wayland
```

### DRM/KMS

To make sure the Liri session runs without another window system type:

```sh
liri-session -- -platform liri
```
