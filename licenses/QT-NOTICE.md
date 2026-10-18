# Qt and third-party notices (Windows release)

## PokePointer

PokePointer is Copyright (c) 2026 Javier Villanueva Martínez, licensed under the
MIT License (see `../LICENSE`). This license covers PokePointer's own code, **not**
the Qt libraries bundled with the Windows binary.

## Qt 6.12.0

This distribution contains the following **separately licensed** Qt shared
libraries from the open-source Qt 6.12.0 Windows MSVC x64 build:

- `bin/Qt6Core.dll` — Qt Core
- `bin/Qt6Bluetooth.dll` — Qt Bluetooth

Qt is Copyright (C) The Qt Company Ltd. and other Qt contributors.
PokePointer is not affiliated with, sponsored by, or endorsed by The Qt Company.

These Qt modules are distributed under the GNU Lesser General Public License,
version 3 (LGPLv3), as permitted for the Qt open-source distribution. The full
license texts are included with this package:

- `LGPL-3.0.txt` — GNU Lesser General Public License, version 3
- `GPL-3.0.txt` — GNU General Public License, version 3, whose terms are
  incorporated into LGPLv3

The Qt shared libraries are dynamically linked, stored next to the executable,
and are not modified by PokePointer. You may replace them with compatible,
user-modified Qt libraries. PokePointer performs no DRM or signature checks on
these DLLs. Compatibility of modified replacements is the user's responsibility.

## Qt source code and build information

The corresponding **Qt 6.12.0** sources can be obtained from the official Qt
source-package downloads. The release is built with the `msvc2022_64` Qt kit;
`qtbase` provides Qt Core and `qtconnectivity` provides Qt Bluetooth.

- Qt 6.12.0 source packages: https://download.qt.io/official_releases/qt/6.12/6.12.0/submodules/
- Qt source repositories: https://code.qt.io/
- Qt 6.12 Windows build documentation: https://doc.qt.io/qt-6/windows-building.html
- Qt licensing overview: https://doc.qt.io/qt-6/licensing.html

If the distribution ever switches to a different Qt build/version, update these
notices and ensure source access remains available for the corresponding build,
including any patches applied to Qt by the distributor. No patches to Qt are
applied by the PokePointer workflow as currently configured.

## Third-party attributions included with Qt

This package includes `QT-THIRD-PARTY-NOTICES.md`, covering the Qt 6.12 Qt Core
third-party inventory, source-specific copyright notices and applicable
redistribution terms, with the complete Apache-2.0 and CC0-1.0 texts. When the
installed Qt kit supplies SPDX SBOM files for `qtbase` and `qtconnectivity`,
CMake copies those to `licenses/sbom/` for build-specific provenance.

Official component and license inventory:
https://doc.qt.io/qt-6/licenses-used-in-qt.html

Qt Bluetooth's Linux-only BlueZ utility is not shipped in this Windows package.
Check the Qt-provided source and SBOM when upgrading the Qt version or modules.

## Independent software

Windows operating-system libraries are provided by Microsoft and are not
included in this distribution. Microsoft Visual C++ runtime components are not
bundled by PokePointer; some users may need the official x64 redistributable.
