## VIS: Visual Input

Vision is, without doubt, the most crucial sense (data input) of an intelligent being
for their knowledge and information to be based upon.

Mergen uses the default color model outputted by many cameras called [**Y′UV**](
https://en.wikipedia.org/wiki/Y%E2%80%B2UV), instead of RGB or etc.
In terms of hexadecimal, *YUV* is no different than [YCbCr](https://en.wikipedia.org/wiki/YCbCr).

This component contains the following steps:

1. [**Camera**](camera.cpp) : responsible for dealing with Android's camera API,
   reading image frames and passing them onto the next step. This step is platform-specific.

2.
    1. [~~Segmentation~~](segmentation.cpp) : it divides an image frame into numerous segments
       and performs some kinds of analyses on some or all of them, including Image Tracing.

       Translated from *MyCV*'s [*Region Growing 4*](
       https://github.com/fulcrum6378/mycv/blob/master/segmentation/region_growing_4.py) and [*Comprehender*](
       https://github.com/fulcrum6378/mycv/blob/master/tracing/comprehender_rg4.py).

       Then the analysed segments will go under an *Object Tracking* procedure.

       This method was deprecated in favour of Edge Detection.

    2. [**EdgeDetection**](edge_detection.cpp) : it uses GPU/Vulkan to detect edge pixels of an image frame.

3. [**Perception**](perception.hpp) : making sense of the segments' changes across frames.

#### Deprecated classes

- [BitmapStream](bitmap.hpp) : it stores image frames in BMP image format for testing in *MyCV*.
- [Segment](segment.hpp) : it defines a Segment and type of path points in it.
  temporarily on a non-volatile (persistent) memory for debugging.
  Translated from *MyCV*'s [*Volatile Indices 1*](
  https://github.com/fulcrum6378/mycv/blob/master/storage/volatile_indices_1.py).
- [VisualSTM](visual_stm.cpp) : *Visual Short-Term Memory*; it stores shapes (segments)
- [VisMemory](memory.cpp) : it stores specific shapes for learning.

## VIS: Visual Output

- [**Colouring**](colouring.hpp) : it makes a very simple effect in the UI of the Android app,
  indicating pleasure (in green) or pain (in red). (subclass of [Criterion](../rew/criterion.hpp))

