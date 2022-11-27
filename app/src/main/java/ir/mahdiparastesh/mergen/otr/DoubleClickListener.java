package ir.mahdiparastesh.mergen.otr;

import android.os.SystemClock;
import android.view.View;

public abstract class DoubleClickListener implements View.OnClickListener {
    private long lastClickAt = 0;

    @Override
    public void onClick(View v) {
        if ((SystemClock.elapsedRealtime() - lastClickAt) < 200)
            onDoubleClick();
        lastClickAt = SystemClock.elapsedRealtime();
    }

    public abstract void onDoubleClick();
}
