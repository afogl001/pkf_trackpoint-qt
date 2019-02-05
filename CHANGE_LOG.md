# **v1.1**
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
