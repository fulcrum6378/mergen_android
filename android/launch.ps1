# cd ..
gradle : android: assembleDebug
adb install -t android/build/outputs/apk/debug/android-debug.apk
adb shell am start -n ir.mahdiparastesh.mergen/ir.mahdiparastesh.mergen.Main
