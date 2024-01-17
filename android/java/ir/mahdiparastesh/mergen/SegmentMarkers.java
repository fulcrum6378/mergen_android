package ir.mahdiparastesh.mergen;

import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

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
     * Java numbers are always big-endian! (bits have the same order, but bytes need to be reordered)
     */
    public void update(long[] data) {
        int best;
        short cx, cy; // don't make them `int`
        //StringBuilder sb = new StringBuilder();
        for (int sid = 0; sid < data.length; sid++) {
            TextView tv = (TextView) pool.getChildAt(sid);
            best = (int) (data[sid] & 0x00000000FFFFFFFFL);
            if (best == -2) {
                tv.setVisibility(View.INVISIBLE);
                continue;
            }
            cx = (short) ((data[sid] & 0x0000FFFF00000000L) >> 32);
            cy = (short) ((data[sid] & 0xFFFF000000000000L) >> 48);
            //sb.append(best).append(":").append(cx).append("x").append(cy).append(", ");
            float scale = 1088f / 720f;
            tv.setTranslationY(((float) cx) * /*density * */scale);
            tv.setTranslationX(((float) cy) * /*density * */scale);
            tv.setVisibility(View.VISIBLE);
        }
        //Toast.makeText(c, sb.toString(), Toast.LENGTH_SHORT).show();
    }
}
