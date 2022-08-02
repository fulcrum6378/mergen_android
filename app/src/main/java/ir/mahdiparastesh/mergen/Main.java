package ir.mahdiparastesh.mergen;

import android.Manifest;
import android.app.NativeActivity;
import android.os.Bundle;

import java.util.Arrays;

public class Main extends NativeActivity {
    private final int permCode = 372;
    private final String[] requiredPerms =
            new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, permCode);
        else preview();
    } // onResume() is never invoked!?!?

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == permCode) {
            if (Arrays.stream(grantResults).sum() == 0) preview();
            else onBackPressed();
        }
    }

    public native void preview();

    static {
        System.loadLibrary("main");
    } // necessary for preview()
}
