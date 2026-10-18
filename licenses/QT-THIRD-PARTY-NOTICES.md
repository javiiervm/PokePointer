# Qt 6.12.0 — third-party attribution and license notices

This appendix accompanies the PokePointer v1.0.0 **Windows x64** distribution.
It concerns third-party material used in Qt **Core** (`Qt6Core.dll`) and
Qt **Bluetooth** (`Qt6Bluetooth.dll`), not code owned by PokePointer.

Source of the inventory: https://doc.qt.io/qt-6/licenses-used-in-qt.html
Build-specific information may be provided by Qt's SPDX software bill of
materials (SBOM): https://doc.qt.io/qt-6/sbom.html . When an SBOM is supplied
with the build, see `licenses/sbom/` as well.

The notices below are deliberately inclusive of the Qt Core components listed
for Qt 6.12.0. Qt's published list spans several operating systems and build
configurations, so listing an entry is **not** a claim that its code is present
in the particular Windows DLL. The binary package includes **only** Qt Core and
Qt Bluetooth, not Qt GUI/WebEngine. Component versions and license choices
should be checked against the exact Qt binary build if the Qt kit changes.

## Third-party inventory (Qt Core)

| Component | Copyright / attribution | License | Official details |
| --- | --- | --- | --- |
| Apache Tika MIME definitions | Copyright 2026 The Apache Software Foundation | Apache-2.0 | https://doc.qt.io/qt-6/qtcore-attribution-tika-mimetypes.html |
| BLAKE2 | Copyright 2012 Samuel Neves | CC0-1.0 or Apache-2.0 | https://doc.qt.io/qt-6/qtcore-attribution-blake2.html |
| zlib | Copyright 1995–2026 Jean-loup Gailly and Mark Adler | Zlib | https://doc.qt.io/qt-6/qtcore-attribution-zlib.html |
| Easing equations | Copyright 2001 Robert Penner | BSD-3-Clause | https://doc.qt.io/qt-6/qtcore-attribution-easing.html |
| double-conversion | Copyright 2006–2012 V8 project authors | BSD-3-Clause | https://doc.qt.io/qt-6/qtcore-attribution-doubleconversion.html |
| MD4 | Written by Alexander Peslyak (2001), placed in public domain | Public domain | https://doc.qt.io/qt-6/qtcore-attribution-md4.html |
| MD5 | Written by Colin Plumb (1993), no copyright claimed; Ian Jackson contributed | Public domain | https://doc.qt.io/qt-6/qtcore-attribution-md5.html |
| PCRE2 | Copyright 1997–2007 University of Cambridge; 2007–2024 Philip Hazel; 2010–2024 Zoltan Herczeg | BSD-3-Clause with PCRE2 exception | https://doc.qt.io/qt-6/qtcore-attribution-pcre2.html |
| PCRE2 SLJIT | Copyright Zoltan Herczeg | BSD-2-Clause | https://doc.qt.io/qt-6/qtcore-attribution-pcre2-sljit.html |
| SHA-1 | Public-domain implementation | Public domain | https://doc.qt.io/qt-6/qtcore-attribution-sha1.html |
| SHA-3/Keccak | Keccak reference authors | CC0-1.0 | https://doc.qt.io/qt-6/qtcore-attribution-sha3-keccak.html |
| SHA-3 `brg_endian` | Copyright 1998–2013 Brian Gladman | BSD-2-Clause | https://doc.qt.io/qt-6/qtcore-attribution-sha3-endian.html |
| SHA-384/SHA-512 | Copyright 2011 IETF Trust and code authors | BSD-3-Clause | https://doc.qt.io/qt-6/qtcore-attribution-rfc6234.html |
| SipHash | Reference implementation | CC0-1.0 | https://doc.qt.io/qt-6/qtcore-attribution-siphash.html |
| TinyCBOR | Copyright 2015–2025 Intel Corporation | MIT | https://doc.qt.io/qt-6/qtcore-attribution-tinycbor.html |
| Unicode UCD | Copyright 1991–2025 Unicode, Inc. | Unicode-DFS-2016 / Unicode-3.0 as documented by Qt | https://doc.qt.io/qt-6/qtcore-attribution-unicode-character-database.html |
| Unicode CLDR | Copyright 2004–2025 Unicode, Inc. | Unicode-3.0 | https://doc.qt.io/qt-6/qtcore-attribution-unicode-cldr.html |
| `forkfd` | Copyright Intel Corporation; Klarälvdalens Datakonsult AB | MIT; used on Unix, not Windows | https://doc.qt.io/qt-6/qtcore-attribution-forkfd.html |
| `tl::expected` | Header-only expected implementation | CC0-1.0 | https://doc.qt.io/qt-6/qtcore-attribution-tlexpected.html |
| macOS event dispatcher | macOS-specific source | BSD-3-Clause; not Windows | https://doc.qt.io/qt-6/licenses-used-in-qt.html |

Qt Bluetooth also attributes BlueZ, but the Qt documentation identifies this as
a **Linux `sdpscanner` tool**, not a library linked by Qt Bluetooth. PokePointer's
Windows package does not distribute it:
https://doc.qt.io/qt-6/qtbluetooth-attribution-bluez.html

## License texts and redistribution notices

For Apache-2.0 and CC0-1.0, the complete standard license texts are in
`Apache-2.0.txt` and `CC0-1.0.txt` respectively. Other source-specific notices
are reproduced below with attribution to their authors. These notices do not
relicense PokePointer or Qt.

### zlib

Copyright (c) 1995–2026 Jean-loup Gailly and Mark Adler.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a product,
   an acknowledgment in the product documentation would be appreciated but is
   not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

### Robert Penner easing equations — BSD 3-clause

Copyright (c) 2001 Robert Penner. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the author nor the names of contributors may be used to
   endorse or promote products derived from this software without specific
   prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### double-conversion — BSD 3-clause

Copyright 2006–2012 the V8 project authors. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of Google Inc. nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### PCRE2 — BSD 3-clause with binary redistribution exception

Copyright (c) 1997–2007 University of Cambridge.
Copyright (c) 2007–2024 Philip Hazel.
Copyright (c) 2010–2024 Zoltan Herczeg.
All rights reserved. Copyright remains with other PCRE2 contributors as well.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notices,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notices,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the University of Cambridge nor the names of any
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

PCRE2 binary-package exception: the binary-redistribution notice obligation
applies to the package actually including PCRE2; it does not automatically
extend through an additional layer of packaging unless that later package
independently uses PCRE2. We nevertheless provide the notices here.
See the complete original exception at:
https://doc.qt.io/qt-6/qtcore-attribution-pcre2.html

### PCRE2 SLJIT — BSD 2-clause

Copyright Zoltan Herczeg (hzmester@freemail.hu). All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### SHA-3 `brg_endian` — BSD 2-clause variant

Copyright (c) 1998–2013 Brian Gladman, Worcester, UK. All rights reserved.

Redistribution and use of this software (with or without changes) is allowed
without the payment of fees or royalties provided that source code distributions
include the above copyright notice, these conditions and disclaimer, and binary
distributions include them in the documentation.

This software is provided 'as is' with no explicit or implied warranties in
respect of its operation, including correctness and fitness for purpose.

### SHA-384 / SHA-512 — BSD 3-clause

Copyright (c) 2011 IETF Trust and the persons identified as authors of the code.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that:

1. Source redistributions retain the copyright, conditions and disclaimer.
2. Binary redistributions reproduce them in documentation or accompanying
   materials.
3. The names of Internet Society, IETF, IETF Trust and contributors must not
   be used for endorsement without prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### TinyCBOR — MIT

Copyright (c) 2017 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

### Unicode UCD and CLDR — Unicode License v3

Copyright (c) 2004–2023 Unicode, Inc. All rights reserved.
The source inventories also note copyright years up to 2025 for Qt 6.12 data.

Permission is granted, free of charge, to anyone obtaining Unicode data files
or software and associated documentation to use, copy, modify, merge, publish,
distribute, and sell copies, and to permit others to do so, provided the copyright
and permission notice appears with the data/software or in accompanying
documentation.

THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
THIRD PARTY RIGHTS.

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE
BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES,
OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE DATA
FILES OR SOFTWARE.

The name of a copyright holder may not be used in advertising or promotion
without prior written authorization, except as allowed by the license.
Full Qt attribution and license wording:
https://doc.qt.io/qt-6/qtcore-attribution-unicode-cldr.html
https://doc.qt.io/qt-6/qtcore-attribution-unicode-character-database.html

## References and scope

- Apache-2.0 full license: `Apache-2.0.txt`.
- CC0-1.0 full license: `CC0-1.0.txt`.
- LGPLv3 and GPLv3 for Qt itself: `LGPL-3.0.txt` and `GPL-3.0.txt`.
- Source archives: https://download.qt.io/official_releases/qt/6.12/6.12.0/submodules/
- Qt Core third-party inventory: https://doc.qt.io/qt-6/licenses-used-in-qt.html
- Windows Qt 6.12 source-build instructions: https://doc.qt.io/qt-6/windows-building.html
- SBOM documentation: https://doc.qt.io/qt-6/sbom.html

The packages contain unmodified Qt shared libraries. Windows system libraries,
Linux BlueZ tools, and Qt GUI/WebEngine components are not redistributed.
