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

3. [**VisMemory**](memory.cpp) : it stores specific shapes for learning.

#### Other classes and procedural files

- [BitmapStream](bitmap_stream.hpp) : it stores image frames in BMP image format for testing in *MyCV*.
- [Colouring](colouring.hpp) : it makes a very simple effect in the UI of the Android app,
  indicating pleasure (in green) or pain (in red).
- [segment.hpp](segment.hpp) : it defines struct Segment and the type of path points in shapes.
- [VisualSTM](visual_stm.cpp) : *Visual Short-Term Memory*; it stores shapes (segments)
  temporarily on a non-volatile (persistent) memory for debugging.
  Translated from *MyCV*'s [**Volatile Indices 1**](
  https://github.com/fulcrum6378/mycv/blob/master/storage/volatile_indices_1.py).
