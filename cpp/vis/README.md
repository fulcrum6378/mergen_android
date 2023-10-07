## VIS: Vision

Vision is, without doubt, the most crucial sense (data input) of an intelligent being
for their knowledge and information to be based upon.

This part contains the following steps:

1. [**Camera**](cpp/vis/camera.cpp) : responsible for dealing with Android's camera API,
   reading image frames and passing them onto the next step.

2. [**Segmentation**](cpp/vis/segmentation.cpp) : it divides an image frame into numerous segments
   and performs some kinds of analysis on some or all of them.

   It was translated to C++ from a subproject of Mergen, called [**MyCV**](https://github.com/fulcrum6378/mycv).

3. [**VisualSTM**](cpp/vis/visual_stm.cpp) : *Visual Short-Term Memory*; it stores shapes (segments)
   temporarily on a non-volatile (persistent) memory for a later (as soon as possible) analysis by
   [**SCM**](cpp/scm).

#### Other classes

- [BitmapStream](cpp/vis/bitmap_stream.h) : it stores image frames in BMP image format for testing in *MyCV*.
- [Colouring](cpp/vis/colouring.h) : it makes a very simple effect in the UI of the Android app,
  indicating pleasure (in green) or pain (in red).
