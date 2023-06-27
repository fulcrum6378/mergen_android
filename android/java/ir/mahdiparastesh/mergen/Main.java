package ir.mahdiparastesh.mergen;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemClock;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.Toast;

import java.util.Arrays;

@SuppressLint("ClickableViewAccessibility")
public class Main extends Activity implements TextureView.SurfaceTextureListener {
    private long ndkCamera;
    private RelativeLayout root;
    private TextureView preview;
    private Surface surface = null;
    private boolean isRecording = false;
    private Toast toast;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        root = findViewById(R.id.root);
        preview = findViewById(R.id.preview);

        // ask for camera and microphone permissions
        String[] requiredPerms =
                new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, 1);
        else permitted();

        // toast any test string values
        String tested = test();
        if (tested != null) Toast.makeText(this, tested, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (Arrays.stream(grantResults).sum() == 0) permitted();
        else onBackPressed();
    }


    private void permitted() {
        ndkCamera = create();
        Size size = getCameraDimensions(ndkCamera);
        /*Toast.makeText(this, size.getWidth() + " : " + size.getHeight(),
                Toast.LENGTH_SHORT).show();*/
        onRecordingStopped();
        ViewGroup.LayoutParams previewLP = preview.getLayoutParams();
        previewLP.width = size.getWidth();
        previewLP.height = size.getHeight();
        preview.setLayoutParams(previewLP);
        preview.setSurfaceTextureListener(this);
        if (preview.isAvailable()) onSurfaceTextureAvailable(
                preview.getSurfaceTexture(), size.getWidth(), size.getHeight());

        /*import java.io.File;
        import java.io.FileOutputStream;
        import java.io.IOException;
        import java.io.InputStream;
        File senses = new File(getFilesDir(), "senses.xml");
        if (!senses.exists()) try {
            InputStream src = getResources().openRawResource(R.raw.senses);
            FileOutputStream fos = new FileOutputStream(senses);
            int byt;
            while ((byt = src.read()) > -1) fos.write(byt);
            src.close();
            fos.close();
        } catch (IOException ignored) {
        }*/
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        surfaceTexture.setDefaultBufferSize(width, height);
        surface = new Surface(surfaceTexture);
        onSurfaceStatusChanged(ndkCamera, surface, true);
        root.setClickable(true);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        root.setClickable(false);
        recording(false);
        onSurfaceStatusChanged(ndkCamera, surface, false);
        ndkCamera = 0;
        surface = null;
        return true;
    }

    private void recording(boolean bb) {
        if (bb == isRecording) return;
        byte res;
        if (!isRecording) res = start();
        else res = stop();
        if (toast != null) toast.cancel();
        toast = Toast.makeText(this,
                (!isRecording ? "STARTED" : "STOPPED") + ": " + (int) res,
                Toast.LENGTH_SHORT);
        toast.show();
        if (res != 0) return;
        isRecording = bb;
        if (isRecording) onRecordingStarted();
        else onRecordingStopped();
    }

    private void onRecordingStarted() {
        root.setOnClickListener(null);
        root.setOnTouchListener((v, ev) -> {
            Main.this.onTouch(new float[]{
                    ev.getAction(), ev.getRawX(), ev.getRawY(), ev.getSize(), ev.getPressure()});
            return true;
        });
    }

    private void onRecordingStopped() {
        root.setOnTouchListener(null);
        root.setOnClickListener(new DoubleClickListener() {
            @Override
            public void onDoubleClick() {
                recording(true);
            }
        });
    }

    @SuppressWarnings("unused")
    void shake(long dur) {
        Vibrator vib = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) ?
                ((VibratorManager) getSystemService(Context.VIBRATOR_MANAGER_SERVICE))
                        .getDefaultVibrator() : (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        vib.vibrate(VibrationEffect.createOneShot(dur, VibrationEffect.DEFAULT_AMPLITUDE));
    }

    @Override
    public void onBackPressed() {
        if (isRecording) {
            recording(false);
            return;
        }
        super.onBackPressed();
    }

    @Override
    protected void onDestroy() {
        surface.release();
        destroy();
        super.onDestroy();
    }


    public abstract static class DoubleClickListener implements View.OnClickListener {
        private long lastClickAt = 0;

        @Override
        public void onClick(View v) {
            if ((SystemClock.elapsedRealtime() - lastClickAt) < 200) onDoubleClick();
            lastClickAt = SystemClock.elapsedRealtime();
        }

        public abstract void onDoubleClick();
    }


    /** Structs utilities required for recording. */
    private native long create();

    /** Starts recording. */
    private native byte start();

    /** Stops recording. */
    private native byte stop();

    /** Destructs utilities required for recording. */
    private native void destroy();

    /** Shows test string values coming from C++ */
    private native String test();

    /** Camera cannot record in any dimension you want! */
    private native Size getCameraDimensions(long cameraObj);

    private native void onSurfaceStatusChanged(long cameraObj, Surface surface, boolean available);

    private native void onTouch(float[] properties);

    static {
        System.loadLibrary("main");
    }
}
