package ir.mahdiparastesh.mergen;

import android.animation.Animator;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;

/** Visualises how image frames are being analysed by marking some of the largest segments. */
public class SegmentMarkers {
    final String[] alphabet = new String[]{
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

        // prepare markers for the entire alphabet
        for (String a : alphabet) {
            TextView tv = new TextView(c, null, 0, R.style.segmentMarker);
            tv.setText(a);
            this.pool.addView(tv);
        }
    }

    /** Plays all the animations together. */
    AnimatorSet trans;
    /** Maps `alphabet` items to segment indices. */
    Short[] aToSeg = new Short[alphabet.length];
    /** Maps segment indices to `alphabet` items. */
    Integer[] segToA = new Integer[alphabet.length / 2];

    /**
     * Called by C++ to update the pointers on the screen.
     * Java numbers are always big-endian! (bits have the same order, but bytes need to be reordered)
     */
    public void update(long[] data) {
        short best, cx, cy; // don't make them `int`
        float x, y;
        boolean bNew;

        // cancel all previous animations and prepare for new ones
        if (trans != null && trans.isStarted()) trans.cancel();
        ArrayList<Animator> ans = new ArrayList<>();

        // all values of `aToSeg` with these marker indexes will be removed at the end.
        HashSet<Integer> deletables = new HashSet<>();
        for (int a = 0; a < alphabet.length; a++) deletables.add(a);

        // `insert`ed and `update`d segments
        for (short sdx = 0; sdx < data.length; sdx++) {
            best = (short) ((data[sdx] & 0x00000000FFFF0000L) >> 16);
            if (best == -2) continue;
            Integer marker = null;
            TextView tv;

            bNew = best == -1 || segToA[best] == null; // best must not be -2 here!
            if (bNew) {
                for (int i = 0; i < aToSeg.length; i++)
                    if (aToSeg[i] == null) {
                        marker = i;
                        break;
                    }
                if (marker == null)
                    throw new IllegalStateException("Number of segments shouldn't exceed 26!");
            } else {
                marker = segToA[best];
                assert marker != null;
            }
            aToSeg[marker] = sdx;
            deletables.remove(marker);
            tv = (TextView) pool.getChildAt(marker);

            cx = (short) ((data[sdx] & 0x0000FFFF00000000L) >> 32);
            cy = (short) (/*(*/data[sdx]/* & 0xFFFF000000000000L)*/ >> 48);
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
        for (Integer d : deletables) {
            TextView tv = (TextView) pool.getChildAt(d);
            ans.add(ObjectAnimator.ofFloat(tv, View.ALPHA, 0f));
        }

        // play the animations
        trans = new AnimatorSet();
        trans.setDuration(transDur);
        trans.playTogether(ans);
        trans.start();

        // reset `segToA`
        Arrays.fill(segToA, null);
        for (int a = 0; a < aToSeg.length; a++) segToA[aToSeg[a]] = a;
    }
}
