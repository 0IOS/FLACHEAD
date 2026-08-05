# Boot integration (Raspberry Pi OS Lite)

FLACHEAD runs as a systemd service on the framebuffer console — no desktop
required. It boots straight into the music UI: on first launch it creates
`~/.flachead/flachead.db`, migrates the schema, and scans the music folder in
the background (default root `$HOME/Music`, changeable in the on-screen
Settings). The app already exits gracefully on `SIGTERM` (systemd's stop
signal), flushes its logs, and is CPU/RAM-capped so it cannot starve the
single-core Pi.

## Install

```sh
sudo mkdir -p /opt/flachead
sudo cp build-pi/FLACHEAD /opt/flachead/
sudo cp packaging/flachead.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now flachead
```

## The unit file (`packaging/flachead.service`)

| Setting | Value | Why |
|---|---|---|
| `User=pi` | runs unprivileged | GPIO sysfs and DRM need `pi` in the right groups |
| `SDL_VIDEODRIVER=kmsdrm` | DRM/KMS output | no X server on Lite |
| `CPUQuota=80%` | leaves 20% for system + audio | audio must never starve |
| `MemoryMax=128M` | hard RAM cap | under the 120 MB target budget |
| `Restart=on-failure` | self-heals | a crashed UI comes back |

Check status:

```sh
systemctl status flachead
journalctl -u flachead -f
```

## Console log-in (optional)

For debugging on the physical console, enable autologin:

```sh
sudo raspi-config   # System Options → Boot → Console Autologin
```

## GPIO buttons

Add `pi` to the `gpio` group and use the GPIO input backend when a panel
replaces the window:

```sh
# in flachead.service
ExecStart=/opt/flachead/FLACHEAD --input=gpio
```

See `src/input/GpioInputBackend.hpp` for the default wiring and the SPI
display notes in `docs/PI_RUNBOOK.md`.
