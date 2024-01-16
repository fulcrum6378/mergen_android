package ir.mahdiparastesh.mergen;

import android.widget.RelativeLayout;

/** Visualises how image frames are being analysed. */
public class VisualDebug {
    final Main c;
    final RelativeLayout pool;

    public VisualDebug(Main c, RelativeLayout pool) {
        this.c = c;
        this.pool = pool;
    }

    public void update(long[] data) {
        /*Toast.makeText(c, (int) data[0] + ", " + (int) data[1] + ", " + (int) data[2] + ", " + (int) data[3],
                Toast.LENGTH_SHORT).show();*/
    }
}
