# Code changes log (for commit messages)

This file records code changes suitable for use as commit message summaries.

---

## 2026-01-30: Make DirectMusic optional (build without legacy DirectX SDK)

**Summary:** Add `USE_DirectMusic` config and guard all DirectMusic code so OpenJoey builds with Visual Studio 2022 when the June 2010 DirectX SDK is not installed. MIDI falls back to MCI when DirectMusic is disabled.

### Suggested commit message (short)
```
Make DirectMusic optional; build without legacy DirectX SDK (dmusici.h)
```

### Suggested commit message (detailed)
```
Make DirectMusic optional; build without legacy DirectX SDK

- Add USE_DirectMusic in yaneSDK/config/yaneConfig.h (default 0).
- Guard dmusicc.h/dmusici.h in stdafx.h and yaneDirectMusic.h.
- Stub CDirectMusic when USE_DirectMusic 0 (CanUseDirectMusic() false).
- Guard yaneDirectMusic.cpp, yaneMIDIOutputDM.cpp, and SoundFactory
  DirectMusic paths with #if USE_DirectMusic.
- Multimedia/index.h and SoundFactory include DM headers only when enabled.
- MIDI uses MCI when DirectMusic is disabled. Set USE_DirectMusic 1 and
  add June 2010 DirectX SDK include/lib paths to restore DirectMusic.
```

### Files changed
| File | Change |
|------|--------|
| `yaneSDK/config/yaneConfig.h` | Add `USE_DirectMusic` (default 0) and comment. |
| `yaneSDK/stdafx.h` | Wrap `#include <dmusicc.h>` and `<dmusici.h>` in `#if USE_DirectMusic`. |
| `yaneSDK/Multimedia/yaneDirectMusic.h` | Include dmusic headers and full class when `USE_DirectMusic`; else stub `CDirectMusic`. |
| `yaneSDK/Multimedia/yaneDirectMusic.cpp` | Wrap entire implementation in `#if USE_DirectMusic`. |
| `yaneSDK/Multimedia/yaneMIDIOutputDM.cpp` | Wrap entire file in `#if USE_DirectMusic`. |
| `yaneSDK/Multimedia/index.h` | Include `yaneMIDIOutputDM.h` only when `USE_DirectMusic`. |
| `yaneSDK/Multimedia/yaneSoundFactory.cpp` | Guard case 2 (DM), InnerDeleteChainForDM, IncRef/DecRef/GetDirectMusic; add stub implementations when `USE_DirectMusic` 0. Include `yaneMIDIOutputDM.h` only when `USE_DirectMusic`. |

### How to enable DirectMusic again
1. In `yaneSDK/config/yaneConfig.h`, set `#define USE_DirectMusic 1` (or build with `USE_DirectMusic=1`).
2. Install the June 2010 DirectX SDK.
3. Add to the project: Include path `...\Microsoft DirectX SDK (June 2010)\Include`, Library path `...\Lib\x86`.

---

---

## 2026-01-30: Fix delegate.h for C++11+ (rename static_assert template)

**Summary:** Rename the custom `static_assert` template in YTL delegate.h to `delegate_static_assert` so it does not conflict with the C++11 `static_assert` keyword. Fixes MSVC 2022 errors: "unnamed class template", "missing ';' before 'static_assert'", "'struct': missing tag name".

### Suggested commit message (short)
```
Fix delegate.h: rename static_assert template for C++11 compatibility
```

### Suggested commit message (detailed)
```
Fix delegate.h for C++11+ (MSVC 2022)

- Rename custom template static_assert to delegate_static_assert in
  yaneSDK/YTL/delegate.h to avoid conflict with C++11 keyword.
- Fixes: unnamed class template, missing ';' before 'static_assert',
  'struct': missing tag name when building with VS 2022.
```

### Files changed
| File | Change |
|------|--------|
| `yaneSDK/YTL/delegate.h` | Rename template `static_assert` → `delegate_static_assert` (definition and all 5 usages). |

---

## 2026-01-30: Fix xstring debug assertion (string iterator / GetParentDir / rich text)

**Summary:** Fix "cannot dereference string iterator because it is out of range" (MSVC debug assertion in xstring). Guard all string iterator and pointer uses in file/path and rich-text code so empty strings and null pointers are never dereferenced.

### Suggested commit message (short)
```
Fix xstring debug assertion: guard string iterators and null ptrs (GetParentDir, rich text, card text)
```

### Suggested commit message (detailed)
```
Fix xstring debug assertion (string iterator / GetParentDir / rich text)

- yaneSDK/Auxiliary/yaneFile.cpp:
  - SetCurrentDir: avoid end()-1 on empty m_strCurrentDirectory; handle empty path before using iterator.
  - GetParentDir: require filename.size()>=2 for UNC branch; check it!=filename.end() before *it in loops; check it!=filename.begin() before *(it-1); add it++ in UNC for-loop.
  - GetPureFileNameOf: loop while (it != purefilename.end()) to avoid dereferencing end().
- src/system/backport/yaneTextFastPlaneEx.cpp: return 0 for empty tagContent before tagContent[0]; add m_text.empty() check at start of GetNextSegment.
- src/test/CSceneCardList.cpp: guard card name for formattedText (const char* name = card.cardData->name.name ? ... : "") to avoid std::string(nullptr).
```

### Files changed
| File | Change |
|------|--------|
| `yaneSDK/Auxiliary/yaneFile.cpp` | SetCurrentDir empty-path handling; GetParentDir UNC and main-loop bounds checks; GetPureFileNameOf loop bound. |
| `src/system/backport/yaneTextFastPlaneEx.cpp` | Empty tagContent and m_text checks in parser. |
| `src/test/CSceneCardList.cpp` | Null-safe name for card preview formattedText. |

---

## 2026-01-30: Fix build (solution dependency + explicit lib; remove ProjectReference)

**Summary:** Resolve "referenced project does not exist", "GetTargetPathDependsOn already consumed", and "cannot open input file ... yaneSDK.lib". Use solution-level build dependency and explicit linker settings instead of ProjectReference.

### Suggested commit message (short)
```
Fix build: solution dependency + explicit yaneSDK lib path; remove ProjectReference
```

### Suggested commit message (detailed)
```
Fix build: solution dependency + explicit lib; remove ProjectReference

- yaneSDK.sln: add GlobalSection(ProjectDependencies) so YaneSkeleton builds after yaneSDK (Debug|x86, Release|x86).
- src/YaneSkeleton.vcxproj: remove ProjectReference and CustomBuild items; add for Debug AdditionalLibraryDirectories ..\yaneSDK\ProjectFiles\MSVC7.1\lib and AdditionalDependencies yaneSDK_d.lib; for Release same AdditionalLibraryDirectories and yaneSDK.lib.
- Fixes: referenced project {ADE7FA7E-...} does not exist, GetTargetPathDependsOn already consumed, cannot open input file ... yaneSDK.lib.
```

### Files changed
| File | Change |
|------|--------|
| `yaneSDK.sln` | Add ProjectDependencies so YaneSkeleton depends on yaneSDK. |
| `src/YaneSkeleton.vcxproj` | Remove ProjectReference and CustomBuild; add Link AdditionalLibraryDirectories and AdditionalDependencies per config. |

---

## GetTargetPathDependsOn (MSBuild warning)

**Symptom:** "The property 'GetTargetPathDependsOn' is being set to a value for the first time, but it was already consumed at Microsoft.Common.CurrentVersion.targets (2243,5)."

**Cause:** Something (often a NuGet or custom target) sets or appends to `GetTargetPathDependsOn` after the main build has already read it.

**Workaround (superseded for this repo):** We avoid ProjectReference to the static lib and use solution dependency + explicit linker settings instead (see "2026-01-30: Fix build" above). If you need to set the property early, add in the project directory `Directory.Build.props` with:

```xml
<PropertyGroup>
  <GetTargetPathDependsOn>$(GetTargetPathDependsOn)</GetTargetPathDependsOn>
</PropertyGroup>
```

---

*Append new entries below with date and short summary.*
