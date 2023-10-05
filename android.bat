CALL gradle :android:assembleDebug
CALL adb install -t android/build/outputs/apk/debug/android-debug.apk
CALL adb shell am start -n ir.mahdiparastesh.mergen/ir.mahdiparastesh.mergen.Main
