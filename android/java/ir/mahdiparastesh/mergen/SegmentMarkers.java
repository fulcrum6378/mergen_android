package ir.mahdiparastesh.mergen;

import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;

/** Visualises how image frames are being analysed. */
public class SegmentMarkers {
    static final int ALL = 20; // `MAX_SEGS` in segmentation.hpp
    static final String[] dict = new String[]{
            "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
            "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
            "U", "V", "W", "X", "Y", "Z",
            "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
            "k", "l", "m", "n", "o", "p", "q", "r", "s", "t",
            "u", "v", "w", "x", "y", "z",
    };

    final Main c;
    final RelativeLayout pool;

    public SegmentMarkers(Main c, RelativeLayout pool) {
        this.c = c;
        this.pool = pool;

        for (int i = 0; i < ALL; i++) {
            TextView tv = new TextView(c, null, 0, R.style.segmentMarker);
            tv.setText(dict[i]);
            this.pool.addView(tv);
        }
    }

    /**
     * Called by C++ to update the pointers on the screen.
     * Assuming this machine is little-endian!
     */
    public void update(long[] data) {
        int best, cx, cy;
        for (int sid = 0; sid < data.length; sid++) {
            TextView tv = (TextView) pool.getChildAt(sid);
            best = (int) data[sid];
            if (best == -2) {
                tv.setVisibility(View.INVISIBLE);
                continue;
            }
            cx = (int) ((data[sid] >> 4) & 0xFFFF);
            cy = (int) ((data[sid] >> 6) & 0xFFFF);
            tv.setTranslationX((float) cx);
            tv.setTranslationY((float) cy);
            tv.setVisibility(View.VISIBLE);
        }
    }
}
