# AxelGUI

A modern, lightweight Qt6 GUI for the [Axel](https://github.com/axel-download-accelerator/axel) download accelerator, built with C++17 and Clang. It includes real-time speed monitoring, a dynamic theme engine, and native browser integration for Firefox just like IDM.

[![AUR version](https://img.shields.io/aur/version/axel-gui-git?color=fe8019&logo=arch-linux)](https://aur.archlinux.org/packages/axel-gui-git)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-orange.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Qt6](https://img.shields.io/badge/Qt-6-orange.svg)](https://www.qt.io/)

---

## UI


<p align="center">
<img src="https://raw.githubusercontent.com/TheSerphh/sampleshots/refs/heads/main/Screenshot_20260914_223458.png" alt="img" align="center" width="1000px">
</p>

## Features

- **Multi-Connection Acceleration**: Split downloads into up to 64 concurrent connections using `axel`.
- **Live Transfer Metrics**: Real-time progress bar, transfer rate, ETA, and integrated output log viewer.
- **Dynamic Theming Engine**:
  - Pre-installed popular color schemes: **Gruvbox Dark**, **Nord**, **Catppuccin Mocha**, **Tokyo Night**, and **Solarized Dark**.
  - Custom themes via JSON file import.
  - CLI safety net (`--reset-theme` / `-r`) to recover instantly from corrupted theme configs.
- **Firefox Integration**: An included WebExtension intercepts browser downloads and automatically forwards them to AxelGUI via native messaging.

---

## Installation

### Arch Linux (AUR)

Install via your preferred AUR helper:

```bash
yay -S axel-gui-git
# or
paru -S axel-gui-git
