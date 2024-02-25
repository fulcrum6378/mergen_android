package ir.mahdiparastesh.mergen;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

/** Visualises how image frames are being analysed by marking some of the largest segments. */
public class SegmentMarkers {
    final float posMultiplier = 1088f / 720f;

    final Main c;
    final RelativeLayout pool;

    public SegmentMarkers(Main c, RelativeLayout pool) {
        this.c = c;
        this.pool = pool;
    }

    /** Nominal visible ID incrementer */
    int incrementer = 0;
    /** Maps nominal visible IDs (TextView positions) to SIDs. */
    HashMap<Short, Integer> markers = new HashMap<>();
    /** Holds on to the nulled positions (used for efficiency). */
    ArrayList<Integer> freedMarkers = new ArrayList<>();
    /** Plays all the animations together. */
    AnimatorSet trans;

    /**
     * Called by C++ to update the pointers on the screen.
     * Java numbers are always big-endian! (bits have the same order, but bytes need to be reordered)
     */
    public void update(long[] data) {
        short sid, best, cx, cy; // don't make them `int`
        float x, y;
        boolean bNew;
        int pos;

        // cancel all previous animations and prepare for new ones
        if (trans != null && trans.isStarted()) trans.cancel();
        ArrayList<Animator> ans = new ArrayList<>();

        // all the remaining SIDs will be discarded at the end.
        HashSet<Short> deletables = new HashSet<>(markers.keySet());

        // inserted and updated segments
        for (long d : data) {
            sid = (short) (d & 0x000000000000FFFFL);
            best = (short) ((d & 0x00000000FFFF0000L) >> 16);
            if (best == -2) continue;
            TextView tv;

            bNew = best == -1 || !markers.containsKey(best);
            if (bNew) {
                if (freedMarkers.isEmpty()) {
                    pos = incrementer;
                    tv = new TextView(c, null, 0, R.style.segmentMarker);
                    tv.setText(String.valueOf(pos));
                    pool.addView(tv);
                    incrementer++;
                } else {
                    pos = freedMarkers.get(0);
                    tv = (TextView) pool.getChildAt(pos);
                    freedMarkers.remove(0);
                }
            } else {
                //noinspection DataFlowIssue
                pos = markers.get(best);
                tv = (TextView) pool.getChildAt(pos);
                tv.setText(String.valueOf(pos));
                markers.remove(best);
            }
            markers.put(sid, pos);
            deletables.remove(sid);

            cx = (short) ((d & 0x0000FFFF00000000L) >> 32);
            cy = (short) (/*(*/d/* & 0xFFFF000000000000L)*/ >> 48);
            x = (((float) cx) * posMultiplier) - (tv.getWidth() / 2f);
            y = (((float) cy) * posMultiplier) - (tv.getHeight() / 2f);

            if (bNew) {
                tv.setTranslationX(x);
                tv.setTranslationY(y);
                ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 1f));
            } else {
                ans.add(ObjectAnimator.ofFloat(tv, View.TRANSLATION_X, x));
                ans.add(ObjectAnimator.ofFloat(tv, View.TRANSLATION_Y, y));
                if (tv.getAlpha() != 1f)
                    ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 1f));
            }
        }

        // `delete`d segments
        for (Short d : deletables) {
            TextView tv = (TextView) pool.getChildAt(d);
            ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 0f));
        }

        // play the animations
        trans = new AnimatorSet();
        trans.setDuration(400L);
        trans.playTogether(ans);
        trans.start();
    }
}
