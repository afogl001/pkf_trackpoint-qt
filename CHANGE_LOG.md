# **v2.0**
## v2.0.0
  - Features
    + Add "Enable scrolling" for distros using "Evdev" device manager
    + Add window icon
  - Other
    + Add variable "runMode" to track if running in standard mode (with root privileges), "Test" mode, or "ReadOnly" mode
    + Updated some misc. comments in code
    + Add "CONFIG += console" to project file in order to view qDebug info on CentOS and OpenSUSE builds of Qt Creator.
    + Removed statusBar and mainToolBar as they were not being used and took up screen space.
  - Bugs
    + Introduced unsetting scrolling settings bug.  If 1) running in standard mode, 2) toggling "Test" mode off and on again, 3) distro running Evdev, and 4) user has enabled scrolling, then scrolling will be unset.  Remediation is to reset scrolling (by click the appropriate checkbox) before exiting.  While there are several methods of addressing this bug, it's numerous prerequisites and ease of remediation don't seem to warrant introducing additional code to address.  This may be re-investigated later or if a sufficiently simple method of address becomes apparent.
    + Introduced delay in starting pkf_trackpoint-qt.  If 1) running a distro that uses Evdev, 2) uses a slow and large HDD, and 3) does not have scrolling set, then pkf_trackpoint-qt may take up to 35 seconds to start (running CentOS on X220 w/ 5400rpm HDD).  This is due to the "checkDeviceMgr" function recursively searching possible locations of the "xorg.conf.d" directory with the "90-trackpoint.conf" file.  Added addressing this under the "Planned Improvements" section of README but the likelihood and impact of bug does not warrant withholding release of existing version.         


# v1.2
## v1.2.0
  - Features
    + Add dynamic discovery of TrackPoint settings path
    + Add condition if TrackPoint not discovered, open in "Test" mode
  - Other
    + Clean up some conditionals with explicit brackets
# v1.1
## v1.1.0
  - Features
    + Add support for KDE Neon
    + Add menu entry for toggling "Test" mode
    + Enable root user to use "Test" mode
  - Fixes
    + Fix typo where un-checking "Set Persistence" did not remove systemd files
    + Fix plethora of typos in trackpoint.sh file
    + Fix typo/misspelling of several instances of "sensitivity"
    + Current settings maintained when un-checking "Set Persistence"
  - Other
   + Remove dynamic path assignment in trackpoint.sh.  Provided fail-safe if moved to other host but was largely unnecessary and redundant
   + Dynamically add value for trackpoint.service "ConditionPathExists" instead of statically listing all possibilities
   + Add status check for "Persist" checkbox in "setPersistPaths"
   + Update README
   + Remove extraneous/unused variables for specific setting values (e.x., "speedPath")

# v1.0
Initial functional release


# v0.0.1
Initial commit of non-functional draft
