# Windows launcher icon repair

The Windows installer shortcuts target `OpenKJ-Remote.exe`, which configures
the isolated runtime before starting `openkj.exe`. The launcher compilation
omitted Windows resources, so shortcuts and its Explorer entry used a generic
application icon even though the main executable retained the OpenKJ icon.

Both source build workflows now compile the existing `src/appicon.rc` and link
that resource into the launcher. No artwork or playback code changes. Installer
shortcuts explicitly select the existing main program icon, and the uninstall
entry uses that same icon. Shortcut targets and launch behavior are unchanged.

The Windows verification compares every one of the original icon's ten image
sizes (16 through 256 pixels) byte for byte with the resources embedded in both
executables. Its JSON evidence accompanies the existing maintenance checks.

For an immediate workaround on the currently published installer, right-click
the desktop shortcut, select Properties, Shortcut, Change Icon, then Browse to
`openkj.exe` in the installed OpenKJ directory. Select its OpenKJ icon and apply.
A pinned taskbar shortcut may need to be unpinned and pinned again.
