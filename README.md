# **pkf_trackpoint-qt** v1.0.0
Qt application for configuring a TrackPoint

## Description
This program will allows you to change the speed, sensitivity, and press_to_select settings of the TrackPoint on ThinkPads running GNU/Linux ***(See "Limitations" below)*** .  

## Usage
### Modes
- Normal: Run program as root or by running it with "sudo" for use of all features
- ReadOnly: Run program as normal user to view current TrackPoint settings
- Testing:  Create the directory "/tmp/pkf_trackpoint" and create three files, "speed", "sensitivity", and "press_to_select" inside the directory.  Then run as normal user.  
*NOTE: Running as root/sudo with these files will still execute the program in "Normal" mode*

### Operation
- To change one or more settings of the TrackPoint, enter the appropriate value in the text field and click "Apply".  Text fields left blank are ignored.

- To enable current settings to persist reboots, check the "Enable Persistence" checkbox

- To disable persistence (and remove any files created for persistence), un-check the "Enable Persistence" checkbox

- To reset the TrackPoint settings to default values, click "Defaults".  

## Limitations
All Limitations will be addressed in future versions
- Persistence currently only works for systemd
- Only detects common TrackPoint directories (i.e., "/sys/device/platform/i8042...").  Some GNU/Linux distributions (such as KDE Neon) use a less common directory.

### Disclaimer...
This program was written by a novice who is just learing C++ and Qt.  As such, the author is aware that there is much room for improvement in the code and appreciates any suggestions for cleaning it up or making it more efficient.  The author can be reached at "afogl001@members.fsf.org".    

### Planned Improvements
- Fix minor offset of labels.
- Add initd support for persistence
- Add less common TrackPoint configuration paths (KDE Neon)
- Allow user to run and prompt for escalated privileges
- Add menu item to enable/disable Test mode (prompt user to restart program for changes to take effect)
- Add menu items for "Close", "Help", and "About".
- Clean up code by
  - Making better use of functions and parameters
  - Leverage variables to reduce repetitious text/values
  - Find better way of creating persistence files (instead of just streaming line-by-line)
  - Consider breaking out some code from main and put in other classes
- Create binaries
- Create .deb and .rpm packages

### **Code overview**
#### Process Flow
The main loop will...
1. Set the path for TrackPoint settings by running "setSettingsPath", which will...
  - If in "Test" mode **(see "Modes" above)**, set path to the testing directory "/tmp/pkf_trackpoint"
  - Otherwise, as long as a TrackPoint settings directory is detected (depends on existence of trackpad), set path appropriately
  - If no TrackPoint setting directory is detected, alerts user and exits program
1. Set the path for persistence files by running "setPersistPaths", which will...
  - If in "Test" mode **(see "Modes" above)**, set path to the testing directory "/tmp/pkf_trackpoint"
  - Otherwise, as long as an initialization system is detected (currently only supports systemd), set path appropriately
  - If no initialization setting directory is detected, sets path to "N/A", which will prevent the user from checking the "Enable Persistence" checkbox.
1. Check whether the user is root or not by running "checkUser", which will...
  - If effective user is root, sets initialization command to actual command (i.e., "systemctl" for systemd)
  -  If effective user is anyone other than root, sets initialization command with prefixed "echo" (i.e., "echo systemctl" for systemd) and displays text in Info label for the user to run as root or with sudo to make changes.
1. Set validators for the text input fields by running "validateSettings" which will...
  - Set the input text boxes for speed and sensitivity only accept values 1-255 (though user still permited to enter "0")
  - Set the input text box for press_to_select to only accept values 0-1
1. Display the current TrackPoint settings by running "displaySettings", which will...
  - Read TrackPoint configuration settings and print them to the appropriate labels.
1. Check the "Enable Persistence" checkbox if it detects persistence has been enabled.

User can then...
1. Enter values into text boxes and click "Apply", which will...
  - Run "applySettings" for all three settings, passing in the appropriate settings path and text of respective text box, which will in turn apply the setting only if the text box was not empty.
  - Update the displayed values by running "displaySettings"
  - Run "installTrackpointSH" **(covered below in "Enable Persistence...")**, only if persistence is enabled, to apply changes made
  - Clear all text boxes.
1. Reset TrackPoint settings to defaults by clicking "Defaults", which will...
  - Run "applySettings" for all three settings, passing in the appropriate settings path and default value, which will in turn apply all settings to default values
  - Update the displayed values by running "displaySettings"
  - Run "installTrackpointSH" **(covered below in "Enable Persistence...")** only if persistence is enabled, to apply changes made   
1. Enable Persistence by checking the "Enable Persistence" which will...
  - If status is changing to "checked"...
    - If no initialization system is detected, alert user and un-checks the checkbox
    - Else, runs functions to create files for persistence, then executes commands to run and enable persistence via the initialization system
  - If status is changing to "unchecked"...
    - If no initialization system is detected, notification passed to console that nothing is changing.
    - Else, commands run to disable and stop persistence, then all files created for persistence are removed, then initialization is reloaded if required.
