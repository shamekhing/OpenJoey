# OpenJoey - Open Source reimplementation of Yu-Gi-Oh! - Power of Chaos
**OpenJoey**: a free, open source black-box re-implementation (NOT a decompilation) of the [Power of Chaos](https://yugioh.fandom.com/wiki/Yu-Gi-Oh!_Power_of_Chaos) game serires.
Powered by the yaneSDK engine for accurace reimplementation.

## Work in progress
This project is in the *very* early stages. There is still a long way to go before there's anything playable. 
Pull requests are always welcome.

## Discord/Contact
[Yu-Gi-Oh! Power of Chaos/Online modding discord](https://discord.com/invite/GAKKaJYwF7)

## What is yaneSDK?
TL;DR: Early 2000s free C++ engine for japanese indie devs that was also used for Power of Chaos games.

## What do i need to compile this project?
The "TEST" project file is for Code::Blocks. You will also need:
* Microsoft Visual C++ Toolkit 2003 (the SDKv3 has also support for VC++6/.NET/.NET2003 and BCC5.5/BCB5/6!, look in yaneSDK\ProjectFiles folder)
* Microsoft Platform SDK (older one that plays nice with Visual C++ Toolkit 2003)
* Microsoft DirectX SDK (yaneSDK has support for DX3 up to DX8) - this project assumes the usage of DX 6.1 or higher

or...
* Just use Visual Studio .NET 2003 (VC7.1) with everything preinstalled.

## Untouched yaneSDK source code
Raw untouched source code is in misc folder. (v1-v5)
The engine is used in way more games (mostly older visual novels).
YaneSDK can also be compiled in lib mode.

# Where was yaneSDK used?
It was also used by other japanese games in early 00's (way too many to count, most confirmed ones are visual novels) and is used even to this day for some VNs.

## Documentation
[Official YaneSDK Homepage](https://bm98.yaneu.com/yaneSDK.html).
There is also a [gamedev programming book series](https://www.amazon.co.jp/Windowsプロフェッショナルゲームプログラミング-やね-うらお/dp/479800314X) on it how to use the engine (but in japanese only).

## Runtime data (game data)

Card/game data is required at runtime but is omitted from this repo to reduce legal risk.

### Where to place data

Next to the executable (e.g. in the same folder as `YaneSkeleton.exe`), create a `data` folder with:

- `data\bin#\` — card .bin files
- `data\card\` — list_card.txt and card images
- `data\mini\` — list_card.txt and mini card images
- `data\y\` — title, list, etc. (UI data)
- `data\cursor\` — cursor/UI assets

### Where to obtain data

Copy the data folder from:

- A legally acquired Power of Chaos installation (e.g. "Yu-Gi-Oh! Power of Chaos - The Ancient Duel", or the game's data folder), or
- A mod data template (e.g. yu-gi-oh_mod_data_stock_template).

### Required files

**In `data\bin#`** (for English): `card_prop.bin`, `card_nameeng.bin`, `card_id.bin`, `card_intid.bin`, `card_desceng.bin`, `card_indxeng.bin`, `dlg_texteng.bin`, `dlg_indxeng.bin`, `card_pack.bin`.  
Also **`data\card\list_card.txt`** and **`data\mini\list_card.txt`**.

## Legal disclaimers
* This project is not affiliated with or endorsed by KONAMI or KABUSHIKI KAISHA SHUEISHA in any way. Yu-Gi-Oh! is a trademark of KABUSHIKI KAISHA SHUEISHA.
* This project is non-commercial. The source code is available for free and always will be.
* OpenJoey is nowhere near playable yet, but when it is and you want to play with it,
  you will need to have a legally acquired installation of one of those games. OpenJoey uses data files from the original games. 
* This is a blackbox re-implementation project. The code in this project was written based on reading data files, 
  and observing the game running. For engine functionality, the public domain library yaneSDK was used.
  I believe this puts the project in the clear, legally speaking. If someone at KONAMI or KABUSHIKI KAISHA SHUEISHA disagrees, please talk to me.
* If you want to contribute to this repository, your contribution must be either your own original code, or open source code with a
  clear acknowledgement of its origin. No code that was acquired through reverse engineering executable binaries will be accepted.
* No assets from the original games are included in this repo.

## License
The source code provided in this repository for OpenJoey is licenced under the [GNU General Public License version 3](https://www.gnu.org/licenses/gpl.html).

The original yaneSDK was released as public domain by Motohiro Isozaki aka [yaneurao](https://github.com/yaneurao).
You can also download the original source code on his website if you want.
[Official YaneSDK Homepage](https://bm98.yaneu.com/yaneSDK.html)

## TODO
* CI compile (something like a Docker image with msvc toolkit & wine)