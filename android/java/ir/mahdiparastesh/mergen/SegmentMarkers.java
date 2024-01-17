package ir.mahdiparastesh.mergen;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;

/** Visualises how image frames are being analysed by marking some of the largest segments. */
public class SegmentMarkers {
    final int ALL = 20; // `MAX_SEGS` in segmentation.hpp
    final String[] dict = new String[]{
            "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
            "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
            "U", "V", "W", "X", "Y", "Z",
            "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
            "k", "l", "m", "n", "o", "p", "q", "r", "s", "t",
            "u", "v", "w", "x", "y", "z",
    };
    final float posMultiplier = 1088f / 720f;
    final long transDur = 329L;

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

    AnimatorSet trans;

    /**
     * Called by C++ to update the pointers on the screen.
     * Java numbers are always big-endian! (bits have the same order, but bytes need to be reordered)
     */
    public void update(long[] data) {
        if (trans != null && trans.isStarted()) trans.cancel();

        int best;
        short cx, cy; // don't make them `int`
        float x, y;
        //StringBuilder sb = new StringBuilder();
        ArrayList<Animator> ans = new ArrayList<>();
        for (int sid = 0; sid < data.length; sid++) {
            TextView tv = (TextView) pool.getChildAt(sid);
            best = (int) (data[sid] & 0x00000000FFFFFFFFL);
            if (best == -2) {
                ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 0f));
                continue;
            }
            cx = (short) ((data[sid] & 0x0000FFFF00000000L) >> 32);
            cy = (short) ((data[sid] & 0xFFFF000000000000L) >> 48);
            //sb.append(best).append(":").append(cx).append("x").append(cy).append(", ");
            x = (((float) cx) * posMultiplier) - (tv.getWidth() / 2f);
            y = (((float) cy) * posMultiplier) - (tv.getHeight() / 2f);

            if (tv.getAlpha() != 0f) {
                ans.add(ObjectAnimator.ofFloat(tv, View.TRANSLATION_X, x));
                ans.add(ObjectAnimator.ofFloat(tv, View.TRANSLATION_Y, y));
                if (tv.getAlpha() != 1f)
                    ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 1f));
            } else {
                tv.setTranslationX(x);
                tv.setTranslationY(y);
                ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 1f));
            }
        }
        //Toast.makeText(c, sb.toString(), Toast.LENGTH_SHORT).show();
        trans = new AnimatorSet();
        trans.setDuration(transDur);
        trans.playTogether(ans);
        trans.start();
    }
}
