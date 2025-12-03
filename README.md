This project aims to have multiple movesets (Styles) available, while switching between them in-game in real time.
It has been rewritten to include various improvements.

Current features:
- Switch between styles in-game based on a Style list defined in an .ini file through buttons
- All settings can be found in Styles.ini too.
- Can change settings in game by accessing the ImGui Menu with a key. Default keys are:
Number 4 to change Style in keyboard, LEFT_STICK_PRESS + B/CIRCLE in controller, Number 9 to open the menu on keyboard. These are remappable.
- Infinite amount of Styles, cycling through them.
- Plays a sound for switching Styles; uses a different when reaching vanilla.
- Can start from an specific Style.
- Can keep the previous Style after restarting a level.
- Can disable the ImGui menu via itself.
Compared to previous versions, the Styles format has been changed, it has better stability and the Weapon Switcher has been removed to avoid crashes. A separate mod might be released for that.

Credits to:
RuslanchikX for the original idea and the original version of Style Switcher.
_all_of_this_ends_here (a.k.a FutureGohan) for adapting Style Switcher to the newest MGR PLUGIN SDK version.
Space Core for general help in the project and with C++.
Aura for rewriting Style Switcher almost completely for better readability and implement some of the improvements.
Frouk for the mgr-plugin-sdk and the ImGui template.
The ImGui team for ImGui.
