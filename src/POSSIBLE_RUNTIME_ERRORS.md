# Possible Runtime Errors in OpenJoey/src

Identified from a static scan of `OpenJoey/src`. Ordered by severity/likelihood.

---

## 1. **CRITICAL: Dangling pointers in CSceneSettings** (same pattern as CSceneSplash)

**File:** `test/CSceneSettings.cpp` (lines 36–45), `test/CSceneSettings.h`

**Issue:** `m_settingsBackdrop`, `m_settingsWindowBtn`, `m_settingsFullscreenBtn`, `m_settingsBitBtn` are `ISurface*` assigned from `m_vPlaneLoader.GetPlane(n)`. `GetPlane()` returns `CPlane` by value; the temporary is destroyed at the end of the statement, so the stored raw pointers are dangling. Any later use (e.g. `m_settingsBitBtn->GetSurfaceInfo()->GetSize()` at line 283) can crash (0xDDDDDDDD or access violation).

**Fix:** Store `CPlane` (or `smart_ptr<ISurface>`) instead of `ISurface*`, and use `.get()` only when the owning object is still in scope. Same approach as in `CSceneSplash` and the GUI button cache fix.

---

## 2. **CRITICAL: Null `card.cardData` in CSceneCardList**

**File:** `test/CSceneCardList.cpp` (lines 778–793)

**Issue:** Inside the card hover/click block, `card.cardData` is used without a null check (`card.cardData->properties.GetMonsterTypeTextId()`, `card.cardData->description`, `card.cardData->name.name`, `card.cardData->properties.GetMonsterType()`). `cardData` is set from `m_bin->GetCard(intId)` which can return `NULL` (see `CBinSystem.h`). If the card ID is invalid or data is missing, this dereferences null.

**Fix:** Guard the block with `if (card.cardData)` before using `card.cardData->...`.

---

## 3. **GetConstSurfaceInfo() / GetSurfaceInfo() used without null check**

**Issue:** Several call sites use `->GetConstSurfaceInfo()->...` or `->GetSurfaceInfo()->...` without checking the result of `GetSurfaceInfo()`/`GetConstSurfaceInfo()`. If it ever returns null, the next `->` crashes.

**Locations:**

- `system/backport/yaneGUIButton.cpp`  
  - 64, 70: `lp->GetConstSurfaceInfo()->GetPixel(...)` — `lp` is checked, info is not.  
  - 261: `sourcePlane->GetConstSurfaceInfo()->GetSize()` — same.
- `test/CSceneCardList.cpp`  
  - 772–773: `m_cardHoverBorder->GetConstSurfaceInfo()->GetSize()` — no null check on `GetConstSurfaceInfo()`.
- `test/CSceneSettings.cpp`  
  - 283: `m_settingsBitBtn->GetSurfaceInfo()->GetSize()` — also affected by (1); after fixing (1), add a null check for `GetSurfaceInfo()`.

**Fix:** After each `GetConstSurfaceInfo()` or `GetSurfaceInfo()`, check for null before calling `->GetSize()`, `->GetPixel()`, etc.

---

## 4. **Null pointer dereference in capp.cpp (PLANE TEST)**

**File:** `capp.cpp` (lines 72–76)

**Issue:** `CPlane bgplane;` and `CPlane charaplane;` default-construct; default `CPlane` has an empty `smart_ptr<ISurface>` so `get()` is null. Then `bgplane->Load(...)` and `charaplane->Load(...)` call through a null pointer. If this code path runs (e.g. before scene system takes over), it will crash.

**Fix:** Either create the plane from a factory/loader so it holds a valid surface before calling `Load`, or check `if (bgplane.get())` before `bgplane->Load(...)`, or remove/guard this test block so it does not run with default-constructed planes.

---

## 5. **CSceneYesNo – possible null m_vBackground**

**File:** `test/CSceneYesNo.cpp` (line 281)

**Issue:** `m_vBackground->GetSurfaceInfo()` is used. If `m_vBackground` is null (e.g. init failed or path not run), this crashes.

**Fix:** Use `m_vBackground.get()` and check for null before calling `GetSurfaceInfo()`.

---

## 6. **Buffer / string bounds (lower priority)** ✅ FIXED

**File:** `test/CSceneCardList.cpp` (e.g. 182, 466, 515), `test/CSceneSettings.cpp` (214), etc.

**Issue:** Fixed-size buffers (`char fullCardPath[256]`, `char filepath[256]`, etc.) used with `sprintf` or string operations. If paths or formatted output exceed the buffer size, buffer overrun can occur.

**Fix:** Use `snprintf` with size limit, or `std::string`/bounded helpers, and validate path length.

**Applied:** Replaced `sprintf` with `sprintf_s(buffer, sizeof(buffer), fmt, ...)` in CSceneCardList, CSceneSettings, CSceneMainMenu, and CSceneYesNo for all fixed-size buffers.

---

## 7. **smart_ptr with raw address (e.g. SetMouse)** ✅ DOCUMENTED

**File:** e.g. `test/CSceneMainMenu.cpp` (74), `test/CSceneYesNo.cpp` (129, 158)

**Issue:** `SetMouse(smart_ptr<CFixMouse>(&m_mouse, false))` — the second parameter `false` means “do not take ownership”. If the smart_ptr ever tries to delete the pointer, that would be undefined behavior. As long as these objects are never owned (stack/local) and the smart_ptr is only used for the duration of the object’s life, this is safe but fragile.

**Recommendation:** Ensure these mouse objects outlive all GUI parts that hold the smart_ptr; document non-ownership.

**Applied:** Added comment in `yaneGUIParts.h` for SetMouse that non-owning usage requires the pointed-to object to outlive the control. Added “Non-owning: m_mouse outlives scene” at each SetMouse(&m_mouse, false) call in CSceneMainMenu, CSceneSettings, CSceneYesNo, and CSceneCardList.

---

## Summary of recommended fixes (in order)

1. **CSceneSettings:** ✅ FIXED — backdrop/window/fullscreen/bit/slider stored as `CPlane`; `.get()` and null checks added.
2. **CSceneCardList:** ✅ FIXED — `if (card.cardData)` guard and `GetConstSurfaceInfo()` null check for hover border.
3. **GetConstSurfaceInfo / GetSurfaceInfo:** ✅ FIXED — null checks in `yaneGUIButton.cpp` (IsButton, OnSimpleScaleDraw) and CSceneCardList hover block.
4. **capp.cpp:** ✅ FIXED — PLANE TEST: guard `Load` with `if (bgplane.get())` / `if (charaplane.get())`; all switch cases guard `bgplane.get()` / `charaplane.get()` before use.
5. **CSceneYesNo:** ✅ FIXED — `m_vBackground->GetSurfaceInfo()` removed; draw guarded with `if (m_vBackground.get())`.
6. **Buffer / string bounds:** ✅ FIXED — `sprintf` → `sprintf_s(buffer, sizeof(buffer), ...)` in scene files.
7. **SetMouse non-ownership:** ✅ DOCUMENTED — Header comment + per-call "Non-owning: m_mouse outlives scene".
8. **pSrc 0xDDDDDDDD in GeneralBlt (CSceneMainMenu):** ✅ FIXED — m_title1/m_title2/m_menu1..m_menu7 changed from ISurface* to CPlane; originalSurface null checks in MainMenu and Settings; m_pMessageSurface guard in CSceneYesNo. "Non-owning: m_mouse outlives scene".

---

## Full-scan additional fixes (OpenJoey/src)

9. **CScene1 – null deref on default bgPlane:** ✅ FIXED — `test/CScene1.cpp`: default `CPlane bgPlane` is empty; `bgPlane->Load(...)` crashed. Create plane in `OnInit()` with `bgPlane = CPlane(new CFastPlane())` then call `Load`. Guard `OnDraw` with `if (bgPlane.get())` before `CircleBlt1`.

10. **CSceneSettings – unused ISurface* slider members:** ✅ FIXED — `test/CSceneSettings.h`: `m_settingsVolumeSlider1/2/3` changed from `ISurface*` to `CPlane` for consistency (currently unused; safe if re-enabled).

11. **CBinSystem – buffer bounds:** ✅ FIXED — `system/cards/CBinSystem.cpp`: all `sprintf(fullPath, ...)` / `sprintf(indexPath, ...)` replaced with `sprintf_s(..., sizeof(...), ...)`.

12. **CSceneYesNo – commented debug sprintf:** ✅ FIXED — `test/CSceneYesNo.cpp`: inside commented debug block, `sprintf` → `sprintf_s(buf, sizeof(buf), ...)` for consistency when uncommented.

13. **yaneGUITextBox – dangling thumb in SetSliderLoader:** ✅ FIXED — `system/backport/yaneGUITextBox.cpp`: local `CPlane pln = ...GetPlane(5)` was destroyed at end of function; `m_vSliderThumbGraphic` pointed at freed surface. Added member `CPlane m_sliderThumbPlane` to hold ownership; assign `GetPlane(5)` to it and pass non-owning ref to `SetPlane`.

14. **yanePlaneEffectBlt – FadeBlt null pointers:** ✅ FIXED — `system/effects/yanePlaneEffectBlt.cpp`: `FadeBlt(lpDraw, lpPlane, ...)` now returns 0 if `!lpDraw || !lpPlane` before calling `lpPlane->GetSize(...)`.

15. **CBinDecompress – strcpy bounds:** ✅ FIXED — `system/cards/CBinDecompress.cpp`: `strcpy(tempName, filename)` replaced with `strcpy_s(tempName, sizeof(tempName), filename)`. "Non-owning: m_mouse outlives scene".

16. **Main menu – green back buffer / UI misposition:** ✅ FIXED — Back buffer now explicitly cleared to black (`GetSecondary()->SetFillColor(0)` in `yaneFastDraw.cpp` after CreateSecondary). Main menu title, backdrop, and buttons use positions from `data/y/title/title.txt` scaled 640×480→800×600 (`CSceneMainMenu.cpp`). If a plane has no position in title.txt, GetXY returns (0,0) and that plane is drawn at (0,0)—safe. “Non-owning: m_mouse outlives scene”.
