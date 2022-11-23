package ir.mahdiparastesh.mergen;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.AudioManager;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;
import android.view.ViewGroup;
import android.widget.Toast;

import java.util.Arrays;

import ir.mahdiparastesh.mergen.otr.DoubleClickListener;

@SuppressLint("ClickableViewAccessibility")
public class Main extends Activity implements TextureView.SurfaceTextureListener {
    private final int permCode = 372;
    private final String[] requiredPerms =
            new String[]{Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO};
    private DisplayMetrics dm;
    private long ndkCamera;
    private TextureView preview;
    private Surface surface = null;
    private boolean isRecording = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        preview = findViewById(R.id.preview);
        dm = getResources().getDisplayMetrics();

        if (Arrays.stream(requiredPerms).anyMatch(s -> checkSelfPermission(s) < 0))
            requestPermissions(requiredPerms, permCode);
        else permitted();

        String tested = test();
        if (tested != null) Toast.makeText(this, tested, Toast.LENGTH_LONG).show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == permCode) {
            if (Arrays.stream(grantResults).sum() == 0) permitted();
            else onBackPressed();
        }
    }

    private void permitted() {
        int dim = Math.min(dm.widthPixels, dm.heightPixels),
                w = dim, h = dim;
        ViewGroup.LayoutParams previewLP = preview.getLayoutParams();
        previewLP.width = w;
        previewLP.height = h;
        preview.setLayoutParams(previewLP);

        AudioManager audioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        ndkCamera = prepare(
                Integer.parseInt(audioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE)),
                Integer.parseInt(audioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER)));

        onRecordingStopped();
        preview.setSurfaceTextureListener(this); // don't make it in-line.
        if (preview.isAvailable()) onSurfaceTextureAvailable(
                preview.getSurfaceTexture(), preview.getWidth(), preview.getHeight());
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        Size cameraPreviewSize = getMinimumCompatiblePreviewSize(ndkCamera);
        surfaceTexture.setDefaultBufferSize(
                cameraPreviewSize.getWidth(), cameraPreviewSize.getHeight());
        surface = new Surface(surfaceTexture);
        onSurfaceStatusChanged(true, ndkCamera, surface);
        preview.setClickable(true);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        preview.setClickable(false);
        recording(false);
        onSurfaceStatusChanged(false, ndkCamera, surface);
        ndkCamera = 0;
        surface = null;
        return true;
    }

    private void recording(boolean bb) {
        if (bb == isRecording) return;
        byte res;
        if (!isRecording) res = start();
        else res = stop();
        Toast.makeText(this,
                (!isRecording ? "STARTED" : "STOPPED") + ": " + (int) res,
                Toast.LENGTH_SHORT).show();
        if (res != 0) return;
        isRecording = bb;
        if (isRecording) onRecordingStarted();
        else onRecordingStopped();
    }

    private void onRecordingStarted() {
        preview.setOnClickListener(null);
        preview.setOnTouchListener((v, event) -> {
            // TODO IMPLEMENT HPT {@link android.view.MotionEvent event}
            return false;
        });
    }

    private void onRecordingStopped() {
        preview.setOnTouchListener(null);
        preview.setOnClickListener(new DoubleClickListener() {
            @Override // never use it like lambda!
            public void onDoubleClick() {
                recording(true);
            }
        });
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


    private native long prepare(int sampleRate, int framesPerBuf);

    private native byte start();

    private native byte stop();

    private native void destroy();

    private native String test();

    private native Size getMinimumCompatiblePreviewSize(long ndkCamera);

    private native void onSurfaceStatusChanged(boolean available, long ndkCamera, Surface surface);

    static {
        System.loadLibrary("main");
    }
}
