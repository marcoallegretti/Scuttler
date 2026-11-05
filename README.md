# Scuttler

Scuttler is a minimal Qt WebEngine shell for the web user interface of the
[Agama](https://github.com/agama-project/agama) installer. It plays the role
for Agama that [Slitherer](https://gitlab.com/VelocityLimitless/Projects/slitherer)
plays for Anaconda, and is based on the Qt WebView minibrowser example.

## Behavior

```
scuttler [--fullscreen | --windowed] [url]
```

Without a URL argument, Scuttler opens the Agama web UI on `http://localhost/`.
Agama treats only `localhost` and `127.0.0.0/8` as local connections.

- **Login:** Scuttler logs in with the token from the systemd credential
  `agama-token` (`$CREDENTIALS_DIRECTORY/agama-token`), when one is provided.
  Otherwise it shows Agama's login page.
- **Language:** it passes the language from the UEFI `PlatformLang` variable, as
  the Agama live image does for its browser.

The window covers the whole screen unless `--windowed` is given.

The page is reloaded after its web content process terminates, and after
connection errors, so Scuttler can start before the Agama web server listens.
Cookies and the HTTP cache are kept in memory only. Downloads and new windows
are not supported.

A URL given on the command line is visible to other local users in the process
list, so do not pass the token that way.

Qt WebEngine's Chromium sandbox does not work as root; run Scuttler as an
unprivileged user.

## Installer session

`session/` contains a systemd service that runs Scuttler as the display manager
of an Agama live image. It starts KWin on the first virtual terminal as the
unprivileged `scuttler` user, through a logind session, and passes the Agama
token as the `agama-token` credential. KWin follows the keyboard layout that
Agama sets through localed. The service is bound to `agama-web-server.service`,
so a new web server, and with it a new token, starts a new session. The
session has no D-Bus session bus, and with it no accessibility bus.

Scuttler runs on Xwayland in this session, because Qt WebEngine closes form
drop-downs at once on Wayland
([QTBUG-147154](https://bugreports.qt.io/browse/QTBUG-147154)). KWin scales it
on high-density displays.

Enable it with `systemctl enable --force scuttler.service`, which replaces
another display manager. The `scuttler-kiosk` package in
`packaging/scuttler.spec` installs it.

## Building

Requirements: CMake 3.21, a C++17 compiler, and Qt 6.11 or newer with Qt
WebEngine and Qt Quick Controls. The tests also need Qt Test and `qmllint`;
configure with `-DBUILD_TESTING=OFF` to skip them.

```
cmake -B build
cmake --build build
ctest --test-dir build
cmake --install build
```

With `-DSCUTTLER_WAYLAND_TESTS=ON`, the tests also run Scuttler in a headless
[cage](https://github.com/cage-kiosk/cage) session, which needs cage and
Python 3.

## License

Scuttler is licensed under the [EUPL-1.2](LICENSES/EUPL-1.2.txt). `main.cpp` and
`main.qml` are derived from the Qt WebView minibrowser example and also remain
under the [BSD-3-Clause](LICENSES/BSD-3-Clause.txt) license.
