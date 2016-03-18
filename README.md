# cuteAndroidBackup
Basic application written in Qt to backup and restore android apps through ADB

It is built using Qt Creator against Qt 5.5 and currently does backup and restore of applications data and files.

You can select multiple packages from the list and the application will back them up one after the other.

# TODO:
- Add the ability to copy apk (adb pull /xxx/xxx.apk)
- Find a way to get the actual package names.
- Better handling of errors and user notifications
- Allow to select between multiple connected android devices (currently it connects to the default)

Feel free to submit pull-requests for fixes and new features.

Version 1.0:
<img src="/images/cuteAndroidBackup_v1.0.png">

Version 1.1: (Following KDE Dark theme)
<img src="/images/cuteAndroidBackup_v1.1.png">
