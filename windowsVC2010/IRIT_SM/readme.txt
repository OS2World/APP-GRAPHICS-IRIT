The property page GlobalRules.vsprops is included by all projects.
The property page ExecRules.vsprops is included by all executable project and IritDll

Editing property page
---------------------
* Open the property manager in visual studio
* Navigate to one of the projects that includes the property page.
* Navigate down the tree until you get to the property page.
* Open the property page and edit it.

Requried actions when adding new library
----------------------------------------
Add GlobalRules.vsprops property page to your debug and release projects.
Add "libname.lib" to the user user macro IritLibs defined in GlobalRules.vsprops.
Add "libnameD.lib" to the user user macro IritLibsD defined in GlobalRules.vsprops.

Requried actions when adding new executable
-------------------------------------------
Add GlobalRules.vsprops property page to your debug and release projects.
Add ExecRules.vsprops property page to your debug and release projects.
