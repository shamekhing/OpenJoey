# Deck Editor Exit – Analysis (think first, code later)

## What we know
- User clicks "Deck construction" (first/main menu option).
- App exits with **code 0** (clean exit).
- No crash dialog; DLL load/unload and thread exits look normal.

## How the app can exit with code 0
The main loop is `while (IsThreadValid()) { ... }`. So we leave it only if:

1. **`IsThreadValid()` becomes false**  
   Something closes the main thread (e.g. `Close()` / thread manager).

2. **We hit `break`**  
   The only `break` in the loop is:
   - `if (key.IsKeyPush(0))` (ESC) **and**
   - `GetSceneNo() == SCENE_MAINMENU || GetSceneNo() == SCENE_SPLASH`  
   So we break only when user presses ESC **and** we're on main menu or splash.

So either:
- **(A)** Something is closing the thread (e.g. `OnPreClose` / `Close()`), or  
- **(B)** We're taking the ESC `break` even though the user didn’t intend to exit.

## Flow when user clicks “Deck construction”
1. **MainMenu::OnMove**  
   - First button (i=0) sets `m_nButton = 1`.  
   - `case 1:` → `CallSceneFast(SCENE_DECKEDITOR)` (no `OnPreClose`).

2. **CSceneControl::OnMove** (same or next frame)  
   - Message 3 → `CreateScene(SCENE_DECKEDITOR)`.  
   - Factory: `new CSceneDeckEditor()`, `SetApp`, return.  
   - CreateScene: `SetSceneControl`, **OnInit()**, then push scene.  
   - Stack becomes [MainMenu, DeckEditor]. Current scene = DeckEditor.

3. **CApp::MainThread**  
   - Calls `OnMove(surface)`, then `OnDraw(surface)`, then `key.Input()`, ESC check, `WaitFrame()`.

So if the **first** menu option is really “Deck construction” and it’s button index 0 → `m_nButton == 1`, we should **not** call `OnPreClose` and we **should** transition to the deck editor. For the app to exit we need (A) or (B) above.

## Hypotheses

### H1 – Wrong button mapping (most plausible without logs)
- **First visible** menu item might not be `m_vButtons[0]`.  
- If the first **drawn** option is actually the second button (index 1), then one click sets `m_nButton = 2` → `OnPreClose()` → window close → clean exit.  
- So the user might be clicking what looks like “Deck construction” but the hit-test maps to the **exit** action.

**Check:** Swap actions so that **case 2** opens the deck editor and **case 1** does nothing (or something harmless). If exit stops when clicking “Deck construction”, then the first visual button was really case 2 (exit).

### H2 – Exception before Deck Editor runs
- If `new CSceneDeckEditor()` or a **member/base constructor** throws (e.g. `CKey1` / `CKeyInput` / `CJoyStick`), the factory throws.  
- CreateScene then throws **before** `OnInit()` and **before** `push_back`.  
- The exception may be caught by the thread/app wrapper and turn into a clean process exit (code 0).  
- In that case we **never** enter `CSceneDeckEditor::OnInit()`.

**Check:** Add a single `OutputDebugStringA("DeckEditor: OnInit\n");` at the **very first line** of `CSceneDeckEditor::OnInit()`.  
- If you **never** see it when clicking “Deck construction”, the failure is **before** OnInit (constructor or factory).  
- If you **do** see it, the failure is **in** OnInit or **after** (OnMove/OnDraw or code that runs after transition).

### H3 – ESC or “exit” triggered by something else
- Another component might be synthesizing “key 0” or an exit request when the deck editor is created or first run.  
- Less likely if we don’t press ESC, but possible with focus/input bugs.

### H4 – Thread/Close() from framework
- Some framework or DirectX/device-lost path might call `Close()` when transitioning scenes.  
- Would need a breakpoint or log inside `Close()` / `IsThreadValid()` to confirm.

## Recommended next step (one diagnostic, no big refactor)
1. Add **one** debug line at the **very first line** of `CSceneDeckEditor::OnInit()`:  
   `OutputDebugStringA("DeckEditor: OnInit\n");`
2. Run, click “Deck construction”, and check Debug output:
   - **No "DeckEditor: OnInit"** → problem is **before** the deck scene (constructor/factory or wrong button → exit). Then try **H1** (swap case 1 / case 2).
   - **"DeckEditor: OnInit" appears** → we **do** enter the deck editor; problem is **inside** OnInit or **after** (OnMove, OnDraw, or something that runs right after the transition).

After that, we can either add a second log (e.g. first line of OnMove/OnDraw) or swap the menu actions, depending on what the first log shows.

---

## Debug trail added (run and check Output window)

All messages are prefixed with `[OpenJoey]` so you can filter.

1. **MainMenu** – When a button is clicked: `MainMenu: button N clicked (index i)`, then either `case 1 -> OnPreClose (exit)` or `case 2 -> CallSceneFast(DECKEDITOR)` etc.
2. **MainLoop** – When closing: `MainLoop: m_bWindowClosing -> CallSceneFast(ISEND)`; when ESC: `ESC -> ReturnScene` or `ESC on main/splash -> break (exit)`.
3. **DeckEditor** – `DeckEditor: OnInit`, then `OnInit done` (or `OnInit exception`), then on first frame `DeckEditor: OnMove (first frame)`.

**How to read it:** Click “Deck construction”, then look at the **last** `[OpenJoey]` line before the process exits. That tells you where the exit path started (wrong button → case 1, or DeckEditor then something else).
