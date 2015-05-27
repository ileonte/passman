# BUILD INSTRUCTIONS

* `BASEDIR` is the directory containing this README
* `QMAKE4` is the path to the qmake binary from your Qt4 installation
* `QMAKE5` is the path to the qmake binary from your Qt5 installation

## Wallet backend library
The first thing you need to do is build the backend wallet library.
There is currently only one supported backend: kwallet4. To build it
run:

```
cd $BASEDIR/wallets
$QMAKE4 CONFIG+=kwallet4
make
```

## Main application
Next you will need to build the main application:

```
cd $BASEDIR
$QMAKE5
make
```

You can now run `./passman`

# Using
The first thing you need to do is select a storage backend and a wallet.
To do so open the settings window by clicking on the tray icon and select
the storage backend and wallet from the dropdowns in the **Wallet settings**
section.

By default the application does not install any global keyboard shortcuts.
You will need to set these yourself in the **Hotkey settings** section of
the settings window. The following hotkeys are recommended for a typical
KDE desktop setup:

* `ALT+F1` - global hotkey for searching
* `CTRL+ALT+S` - global hotkey for showing/hiding the settings window
* `CRTL+ALT+Q` - global hotkey for quitting the application
* `CTRL+ALT+M` - global hotkey for showing/hiding the password manager

Please note that other than the *global hotkey for searching* all other hotkeys
are optional as their functionality can be triggered by using the buttons from
the bottom of the settings window or by using the tray icon.