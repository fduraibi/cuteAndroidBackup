# cuteAndroidBackup
Basic application written in Qt to backup android apps through ADB

It is built using Qt Creator against Qt 5.5 and currently does basic backup of applications data, no restore yet.

You can select multiple packages from the list and the application will back them up one after the other.

# TODO:
- Add buttons to reconnect and re-list applications
- Add restore functionality
- Add more backup options (backup with apk, with odd ...etc)
- Add the ability to copy apk (adb pull /xxx/xxx.apk)
- Find a way to get the actual package names.
- Better handling of errors and user notifications
- Allow to select between multiple connected android devices (currently it connects to the default)
- Add option to set backup folder

Feel free to submit pull-requests for fixes and added features.

<img src="/images/cuteAndroidBackup.png">
