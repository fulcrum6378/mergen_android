## VIS: Vision

Vision is, without doubt, the most crucial sense (data input) of an intelligent being
for their knowledge and information to be based upon.

This component contains the following steps:

1. [**Camera**](camera.cpp) : responsible for dealing with Android's camera API,
   reading image frames and passing them onto the next step. This step is platform-specific.

2. [**Segmentation**](segmentation.cpp) : it divides an image frame into numerous segments
   and performs some kinds of analysis on some or all of them.

   Translated from *MyCV*'s [**Region Growing 4**](
   https://github.com/fulcrum6378/mycv/blob/master/segmentation/region_growing_4.py) and [**Comprehender**](
   https://github.com/fulcrum6378/mycv/blob/master/tracing/comprehender_rg4.py).

   2.1. **Resegmentation** : which updates segments of *Segmentation* in a second analysis.
   Translated from [First](https://github.com/fulcrum6378/mycv/blob/master/resegmentation/first_rg4.py).

3. [**VisualSTM**](visual_stm.cpp) : *Visual Short-Term Memory*; it stores shapes (segments)
   temporarily on a non-volatile (persistent) memory for a later (as soon as possible) analysis by
   the [**Subconscious Mind (SCM)**](../scm).

   Translated from *MyCV*'s [**Sequence Files 2**](
   https://github.com/fulcrum6378/mycv/blob/master/storage/sequence_files_2.py).

#### Other classes

- [BitmapStream](bitmap_stream.h) : it stores image frames in BMP image format for testing in *MyCV*.
- [Colouring](colouring.h) : it makes a very simple effect in the UI of the Android app,
  indicating pleasure (in green) or pain (in red).
