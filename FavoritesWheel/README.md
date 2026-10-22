# Favorites Wheel 1.0.0

Separate SKSE plugin for Skyrim Special Edition / Anniversary Edition, Windows, mouse and keyboard. No SurvivalSync installation is required. Requires SKSE64 matching your Skyrim runtime and Address Library for that runtime. VR is not supported.

Install the ZIP with MO2/Vortex, or copy its SKSE folder into Skyrim's Data folder. Launch through SKSE (or the SKSE-enabled Skyrim Together launcher). Do not replace SurvivalSync files.

## Controls

- Hold middle mouse, move toward a favorite, release to equip/use it.
- Q / E changes GEAR, Magic, Potions. The last category is remembered between openings and game launches.
- The empty center cancels. Esc or Tab cancels. A different menu, loading or losing game focus cancels.
- Mouse wheel / Page Up / Page Down shows more items in a category, ten per ring.
- F1 while the wheel is open opens settings. Click an option, or press F3 for hold/release, F4 for toggle/click, H for equip hand. F1/Esc returns.
- Toggle/click mode: middle mouse opens, left click chooses, middle mouse again cancels.

Favorites are read from the game each time the wheel opens. GEAR includes favorited weapons, armor, ammunition and torches. Magic includes favorited spells, powers, shouts and scrolls. Potions includes favorited alchemy consumables and poisons. Gear and spells are equipped; potions use Skyrim's native consume/equip action. Selecting a spell does not cast it. Powers/shouts go to the voice slot. Right hand is default; change it in settings.

Settings live beside the plugin in Data/SKSE/Plugins/FavoritesWheel.ini. SelectionMode=0/1, Hand=0/1, LastWheel=0/1/2. Radius=180..380 and SensitivityPercent=25..250 can be adjusted there with the game closed. Defaults are restored by deleting the INI with the game closed.

The wheel does not pause or slow the world; movement remains available. Mouse aiming and wheel-related inputs are captured while open. Favorited item variants are rechecked before equipping; stale entries are not replaced with a different enchanted or tempered item. No ESP, Papyrus scripts or save records are added.

Other mods binding middle mouse or Q/E may need their bindings changed. Do not open another radial menu on the same binding.

Build verification and native two-PC gameplay are separate: this release must be checked in your actual load order. Diagnostics: Documents/My Games/Skyrim Special Edition/SKSE/FavoritesWheel.log. Unsupported native input call layouts leave the wheel disabled instead of applying an unchecked patch.

## Build

CMake 3.24+, Visual Studio 2022 C++ tools, vcpkg (x64-windows-static). Configure with the vcpkg toolchain, build Release, then run ctest. CommonLibSSE-NG and Dear ImGui sources are pinned in CMakeLists.txt; their MIT licenses ship with the package.
