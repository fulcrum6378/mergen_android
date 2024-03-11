# cd ..
gradle :android:assembleDebug  # do not let your IDE make spaces between ":" characters.
adb install -t android/build/outputs/apk/debug/android-debug.apk
adb shell am start -n ir.mahdiparastesh.mergen/ir.mahdiparastesh.mergen.Main
